/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import { StringDecoder } from "string_decoder";
import * as vscode from "vscode";
import { RawRepl } from "./rawRepl";
import { SerialPortInfo, SerialTransport } from "./serialTransport";

let transport: SerialTransport | undefined;
let rawRepl: RawRepl | undefined;
let output: vscode.OutputChannel | undefined;
let status: vscode.StatusBarItem | undefined;
let runButton: vscode.StatusBarItem | undefined;
let stopButton: vscode.StatusBarItem | undefined;
let resetButton: vscode.StatusBarItem | undefined;
let currentPort = "";
let suppressSerialOutput = false;
let rawOutputParser: RawOutputParser | undefined;

interface RawOutputParser {
    phase: "stdout" | "stderr" | "prompt";
    decoder: StringDecoder;
    stderrBytes: number;
    stopRequested: boolean;
}

export function activate(context: vscode.ExtensionContext): void {
    const nextOutput = vscode.window.createOutputChannel("ESP-VISION");
    const nextStatus = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 100);
    const nextRunButton = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 99);
    const nextStopButton = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 98);
    const nextResetButton = vscode.window.createStatusBarItem(vscode.StatusBarAlignment.Left, 97);
    output = nextOutput;
    status = nextStatus;
    runButton = nextRunButton;
    stopButton = nextStopButton;
    resetButton = nextResetButton;
    nextRunButton.text = "$(play)";
    nextRunButton.tooltip = "ESP-VISION: Run Current File";
    nextRunButton.command = "espVision.runCurrentFile";
    nextStopButton.text = "$(debug-stop)";
    nextStopButton.tooltip = "ESP-VISION: Stop Script";
    nextStopButton.command = "espVision.stopScript";
    nextResetButton.text = "$(debug-restart)";
    nextResetButton.tooltip = "ESP-VISION: Soft Reset";
    nextResetButton.command = "espVision.softReset";
    context.subscriptions.push(nextOutput, nextStatus, nextRunButton, nextStopButton, nextResetButton);
    updateStatus();

    context.subscriptions.push(
        vscode.commands.registerCommand("espVision.connect", connect),
        vscode.commands.registerCommand("espVision.disconnect", disconnect),
        vscode.commands.registerCommand("espVision.runCurrentFile", runCurrentFile),
        vscode.commands.registerCommand("espVision.stopScript", stopScript),
        vscode.commands.registerCommand("espVision.softReset", softReset),
    );
}

export async function deactivate(): Promise<void> {
    await disconnect({ quiet: true });
    output = undefined;
    status = undefined;
    runButton = undefined;
    stopButton = undefined;
    resetButton = undefined;
}

async function connect(): Promise<void> {
    const config = vscode.workspace.getConfiguration("espVision");
    const configuredPort = config.get<string>("serialPort", "");
    const baudRate = config.get<number>("baudRate", 115200);
    const port = configuredPort || await pickSerialPort();

    if (!port) {
        return;
    }

    if (transport?.isOpen) {
        await disconnect();
    }

    const nextTransport = new SerialTransport();
    nextTransport.on("data", handleSerialData);
    nextTransport.on("error", (error: Error) => {
        appendOutputLine(`\n[serial error] ${error.message}`);
        vscode.window.showErrorMessage(`ESP-VISION serial error: ${error.message}`);
    });
    nextTransport.on("close", () => {
        if (transport === nextTransport) {
            transport = undefined;
            rawRepl = undefined;
            currentPort = "";
        }
        appendOutputLine("\n[serial closed]");
        updateStatus(false);
    });

    await nextTransport.open(port, baudRate);
    transport = nextTransport;
    rawRepl = new RawRepl(nextTransport);
    currentPort = port;
    showOutput(true);
    appendOutputLine(`[connected] ${port} @ ${baudRate}`);
    updateStatus(true);
}

async function disconnect(options: { quiet?: boolean } = {}): Promise<void> {
    const currentTransport = transport;
    if (!currentTransport) {
        return;
    }

    transport = undefined;
    rawRepl = undefined;
    currentPort = "";
    rawOutputParser = undefined;
    suppressSerialOutput = false;

    if (options.quiet) {
        currentTransport.removeAllListeners();
    }

    await currentTransport.close();
    updateStatus(false);
}

async function runCurrentFile(): Promise<void> {
    const repl = requireRawRepl();
    if (!repl) {
        return;
    }

    const editor = vscode.window.activeTextEditor;
    if (!editor) {
        vscode.window.showWarningMessage("No active editor.");
        return;
    }

    if (editor.document.isDirty) {
        await editor.document.save();
    }

    showOutput(true);
    appendOutputLine(`\n[run] ${editor.document.fileName}`);
    rawOutputParser = undefined;
    suppressSerialOutput = true;
    try {
        await repl.runScript(editor.document.getText());
        rawOutputParser = {
            phase: "stdout",
            decoder: new StringDecoder("utf8"),
            stderrBytes: 0,
            stopRequested: false,
        };
        const pending = transport?.takeInput();
        if (pending?.length) {
            handleSerialData(pending);
        }
    } catch (error) {
        const message = error instanceof Error ? error.message : String(error);
        appendOutputLine(`[error] ${message}`);
        vscode.window.showErrorMessage(`ESP-VISION run failed: ${message}`);
    } finally {
        suppressSerialOutput = false;
    }
}

async function stopScript(): Promise<void> {
    const repl = requireRawRepl();
    if (!repl) {
        return;
    }

    appendOutputLine("\n[stop]");
    if (rawOutputParser) {
        rawOutputParser.stopRequested = true;
    }
    await repl.stopScript();
}

async function softReset(): Promise<void> {
    const repl = requireRawRepl();
    if (!repl) {
        return;
    }

    appendOutputLine("\n[soft reset]");
    rawOutputParser = undefined;
    suppressSerialOutput = true;
    try {
        await repl.softReset();
        transport?.clearInput();
    } finally {
        suppressSerialOutput = false;
    }
}

async function pickSerialPort(): Promise<string | undefined> {
    const ports = await SerialTransport.listPorts();
    if (ports.length === 0) {
        vscode.window.showWarningMessage("No serial ports found.");
        return undefined;
    }

    const selected = await vscode.window.showQuickPick(
        ports.map((port: SerialPortInfo) => ({
            label: port.path,
            description: port.label === port.path ? undefined : port.label,
            port,
        })),
        { placeHolder: "Select ESP-VISION serial port" },
    );

    return selected?.port.path;
}

function requireRawRepl(): RawRepl | undefined {
    if (!rawRepl || !transport?.isOpen) {
        vscode.window.showWarningMessage("ESP-VISION is not connected.");
        return undefined;
    }
    return rawRepl;
}

function updateStatus(connected = Boolean(transport?.isOpen)): void {
    if (!status) {
        return;
    }
    try {
        status.text = connected ? `$(plug) ${currentPort}` : "$(plug) ESP-VISION";
        status.tooltip = connected ? "ESP-VISION board connected" : "Connect to ESP-VISION board";
        status.command = connected ? "espVision.disconnect" : "espVision.connect";
        status.show();
        runButton?.show();
        stopButton?.show();
        resetButton?.show();
    } catch {
        status = undefined;
    }
}

function appendOutput(value: string): void {
    try {
        output?.append(value);
    } catch {
        output = undefined;
    }
}

function handleSerialData(data: Buffer): void {
    if (rawOutputParser) {
        handleRawOutputData(data);
        return;
    }

    if (suppressSerialOutput) {
        return;
    }

    appendOutput(data.toString("utf8"));
}

function handleRawOutputData(data: Buffer): void {
    let remaining = data;

    while (remaining.length > 0 && rawOutputParser) {
        const parser = rawOutputParser;
        if (rawOutputParser.phase === "prompt") {
            const promptIndex = remaining.indexOf(">");
            if (promptIndex < 0) {
                return;
            }
            appendOutputLine(statusTextForRawParser(parser));
            rawOutputParser = undefined;
            remaining = remaining.subarray(promptIndex + 1);
            continue;
        }

        const delimiterIndex = remaining.indexOf(0x04);
        if (delimiterIndex < 0) {
            appendRawStreamData(parser, remaining);
            return;
        }

        appendRawStreamData(parser, remaining.subarray(0, delimiterIndex));
        appendOutput(normalizeLineEndings(parser.decoder.end()));
        remaining = remaining.subarray(delimiterIndex + 1);
        parser.decoder = new StringDecoder("utf8");
        parser.phase = parser.phase === "stdout" ? "stderr" : "prompt";
    }

    if (remaining.length > 0) {
        handleSerialData(remaining);
    }
}

function appendRawStreamData(parser: RawOutputParser, data: Buffer): void {
    if (data.length === 0) {
        return;
    }
    if (parser.phase === "stderr") {
        parser.stderrBytes += data.length;
    }
    appendOutput(normalizeLineEndings(parser.decoder.write(data)));
}

function statusTextForRawParser(parser: RawOutputParser): string {
    if (parser.stopRequested) {
        return "[stopped]";
    }
    if (parser.stderrBytes > 0) {
        return "[failed]";
    }
    return "[done]";
}

function normalizeLineEndings(value: string): string {
    return value.replace(/\r\n/g, "\n").replace(/\r/g, "\n");
}

function appendOutputLine(value: string): void {
    try {
        output?.appendLine(value);
    } catch {
        output = undefined;
    }
}

function showOutput(preserveFocus: boolean): void {
    try {
        output?.show(preserveFocus);
    } catch {
        output = undefined;
    }
}
