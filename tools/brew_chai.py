#!/usr/bin/env python3
"""brew_chai.py — The ChaiScript Build & Run Orchestrator

Automates the full compilation pipeline on any OS:
  Build C++ Compiler → Compile .chai → C Compiler Assemble → Execute

Usage:
    python3 tools/brew_chai.py examples/hello_chai.chai
    python3 tools/brew_chai.py examples/hello_chai.chai --keep --verbose
"""

import subprocess
import sys
import os
import argparse
import platform
import shutil


# ── Color helpers (gracefully degrade on Windows cmd without ANSI support) ──

def supports_color():
    """Check if the terminal supports ANSI color codes."""
    if os.environ.get("NO_COLOR"):
        return False
    if platform.system() == "Windows":
        # Windows Terminal and modern cmd support ANSI, older cmd doesn't
        return os.environ.get("WT_SESSION") or os.environ.get("TERM_PROGRAM")
    return hasattr(sys.stdout, "isatty") and sys.stdout.isatty()

USE_COLOR = supports_color()

def color(code, text):
    return f"\033[{code}m{text}\033[0m" if USE_COLOR else text

def bold_cyan(t):    return color("1;36", t)
def bold_green(t):   return color("1;32", t)
def bold_red(t):     return color("1;31", t)
def dim(t):          return color("90", t)


# ── Platform Detection ──────────────────────────────────────────────────────

def detect_cmake_generator():
    """Auto-detect the best available CMake generator for the current OS."""
    if shutil.which("ninja"):
        return "Ninja"
    if platform.system() == "Windows":
        if shutil.which("nmake"):
            return "NMake Makefiles"
        # Fall back to default (Visual Studio generator)
        return None
    # Unix: fall back to Make
    if shutil.which("make"):
        return "Unix Makefiles"
    return None


def detect_c_compiler():
    """Find a working C compiler on the system."""
    candidates = ["gcc", "cc", "clang"]
    if platform.system() == "Windows":
        candidates = ["gcc", "cl", "clang", "cc"]
    for cc in candidates:
        if shutil.which(cc):
            return cc
    return None


def executable_extension():
    """Return the platform-appropriate executable extension."""
    return ".exe" if platform.system() == "Windows" else ""


# ── Command Runner ──────────────────────────────────────────────────────────

def run_cmd(cmd, label=None, cwd=None, verbose=False):
    """Execute a shell command with styled terminal output."""
    if label:
        print(f"  {label}")
    if verbose:
        print(dim(f"   $ {' '.join(cmd)}"))

    result = subprocess.run(cmd, capture_output=True, text=True, cwd=cwd)

    if result.stdout.strip():
        if verbose:
            print(result.stdout.rstrip())
    if result.returncode != 0:
        if result.stderr.strip():
            print(bold_red(result.stderr.rstrip()))
        sys.exit(result.returncode)
    elif result.stderr.strip() and verbose:
        print(dim(result.stderr.rstrip()))

    return result


# ── Main Pipeline ───────────────────────────────────────────────────────────

def main():
    ap = argparse.ArgumentParser(
        description="\u2615 Brew your .chai source into a native executable"
    )
    ap.add_argument("chai_file", help="Path to the .chai source file")
    ap.add_argument("--keep", action="store_true",
                    help="Keep intermediate .c file after compilation")
    ap.add_argument("--verbose", "-v", action="store_true",
                    help="Show detailed build commands")
    args = ap.parse_args()

    chai_abs = os.path.abspath(args.chai_file)
    if not os.path.exists(chai_abs):
        print(bold_red(f"\u274c File not found: {args.chai_file}"))
        sys.exit(1)

    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    build_dir = os.path.join(project_root, "build")
    os.makedirs(build_dir, exist_ok=True)

    # ── Stage 1: Build the ChaiScript compiler ──────────────────────────────
    print(f"\n{bold_cyan('\u2615 [1/4] Brewing the ChaiScript Compiler (C++20)...')}")

    generator = detect_cmake_generator()
    cmake_cmd = ["cmake", ".."]
    if generator:
        cmake_cmd += ["-G", generator]

    run_cmd(cmake_cmd, "Configuring CMake...", cwd=build_dir, verbose=args.verbose)

    # Use cmake --build for universal build (works with Ninja, Make, MSBuild)
    run_cmd(["cmake", "--build", "."], "Building...", cwd=build_dir, verbose=args.verbose)

    compiler_bin = os.path.join(build_dir, "chaiscript" + executable_extension())
    if not os.path.exists(compiler_bin):
        print(bold_red("\u274c Compiler binary not found after build."))
        sys.exit(1)

    # ── Stage 2: Compile .chai → .c ─────────────────────────────────────────
    c_output = chai_abs.rsplit(".", 1)[0] + ".c"
    basename = os.path.basename(args.chai_file)
    print(f"\n{bold_cyan(f'\U0001f375 [2/4] Steeping {basename} \u2192 C...')}")
    run_cmd([compiler_bin, chai_abs, "-o", c_output], cwd=build_dir, verbose=args.verbose)

    # ── Stage 3: C compiler assemble ────────────────────────────────────────
    ext = executable_extension()
    executable = chai_abs.rsplit(".", 1)[0] + ext
    cc = detect_c_compiler()
    if not cc:
        print(bold_red("\u274c No C compiler found. Install gcc, clang, or MSVC."))
        sys.exit(1)

    print(f"\n{bold_cyan(f'\u2699\ufe0f  [3/4] Compiling generated C with {cc}...')}")

    if cc == "cl":
        # MSVC uses different flags
        run_cmd([cc, "/O2", c_output, f"/Fe:{executable}"], verbose=args.verbose)
    else:
        run_cmd([cc, "-O3", c_output, "-o", executable], verbose=args.verbose)

    # ── Stage 4: Execute ────────────────────────────────────────────────────
    print(f"\n{bold_green('\U0001f680 [4/4] Serving fresh chai:')}")
    separator = dim("\u2500" * 50)
    print(separator)
    result = subprocess.run([executable], capture_output=True, text=True)
    print(result.stdout, end="")
    if result.stderr.strip():
        print(bold_red(result.stderr.rstrip()))
    print(separator)

    # Cleanup intermediate .c unless --keep
    if not args.keep and os.path.exists(c_output):
        os.remove(c_output)

    exit_code = result.returncode
    if exit_code == 0:
        print(f"\n{bold_green(f'\u2705 Chai served successfully (exit code {exit_code}). Aur chahiye?')}")
    else:
        print(f"\n{bold_red(f'\u274c Chai spilled! Process exited with code {exit_code}.')}")

    sys.exit(exit_code)


if __name__ == "__main__":
    main()
