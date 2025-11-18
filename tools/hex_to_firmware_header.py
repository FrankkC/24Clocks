#!/usr/bin/env python3
"""
Convert an Intel HEX file into a C header that matches the format used by
`Master/firmware_slave.h` (a const char array containing the hex file as ASCII
lines).

Usage:
    python3 hex_to_firmware_header.py input.hex output_header.h --symbol firmware_slave_hex

The generated header will include a include-guard and a
`const char <symbol>[] = "...";` with each HEX record as a separate string
literal terminated by `\n` (same style as the repo).

This script is careful about newline normalization and escaping quotes/backslashes
if present (they normally aren't in HEX files).

"""

import argparse
from pathlib import Path
import sys

TEMPLATE = '''#ifndef {guard}
#define {guard}

// Generated from: {src}
// DO NOT EDIT BY HAND - use tools/hex_to_firmware_header.py to regenerate

const char {symbol}[] =
{lines}
;

#endif // {guard}
'''


def make_guard(path: str, symbol: str) -> str:
    # create a conservative guard name
    name = Path(path).name.upper().replace('.', '_').replace('-', '_')
    guard = f"_{name}_{symbol.upper()}_H"
    return guard


def read_hex_lines(path: Path):
    with path.open('r', encoding='utf-8', errors='surrogateescape') as f:
        raw = f.read()
    # Normalize line endings and split into lines
    raw = raw.replace('\r\n', '\n').replace('\r', '\n')
    lines = raw.split('\n')
    # Remove any empty trailing line
    if len(lines) > 0 and lines[-1] == '':
        lines = lines[:-1]
    return lines


def escape_c_string(s: str) -> str:
    # Escape backslash and double quotes; hex lines typically don't contain these
    s = s.replace('\\', '\\\\')
    s = s.replace('"', '\\"')
    return s


def main():
    parser = argparse.ArgumentParser(description='Convert Intel HEX to C header array')
    parser.add_argument('input', help='Input .hex file')
    parser.add_argument('output', help='Output .h file to create/overwrite')
    parser.add_argument('--symbol', default='firmware_slave_hex', help='C symbol name')
    args = parser.parse_args()

    inp = Path(args.input)
    out = Path(args.output)

    if not inp.exists():
        print('ERROR: input file not found:', inp, file=sys.stderr)
        sys.exit(2)

    lines = read_hex_lines(inp)
    if not lines:
        print('ERROR: input file appears empty', file=sys.stderr)
        sys.exit(2)

    # Build C literal per line, each terminated with \n
    c_lines = []
    for L in lines:
        # Keep the exact line content but strip any stray trailing whitespace
        Ls = L.rstrip('\n')
        esc = escape_c_string(Ls)
        c_lines.append(f'"{esc}\\n"')

    # Join with newline and indentation
    joined = '\n'.join('    ' + cl for cl in c_lines)

    guard = make_guard(out.name, args.symbol)

    content = TEMPLATE.format(guard=guard, src=inp.name, symbol=args.symbol, lines=joined)

    out.parent.mkdir(parents=True, exist_ok=True)
    out.write_text(content, encoding='utf-8')
    print(f'Wrote {out} ({len(c_lines)} lines)')


if __name__ == '__main__':
    main()
