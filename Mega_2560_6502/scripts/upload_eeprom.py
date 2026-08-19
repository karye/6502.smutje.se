#!/usr/bin/env python3
"""upload_eeprom.py — skicka .bin-fil till AT28C256 EEPROM-programmerare."""

import serial
import sys
import time

PAGE_SIZE = 256

def upload(bin_path, port, baud=115200):
    with open(bin_path, 'rb') as f:
        data = f.read()

    if len(data) > 32768:
        print(f"FEL: {bin_path} är {len(data)} bytes — max 32768")
        sys.exit(1)

    print(f"Öppnar {port}...")
    ser = serial.Serial(port, baud, timeout=5)
    time.sleep(2)  # Vänta på Arduino-reset

    # Skicka sida för sida
    total_pages = (len(data) + PAGE_SIZE - 1) // PAGE_SIZE
    for i in range(total_pages):
        start = i * PAGE_SIZE
        end = min(start + PAGE_SIZE, len(data))
        page = data[start:end]

        # PADDA sista sidan till 256 bytes med 0xFF (tom EEPROM-byte)
        if len(page) < PAGE_SIZE:
            page = page + b'\xFF' * (PAGE_SIZE - len(page))

        ser.write(b'P')           # PAGE-kommando
        ser.write(page)           # 256 bytes data
        ser.flush()

        # Läs bekräftelse
        response = ser.readline().decode().strip()
        print(f"Sida {i+1}/{total_pages}: {response}")

    # Signalera klart
    ser.write(b'D')               # DONE
    ser.flush()

    print("Klar! Väntar på verifiering...")
    while True:
        line = ser.readline().decode().strip()
        if not line:
            break
        print(line)

    ser.close()
    print("Klart.")

if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f"Använd: {sys.argv[0]} program.bin /dev/ttyACM0")
        sys.exit(1)
    upload(sys.argv[1], sys.argv[2])
