#!/usr/bin/env python3
"""Umicom OS composition of the Framework-owned session staging tool.
Sammy Hegab, Umicom Foundation. MIT. This wrapper launches no applications.
"""
from __future__ import annotations
import argparse
from pathlib import Path
import runpy
import sys

def main() -> None:
    parser = argparse.ArgumentParser(add_help=False)
    parser.add_argument("--framework", required=True)
    known, remaining = parser.parse_known_args()
    framework = Path(known.framework)
    if not framework.is_absolute():
        raise SystemExit("--framework must be an absolute path to the reviewed Framework checkout.")
    tool = framework / "scripts/stage-desktop-session.py"
    if not tool.is_file():
        raise SystemExit("The selected Framework checkout does not contain the session staging tool.")
    sys.argv = [str(tool), *remaining]
    runpy.run_path(str(tool), run_name="__main__")
if __name__ == "__main__":
    main()
