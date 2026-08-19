# Byggscript för PlatformIO — anropas före C++-kompilering
# Kör ca65 + ld65 för att assemblera alla *.asm i asm/ → *.h i src/
# Inkluderas via platformio.ini: extra_scripts = pre:scripts/build_asm.py

Import("env")
import subprocess
import os

# Array-namn i genererade headers. program_hello.h behåller det historiska
# namnet PROGRAM[]; övriga får PROGRAM_<NAMN>[] (t.ex. PROGRAM_FIB[]).
ARRAY_NAMES = {
    "program_hello": "PROGRAM",
}

def build_asm(target, source, env):
    """Assemblera alla asm/*.asm → asm/*.bin → src/*.h via ca65/ld65/bin2h.py."""
    src_dir = os.path.join(env.subst("$PROJECT_DIR"), "src")
    asm_dir = os.path.join(env.subst("$PROJECT_DIR"), "asm")
    script_dir = os.path.join(env.subst("$PROJECT_DIR"), "scripts")
    cfg_path = os.path.join(asm_dir, "program.cfg")
    bin2h = os.path.join(script_dir, "bin2h.py")

    asm_files = sorted(f for f in os.listdir(asm_dir) if f.endswith(".asm"))
    if not asm_files:
        print("[build_asm] Inga .asm-filer i asm/ — hoppar över")
        return 0

    for asm_file in asm_files:
        base = os.path.splitext(asm_file)[0]
        asm_path = os.path.join(asm_dir, asm_file)
        obj_path = os.path.join(asm_dir, base + ".o")
        bin_path = os.path.join(asm_dir, base + ".bin")
        h_path = os.path.join(src_dir, base + ".h")
        arr_name = ARRAY_NAMES.get(base, "PROGRAM_" + base.upper())

        print(f"[build_asm] Assemblerar {asm_path} …")

        # Steg 1: ca65 (assembler → objektfil)
        result = subprocess.run(
            ["ca65", "-o", obj_path, asm_path],
            capture_output=True, text=True
        )
        if result.returncode != 0:
            print(f"[build_asm] ca65 FAILED ({asm_file}):")
            print(result.stderr)
            return result.returncode

        # Steg 2: ld65 (länka → binär)
        result = subprocess.run(
            ["ld65", "-C", cfg_path, "-o", bin_path, obj_path],
            capture_output=True, text=True
        )
        if result.returncode != 0:
            print(f"[build_asm] ld65 FAILED ({asm_file}):")
            print(result.stderr)
            return result.returncode

        # Steg 3: bin → C-header
        result = subprocess.run(
            ["python3", bin2h, bin_path, h_path, arr_name],
            capture_output=True, text=True
        )
        if result.returncode != 0:
            print(f"[build_asm] bin2h FAILED ({asm_file}):")
            print(result.stderr)
            return result.returncode

        print(f"[build_asm] OK — {os.path.getsize(bin_path)} bytes → {h_path} ({arr_name}[])")

    return 0

# Registrera som pre-build hook
env.AddPreAction("buildprog", build_asm)
