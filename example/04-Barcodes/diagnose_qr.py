# 二维码识别诊断脚本
# 测试不同镜像组合，并输出调试信息
import sensor
import time
import gc

sensor.reset()
sensor.set_pixformat(sensor.GRAYSCALE)
sensor.set_framesize(sensor.QVGA)

# 打印当前状态
print("=== 初始状态 ===")
st = sensor.status()
print("状态字典:", st)
print("分辨率: {}x{}".format(st.get('width', '?'), st.get('height', '?')))

# 测试1: 当前设置
print("\n=== 测试1: 当前设置 (hmirror=1, vflip=1) ===")
sensor.skip_frames(time=500)
img = sensor.snapshot()
print("图像尺寸: {}x{}".format(img.width(), img.height()))
print("图像格式:", img.format())
gc.collect()
print("可用内存:", gc.mem_free())
codes = img.find_qrcodes()
print("识别到二维码数量:", len(codes))
for c in codes:
    print("  内容:", c.payload())
img.flush()

# 测试2: 关闭 hmirror
print("\n=== 测试2: hmirror=0, vflip=1 ===")
sensor.set_hmirror(False)
sensor.skip_frames(time=500)
img = sensor.snapshot()
gc.collect()
codes = img.find_qrcodes()
print("识别到二维码数量:", len(codes))
for c in codes:
    print("  内容:", c.payload())
img.flush()

# 测试3: 关闭 vflip
print("\n=== 测试3: hmirror=0, vflip=0 ===")
sensor.set_vflip(False)
sensor.skip_frames(time=500)
img = sensor.snapshot()
gc.collect()
codes = img.find_qrcodes()
print("识别到二维码数量:", len(codes))
for c in codes:
    print("  内容:", c.payload())
img.flush()

# 测试4: 只开 hmirror
print("\n=== 测试4: hmirror=1, vflip=0 ===")
sensor.set_hmirror(True)
sensor.skip_frames(time=500)
img = sensor.snapshot()
gc.collect()
codes = img.find_qrcodes()
print("识别到二维码数量:", len(codes))
for c in codes:
    print("  内容:", c.payload())
img.flush()

# 测试5: 切回 RGB565 试试
print("\n=== 测试5: RGB565 模式 ===")
sensor.set_pixformat(sensor.RGB565)
sensor.set_hmirror(True)
sensor.set_vflip(True)
sensor.skip_frames(time=500)
img = sensor.snapshot()
print("图像格式:", img.format())
gc.collect()
print("可用内存:", gc.mem_free())
# 先转灰度再识别
gs = img.to_grayscale(copy=True)
gc.collect()
print("转灰度后内存:", gc.mem_free())
codes = gs.find_qrcodes()
print("识别到二维码数量:", len(codes))
for c in codes:
    print("  内容:", c.payload())

print("\n=== 诊断完成 ===")
