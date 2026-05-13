import serial
import time
import math

# 虚拟设备通过 COM5 发送数据，上位机从 COM6 接收
PORT = "COM5"
BAUD = 9600

def generate_frame(t):
    # 冷库温度：-18℃ ± 3℃
    temp = -18.0 + 3.0 * math.sin(t * 0.05)
    # 湿度：85% ± 10%
    humidity = 85.0 + 10.0 * math.sin(t * 0.08 + 1.0)
    # 压力：0.1013 ± 0.005 MPa
    pressure = 0.1013 + 0.005 * math.sin(t * 0.03 + 2.0)
    # CO₂：800 ± 300 ppm
    co2 = 800.0 + 300.0 * math.sin(t * 0.06 + 0.5)
    # 门状态：每300帧开门一次
    door = 1 if (int(t) % 300 >= 200 and int(t) % 300 < 230) else 0

    # 状态判断
    if temp > -15.0 or co2 > 1000.0:
        status = "ALARM"
    elif humidity > 95.0 or door == 1:
        status = "WARN"
    else:
        status = "OK"

    return f"{temp:.1f},{humidity:.1f},{pressure:.4f},{co2:.0f},{door},{status}\r\n"

def main():
    try:
        ser = serial.Serial(PORT, BAUD, timeout=1)
        print(f"虚拟设备已启动，通过 {PORT} 发送数据...")
        print("上位机请连接 COM6，按 Ctrl+C 停止")

        t = 0
        while True:
            frame = generate_frame(t)
            ser.write(frame.encode('utf-8'))
            print(f"发送: {frame.strip()}")
            t += 1
            time.sleep(0.1)  # 100ms 发送一帧

    except serial.SerialException as e:
        print(f"串口错误: {e}")
    except KeyboardInterrupt:
        print("\n已停止")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()

if __name__ == "__main__":
    main()