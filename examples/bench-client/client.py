# Silicon LogiX / SLX ResilienceLab
# Copyright (c) 2026 Marco Pezzullo / Silicon LogiX.
# Author: Marco Pezzullo
# License: Silicon LogiX Evaluation License 1.0. See LICENSE.
# Project client implementation.

"""Read or command the shared live bench over local TCP or USB serial."""
import argparse
import json
import socket
import sys


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("op", choices=["read", "measurements", "target", "fault", "live", "run", "pause", "reset", "step"])
    parser.add_argument("--port", type=int, default=47400)
    parser.add_argument("--serial", help="USB serial port, for example COM3 or /dev/ttyACM0")
    parser.add_argument("--baud", type=int, default=115200)
    parser.add_argument("--value", type=float)
    parser.add_argument("--kind")
    parser.add_argument("--sensors", default="0", help="Comma-separated indices, for example 0,1,2")
    parser.add_argument("--controller", choices=["main", "backup"])
    args = parser.parse_args()
    request = {"id": 1, "op": args.op}
    if args.value is not None:
        request["value"] = args.value
    if args.kind:
        request.update(kind=args.kind, sensors=[int(index) for index in args.sensors.split(",")])
    if args.controller:
        request["controller"] = args.controller
    frame = json.dumps(request, allow_nan=False).encode("utf-8") + b"\n"
    if args.serial:
        try:
            import serial  # Optional client dependency; not needed by the desktop.
        except ImportError:
            parser.error("Serial client mode requires pyserial; install the example requirements")
        with serial.Serial(args.serial, args.baud, timeout=3, write_timeout=3) as device:
            device.write(frame)
            reply = device.read_until(b"\n", 32769)
    else:
        with socket.create_connection(("127.0.0.1", args.port), timeout=3) as device:
            device.sendall(frame)
            with device.makefile("rb") as stream:
                reply = stream.readline(32769)
    if not reply.endswith(b"\n") or len(reply) > 32768:
        raise ValueError("Incomplete or oversized API response")
    result = json.loads(reply)
    if result.get("id") != request["id"]:
        raise ValueError("Response ID does not match request")
    print(json.dumps(result, indent=2))
    return 0 if result.get("ok") else 1


if __name__ == "__main__":
    try:
        sys.exit(main())
    except (OSError, ValueError) as error:
        print(str(error), file=sys.stderr)
        sys.exit(2)
