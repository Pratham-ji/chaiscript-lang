#!/usr/bin/env python3
"""brew_chai.py — The ChaiScript Build & Run Orchestrator

Automates the full compilation pipeline:
  Build C++ Compiler → Compile .chai → GCC Assemble → Execute

Usage:
    python3 tools/brew_chai.py examples/hello_chai.chai
"""

import subprocess
import sys
import os
import argparse


def run_cmd(cmd, label=None):
    """Execute a shell command with styled terminal output."""
    if label:
        print(f"\033[1;33m\U0001f449 {label}\033[0m")
    print(f"\033[90m   $ {' '.join(cmd)}\033[0m")
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.stdout.strip():
        print(result.stdout.rstrip())
    if result.returncode != 0:
        if result.stderr.strip():
            print(f"\033[1;31m{result.stderr.rstrip()}\033[0m")
        sys.exit(result.returncode)
    elif result.stderr.strip():
        print(f"\033[90m{result.stderr.rstrip()}\033[0m")
    return result


def main():
    ap = argparse.ArgumentParser(
        description="\U00002615 Brew your .chai source into a native executable"
    )
    ap.add_argument("chai_file", help="Path to the .chai source file")
    ap.add_argument("--keep", action="store_true", help="Keep intermediate .c file")
    args = ap.parse_args()

    # Resolve the source path before we cd into build/
    chai_abs = os.path.abspath(args.chai_file)
    if not os.path.exists(chai_abs):
        print(f"\033[1;31m\U0000274c File not found: {args.chai_file}\033[0m")
        sys.exit(1)

    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    build_dir = os.path.join(project_root, "build")
    os.makedirs(build_dir, exist_ok=True)
    os.chdir(build_dir)

    # ── Stage 1: Build the ChaiScript compiler ──
    print(f"\n\033[1;36m\U00002615 [1/4] Brewing the ChaiScript Compiler (C++20)...\033[0m")
    run_cmd(["cmake", "..", "-G", "Ninja"], "Configuring CMake")
    run_cmd(["ninja"], "Building with Ninja")

    compiler_bin = os.path.join(build_dir, "chaiscript")
    if not os.path.exists(compiler_bin):
        print("\033[1;31m\U0000274c Compiler binary not found after build.\033[0m")
        sys.exit(1)

    # ── Stage 2: Compile .chai → .c ──
    c_output = chai_abs.replace(".chai", ".c")
    print(f"\n\033[1;36m\U0001f375 [2/4] Steeping {os.path.basename(args.chai_file)} \U00002192 C...\033[0m")
    run_cmd([compiler_bin, chai_abs, "-o", c_output])

    # ── Stage 3: GCC assemble ──
    executable = c_output.replace(".c", ".out")
    print(f"\n\033[1;36m\U00002699\U0000fe0f  [3/4] Compiling generated C with gcc -O3...\033[0m")
    run_cmd(["gcc", "-O3", c_output, "-o", executable])

    # ── Stage 4: Execute ──
    print(f"\n\033[1;32m\U0001f680 [4/4] Serving fresh chai:\033[0m")
    print("\033[90m" + "\U00002500" * 40 + "\033[0m")
    result = subprocess.run([executable], capture_output=True, text=True)
    print(result.stdout, end="")
    print("\033[90m" + "\U00002500" * 40 + "\033[0m")

    # Cleanup intermediate .c unless --keep
    if not args.keep and os.path.exists(c_output):
        os.remove(c_output)

    print(f"\n\033[1;32m\U00002705 Chai served successfully. Aur chahiye?\033[0m")


if __name__ == "__main__":
    main()
