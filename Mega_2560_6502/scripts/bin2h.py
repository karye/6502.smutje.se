#!/usr/bin/env python3
"""bin2h.py — konvertera program.bin till program.h (C-header med byte-array).

Användning:
    python3 bin2h.py program.bin program.h [ARRAY_NAMN]

Genererar:
    const unsigned char ARRAY_NAMN[] PROGMEM = { 0xA9, 0xFF, ... };
    const unsigned int ARRAY_NAMN_SIZE = ...;
"""

import sys

def bin2h(bin_path, h_path, arrname="PROGRAM"):
    with open(bin_path, 'rb') as f:
        data = f.read()

    lines = []
    lines.append('// Genererad från program.bin — redigera inte för hand')
    lines.append(f'// Storlek: {len(data)} bytes')
    lines.append('')
    lines.append('#include <avr/pgmspace.h>')
    lines.append('')
    lines.append(f'const unsigned char {arrname}[] PROGMEM = {{')

    hex_bytes = []
    for i, b in enumerate(data):
        hex_bytes.append(f'0x{b:02X}')
        if (i + 1) % 16 == 0:
            lines.append('    ' + ', '.join(hex_bytes) + ',')
            hex_bytes = []
    if hex_bytes:
        lines.append('    ' + ', '.join(hex_bytes))

    lines.append('};')
    lines.append(f'const unsigned int {arrname}_SIZE = {len(data)};')
    lines.append('')

    with open(h_path, 'w') as f:
        f.write('\n'.join(lines))

    print(f'bin2h: {bin_path} ({len(data)} bytes) → {h_path}')

if __name__ == '__main__':
    if len(sys.argv) not in (3, 4):
        print(f'Användning: {sys.argv[0]} program.bin program.h [ARRAY_NAMN]')
        sys.exit(1)
    name = sys.argv[3] if len(sys.argv) == 4 else 'PROGRAM'
    bin2h(sys.argv[1], sys.argv[2], name)
