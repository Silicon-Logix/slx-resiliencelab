<!-- Copyright (c) 2026 Marco Pezzullo / Silicon LogiX. License: see LICENSE. -->
# USB board example

`board-client.ino` illustrates an Arduino-compatible USB CDC client with at least 8 KiB RAM. Connect to the selected COM/tty port at 115200 8N1 after starting the live bench. It polls `measurements` at 5 Hz and lights the built-in LED when the host reports stopped motion or the response times out. A button from pin 2 to ground requests 80% opening.

The board observes host control; it does not replace the motor algorithm. The fixed response buffer is bounded. This example matches only compact diagnostic fields; production firmware should use a complete parser and validate request/response IDs. Board-specific compilation, pins, USB behavior and a physical run must be checked for the selected hardware. No physical board validation is claimed.
