"""
Serial -> CSV collector for the container monitoring sim.
Reads two stream prefixes from the MCU (UART) and writes them into
separate CSV files under ../data/, stamped with PC wall-clock time.

  I,seq,ms,ax,ay,az,gx,gy,gz             -> imu_<ts>.csv     (50Hz)
  E,seq,ms,temp_c,humidity,pressure,gas  -> env_<ts>.csv     (15min)

Usage:
  python collector.py                    # default: COM3 @ 115200
  python collector.py COM5               # custom port
  python collector.py COM5 921600        # custom port + baud
"""
import csv
import datetime
import pathlib
import sys

import serial

PORT = sys.argv[1] if len(sys.argv) > 1 else "COM3"
BAUD = int(sys.argv[2]) if len(sys.argv) > 2 else 115200

DATA_DIR = pathlib.Path(__file__).resolve().parent.parent / "data"
DATA_DIR.mkdir(exist_ok=True)

HEADERS = {
    "I": ["pc_ts", "seq", "mcu_ms", "ax", "ay", "az", "gx", "gy", "gz"],
    "E": ["pc_ts", "seq", "mcu_ms", "temp_c", "humidity", "pressure", "gas_ohm"],
}
PREFIX_NAME = {"I": "imu", "E": "env"}


def main() -> None:
    ts = datetime.datetime.now().strftime("%Y%m%d_%H%M%S")
    files = {
        k: open(DATA_DIR / f"{PREFIX_NAME[k]}_{ts}.csv", "w", newline="", encoding="utf-8")
        for k in HEADERS
    }
    writers = {k: csv.writer(v) for k, v in files.items()}
    for k, w in writers.items():
        w.writerow(HEADERS[k])

    print(f"[collector] {PORT} @ {BAUD}, writing to {DATA_DIR}")
    print("[collector] Ctrl+C to stop.")

    try:
        with serial.Serial(PORT, BAUD, timeout=1) as ser:
            while True:
                raw = ser.readline()
                if not raw:
                    continue
                try:
                    line = raw.decode("ascii", errors="ignore").strip()
                except UnicodeDecodeError:
                    continue
                if not line or line[0] not in writers:
                    continue
                kind, *fields = line.split(",")
                expected = len(HEADERS[kind]) - 1
                if len(fields) != expected:
                    continue
                writers[kind].writerow([datetime.datetime.now().isoformat(timespec="milliseconds")] + fields)
                files[kind].flush()
    except KeyboardInterrupt:
        print("\n[collector] stopping.")
    finally:
        for f in files.values():
            f.close()


if __name__ == "__main__":
    main()
