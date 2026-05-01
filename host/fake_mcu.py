"""
Fake MCU that mimics the container monitor's UART output.
Use a virtual COM port pair (e.g. com0com COM10<->COM11) so this script
writes on one end and collector.py reads on the other.

Usage:
  python fake_mcu.py COM10                        # 50Hz IMU, 30s env (testing)
  python fake_mcu.py COM10 --env-period 900       # 15min env (real timing)
"""
import argparse
import math
import random
import time

import serial


def main() -> None:
    p = argparse.ArgumentParser()
    p.add_argument("port")
    p.add_argument("--baud", type=int, default=115200)
    p.add_argument("--imu-hz", type=float, default=50.0)
    p.add_argument(
        "--env-period",
        type=float,
        default=30.0,
        help="seconds between env samples (real device: 900)",
    )
    args = p.parse_args()

    ser = serial.Serial(args.port, args.baud, timeout=1)
    print(f"[fake_mcu] writing to {args.port} @ {args.baud}, IMU {args.imu_hz}Hz, ENV every {args.env_period}s")

    imu_dt = 1.0 / args.imu_hz
    t0 = time.monotonic()
    next_imu = t0
    next_env = t0
    imu_seq = 0
    env_seq = 0

    try:
        while True:
            now = time.monotonic()

            if now >= next_imu:
                ms = int((now - t0) * 1000)
                t = now - t0
                # gentle ship-roll motion + noise, MPU6050-like raw ints
                ax = int(1000 * math.sin(t * 0.3) + random.gauss(0, 30))
                ay = int(800 * math.cos(t * 0.25) + random.gauss(0, 30))
                az = int(16384 + random.gauss(0, 50))  # ~1g on Z
                gx = int(random.gauss(0, 50))
                gy = int(random.gauss(0, 50))
                gz = int(random.gauss(0, 50))
                ser.write(f"I,{imu_seq},{ms},{ax},{ay},{az},{gx},{gy},{gz}\n".encode())
                imu_seq += 1
                next_imu += imu_dt

            if now >= next_env:
                ms = int((now - t0) * 1000)
                temp = 22.0 + random.gauss(0, 0.3)
                humidity = 55.0 + random.gauss(0, 1.0)
                pressure = 1013.0 + random.gauss(0, 0.5)
                gas = 50000 + int(random.gauss(0, 500))
                ser.write(
                    f"E,{env_seq},{ms},{temp:.2f},{humidity:.2f},{pressure:.2f},{gas}\n".encode()
                )
                env_seq += 1
                next_env += args.env_period

            sleep_for = min(next_imu, next_env) - time.monotonic()
            if sleep_for > 0:
                time.sleep(sleep_for)
    except KeyboardInterrupt:
        print("\n[fake_mcu] stopping.")
    finally:
        ser.close()


if __name__ == "__main__":
    main()
