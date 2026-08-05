#!/usr/bin/env python3
"""bin2h.py — konvertera program.bin till program.h (C-header med byte-array).

Användning:
    python3 bin2h.py program.bin program.h

Genererar:
    const unsigned char PROGRAM[] PROGMEM = { 0xA9, 0xFF, ... };
    const unsigned int PROGRAM_SIZE = ...;
"""

import sys
import os

def bin2h(bin_path, h_path):
    with open(bin_path, 'rb') as f:
        data = f.read()

    lines = []
    lines.append('// Genererad från program.bin — redigera inte för hand')
    lines.append(f'// Storlek: {len(data)} bytes')
    lines.append('')
    lines.append('#include <avr/pgmspace.h>')
    lines.append('')
    lines.append(f'const unsigned char PROGRAM[] PROGMEM = {{')

    hex_bytes = []
    for i, b in enumerate(data):
        hex_bytes.append(f'0x{b:02X}')
        if (i + 1) % 16 == 0:
            lines.append('    ' + ', '.join(hex_bytes) + ',')
            hex_bytes = []
    if hex_bytes:
        lines.append('    ' + ', '.join(hex_bytes))

    lines.append('};')
    lines.append(f'const unsigned int PROGRAM_SIZE = {len(data)};')
    lines.append('')

    with open(h_path, 'w') as f:
        f.write('\n'.join(lines))

    print(f'bin2h: {bin_path} ({len(data)} bytes) → {h_path}')

if __name__ == '__main__':
    if len(sys.argv) != 3:
        print(f'Användning: {sys.argv[0]} program.bin program.h')
        sys.exit(1)
    bin2h(sys.argv[1], sys.argv[2])
