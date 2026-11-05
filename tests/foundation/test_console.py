#!/usr/bin/env python3
#-----------------------------------------------------------------------------
# Umicom OS
# File: tests/foundation/test_console.py
#
# PURPOSE:
#   Exercise the actual console in an ordinary process, never guest init.
#
# AUTHOR AND ORGANISATION:
# Sammy Hegab
# Umicom Foundation
#
# LICENCE:
# MIT
#---------------------------------------------------------------------------
import subprocess
import sys
CASES={
    "help":(b"help\npoweroff\n",["No shell", "STOP_REQUEST=1"]),
    "unknown":(b"execute-command\npoweroff\n",["Unknown command", "STOP_REQUEST=1"]),
    "path":(b"log ../etc/shadow\npoweroff\n",["not a path", "STOP_REQUEST=1"]),
    "long":(b"poweroff"+b"x"*100+b"\npoweroff\n",["Command too long", "STOP_REQUEST=1"]),
    "control":(b"power\x00off\npoweroff\n",["unsupported characters", "STOP_REQUEST=1"]),
    "escape":(b"power\x1boff\npoweroff\n",["unsupported characters", "STOP_REQUEST=1"]),
    "reboot":(b"reboot\n",["STOP_REQUEST=2"]),
    "blank":(b"\n\npoweroff\n",["STOP_REQUEST=1"]),
    "backspace":(b"helx\bp\npoweroff\n",["No shell", "STOP_REQUEST=1"]),
    "delete":(b"helx\x7fp\npoweroff\n",["No shell", "STOP_REQUEST=1"]),
    "empty-edit":(b"\b\x7fhelp\npoweroff\n",["No shell", "STOP_REQUEST=1"]),
    "crlf":(b"help\r\nhelp\rpoweroff\n",["No shell", "STOP_REQUEST=1"]),
    "overflow-edit":(b"poweroff"+b"x"*100+b"\b"*100+b"\nhelp\npoweroff\n",["Command too long", "No shell", "STOP_REQUEST=1"]),
    "invalid-edit":(b"poweroff\x1b\b\nhelp\npoweroff\n",["unsupported characters", "No shell", "STOP_REQUEST=1"]),
}
if len(sys.argv)!=3:raise SystemExit(2)
value,expected=CASES[sys.argv[2]]
result=subprocess.run([sys.argv[1]],input=value,capture_output=True,timeout=5,check=True)
text=result.stdout.decode()
for part in expected:
    if part not in text:raise SystemExit(f"Missing {part!r} in console result: {text}")
if sys.argv[2] == "crlf":
    # One initial prompt and one after each help; CRLF must not add a fourth.
    if text.count("umicom> ") != 3 or text.count("No shell") != 2:
        raise SystemExit(f"CRLF dispatch or following command changed: {text}")
print("Console command and rejection checks passed without power or mount operations.")
