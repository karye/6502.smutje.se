# Byggscript för PlatformIO — anropas före C++-kompilering
# Kör ca65 + ld65 för att assemblera program_hello.hello.asm → program_hello.h
# Inkluderas via platformio.ini: extra_scripts = pre:scripts/build_asm.py

Import("env")
import subprocess
import os

def build_asm(target, source, env):
    """Assemblera program_hello.hello.asm → program_hello.hello.bin → program_hello.h"""
    src_dir = os.path.join(env.subst("$PROJECT_DIR"), "src")
    asm_dir = os.path.join(env.subst("$PROJECT_DIR"), "asm")

    asm_path = os.path.join(asm_dir, "program_hello.hello.asm")
    obj_path = os.path.join(asm_dir, "program_hello.hello.o")
    bin_path = os.path.join(asm_dir, "program_hello.hello.bin")
    h_path   = os.path.join(src_dir, "program_hello.h")
    cfg_path = os.path.join(asm_dir, "program.cfg")

    print(f"[build_asm] Assemblerar {asm_path} …")

    # Steg 1: ca65 (assembler → objektfil)
    result = subprocess.run(
        ["ca65", "-o", obj_path, asm_path],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        print("[build_asm] ca65 FAILED:")
        print(result.stderr)
        return result.returncode

    # Steg 2: ld65 (länka → binär)
    result = subprocess.run(
        ["ld65", "-C", cfg_path, "-o", bin_path, obj_path],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        print("[build_asm] ld65 FAILED:")
        print(result.stderr)
        return result.returncode

    # Steg 3: bin → C-header
    script_dir = os.path.join(env.subst("$PROJECT_DIR"), "scripts")
    result = subprocess.run(
        ["python3", os.path.join(script_dir, "bin2h.py"), bin_path, h_path],
        capture_output=True, text=True
    )
    if result.returncode != 0:
        print("[build_asm] bin2h FAILED:")
        print(result.stderr)
        return result.returncode

    print(f"[build_asm] OK — {os.path.getsize(bin_path)} bytes → {h_path}")
    return 0

# Registrera som pre-build hook
env.AddPreAction("buildprog", build_asm)
