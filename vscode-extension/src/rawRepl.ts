/*
 * SPDX-FileCopyrightText: 2026 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

import { SerialTransport } from "./serialTransport";

const CTRL_A = Buffer.from([0x01]);
const CTRL_B = Buffer.from([0x02]);
const CTRL_C = Buffer.from([0x03]);
const CTRL_D = Buffer.from([0x04]);

function delay(ms: number): Promise<void> {
    return new Promise((resolve) => setTimeout(resolve, ms));
}

export class RawRepl {
    constructor(private readonly transport: SerialTransport) {
    }

    async enter(): Promise<void> {
        this.transport.clearInput();
        await this.transport.write(Buffer.concat([CTRL_C, CTRL_C]));
        await delay(100);
        this.transport.clearInput();
        await this.transport.write(CTRL_A);
        await this.transport.readUntil("raw REPL", 3000);
        await this.transport.readUntil(">", 3000);
    }

    async exit(): Promise<void> {
        await this.transport.write(CTRL_B);
    }

    async runScript(source: string): Promise<void> {
        await this.enter();
        this.transport.clearInput();
        await this.transport.write(source);
        if (!source.endsWith("\n")) {
            await this.transport.write("\n");
        }
        await this.transport.write(CTRL_D);
        await this.transport.readUntil("OK", 3000);
    }

    async stopScript(): Promise<void> {
        await this.transport.write(CTRL_C);
    }

    async softReset(): Promise<void> {
        await this.enter();
        this.transport.clearInput();
        await this.transport.write(CTRL_D);
        await delay(500);
    }
}
