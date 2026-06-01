#!/usr/bin/env python3
"""Virtual serial device for QtDeviceMonitor.

The script writes CSV frames to one side of a virtual serial pair. For example,
create COM5 <-> COM6, start this script on COM5, then connect the app to COM6.
"""

import argparse
import math
import signal
import sys
import time

try:
    import serial
except ImportError as exc:
    raise SystemExit(
        "pyserial is required. Install it with: pip install pyserial"
    ) from exc


DEFAULT_PORT = "COM5"
DEFAULT_BAUD = 9600
DEFAULT_INTERVAL_MS = 100


def generate_frame(t):
    temp = -18.0 + 3.0 * math.sin(t * 0.05)
    humidity = 85.0 + 10.0 * math.sin(t * 0.08 + 1.0)
    pressure = 0.1013 + 0.005 * math.sin(t * 0.03 + 2.0)
    co2 = 800.0 + 300.0 * math.sin(t * 0.06 + 0.5)
    door = 1 if 200 <= (int(t) % 300) < 230 else 0

    if temp > -15.0 or co2 > 1000.0:
        status = "ALARM"
    elif humidity > 95.0 or door == 1:
        status = "WARN"
    else:
        status = "OK"

    return f"{temp:.2f},{humidity:.2f},{pressure:.6f},{co2:.1f},{door},{status}\r\n"


def parse_args():
    parser = argparse.ArgumentParser(description="QtDeviceMonitor virtual serial device")
    parser.add_argument("--port", default=DEFAULT_PORT, help="serial port to write, default: COM5")
    parser.add_argument("--baud", type=int, default=DEFAULT_BAUD, help="baud rate, default: 9600")
    parser.add_argument(
        "--interval-ms",
        type=int,
        default=DEFAULT_INTERVAL_MS,
        help="send interval in milliseconds, default: 100",
    )
    parser.add_argument(
        "--log-every",
        type=int,
        default=10,
        help="print every N frames, use 0 to disable frame logs",
    )
    return parser.parse_args()


def main():
    args = parse_args()
    interval_sec = max(args.interval_ms, 10) / 1000.0
    running = True

    def stop(_signum=None, _frame=None):
        nonlocal running
        running = False

    signal.signal(signal.SIGINT, stop)
    signal.signal(signal.SIGTERM, stop)

    try:
        with serial.Serial(args.port, args.baud, timeout=1) as ser:
            print(
                f"Virtual serial device started on {args.port}, "
                f"baud={args.baud}, interval={args.interval_ms}ms",
                flush=True,
            )
            print("Connect QtDeviceMonitor to the paired port, for example COM6.", flush=True)

            t = 0
            while running:
                frame = generate_frame(t)
                ser.write(frame.encode("utf-8"))
                if args.log_every > 0 and t % args.log_every == 0:
                    print(f"sent: {frame.strip()}", flush=True)
                t += 1
                time.sleep(interval_sec)

    except serial.SerialException as exc:
        print(f"serial error: {exc}", file=sys.stderr, flush=True)
        return 2

    print("Virtual serial device stopped.", flush=True)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
