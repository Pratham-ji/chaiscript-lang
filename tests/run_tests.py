#!/usr/bin/env python3
"""run_tests.py — ChaiScript Compiler Test Suite

Runs every .chai file in tests/cases/, compiles it through the full pipeline,
and compares stdout against the corresponding .expected file.

Usage:
    python3 tests/run_tests.py
    python3 tests/run_tests.py --verbose
"""

import subprocess
import sys
import os
import platform
import shutil
import glob
import tempfile


# ── Color helpers ───────────────────────────────────────────────────────────

def supports_color():
    if os.environ.get("NO_COLOR"):
        return False
    if platform.system() == "Windows":
        return os.environ.get("WT_SESSION") or os.environ.get("TERM_PROGRAM")
    return hasattr(sys.stdout, "isatty") and sys.stdout.isatty()

USE_COLOR = supports_color()

def color(code, text):
    return f"\033[{code}m{text}\033[0m" if USE_COLOR else text

def green(t):  return color("1;32", t)
def red(t):    return color("1;31", t)
def cyan(t):   return color("1;36", t)
def dim(t):    return color("90", t)
def yellow(t): return color("1;33", t)


# ── Platform helpers ────────────────────────────────────────────────────────

def executable_ext():
    return ".exe" if platform.system() == "Windows" else ""

def detect_c_compiler():
    candidates = ["gcc", "cc", "clang"]
    if platform.system() == "Windows":
        candidates = ["gcc", "cl", "clang", "cc"]
    for cc in candidates:
        if shutil.which(cc):
            return cc
    return None


# ── Build the compiler ──────────────────────────────────────────────────────

def build_compiler(project_root, verbose=False):
    build_dir = os.path.join(project_root, "build")
    os.makedirs(build_dir, exist_ok=True)

    # Configure
    cmake_cmd = ["cmake", ".."]
    if shutil.which("ninja"):
        cmake_cmd += ["-G", "Ninja"]
    result = subprocess.run(cmake_cmd, cwd=build_dir,
                            capture_output=True, text=True)
    if result.returncode != 0:
        print(red("CMake configuration failed:"))
        print(result.stderr)
        sys.exit(1)

    # Build
    result = subprocess.run(["cmake", "--build", "."], cwd=build_dir,
                            capture_output=True, text=True)
    if result.returncode != 0:
        print(red("Build failed:"))
        print(result.stderr)
        sys.exit(1)

    compiler = os.path.join(build_dir, "chaiscript" + executable_ext())
    if not os.path.exists(compiler):
        print(red("Compiler binary not found after build."))
        sys.exit(1)

    return compiler


# ── Run a single test ───────────────────────────────────────────────────────

def run_test(compiler_bin, cc, chai_file, expected_file, verbose=False):
    """Compile and run a .chai file. Returns (passed, actual_output, expected_output)."""
    test_name = os.path.basename(chai_file).replace(".chai", "")

    # Check if this is an error test (expects compilation failure)
    is_error_test = test_name.startswith("test_error")

    with tempfile.TemporaryDirectory() as tmpdir:
        c_file = os.path.join(tmpdir, test_name + ".c")
        binary = os.path.join(tmpdir, test_name + executable_ext())

        # Step 1: Compile .chai → .c
        result = subprocess.run(
            [compiler_bin, chai_file, "-o", c_file],
            capture_output=True, text=True
        )

        if is_error_test:
            # Error tests: we expect the compiler to fail and produce diagnostics
            actual = result.stderr.strip()
            with open(expected_file, "r") as f:
                expected = f.read().strip()
            # Normalize ANSI codes for comparison
            import re
            actual_clean = re.sub(r'\033\[[0-9;]*m', '', actual)
            expected_clean = re.sub(r'\033\[[0-9;]*m', '', expected)
            passed = actual_clean == expected_clean
            return passed, actual_clean, expected_clean

        if result.returncode != 0:
            return False, f"COMPILE ERROR:\n{result.stderr.strip()}", ""

        # Step 2: Compile .c → binary
        if cc == "cl":
            cc_cmd = [cc, "/O2", c_file, f"/Fe:{binary}"]
        else:
            cc_cmd = [cc, "-O3", c_file, "-o", binary]

        result = subprocess.run(cc_cmd, capture_output=True, text=True)
        if result.returncode != 0:
            return False, f"C COMPILE ERROR:\n{result.stderr.strip()}", ""

        # Step 3: Execute binary
        result = subprocess.run([binary], capture_output=True, text=True, timeout=10)
        actual = result.stdout.strip()

        with open(expected_file, "r") as f:
            expected = f.read().strip()

        passed = actual == expected
        return passed, actual, expected


# ── Main ────────────────────────────────────────────────────────────────────

def main():
    verbose = "--verbose" in sys.argv or "-v" in sys.argv

    project_root = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
    tests_dir = os.path.join(project_root, "tests", "cases")

    if not os.path.isdir(tests_dir):
        print(red(f"Test directory not found: {tests_dir}"))
        sys.exit(1)

    # Detect C compiler
    cc = detect_c_compiler()
    if not cc:
        print(red("No C compiler found. Install gcc, clang, or MSVC."))
        sys.exit(1)

    # Build compiler
    print(cyan("\u2615 Building ChaiScript compiler...\n"))
    compiler_bin = build_compiler(project_root, verbose)

    # Find test files
    chai_files = sorted(glob.glob(os.path.join(tests_dir, "*.chai")))
    if not chai_files:
        print(yellow("No test files found in tests/cases/"))
        sys.exit(0)

    print(cyan(f"\u2615 Running {len(chai_files)} tests...\n"))

    passed_count = 0
    failed_count = 0
    failures = []

    for chai_file in chai_files:
        test_name = os.path.basename(chai_file).replace(".chai", "")
        expected_file = chai_file.replace(".chai", ".expected")

        if not os.path.exists(expected_file):
            print(yellow(f"  \u26a0  {test_name}: missing .expected file, skipping"))
            continue

        passed, actual, expected = run_test(
            compiler_bin, cc, chai_file, expected_file, verbose
        )

        if passed:
            print(green(f"  \u2713  {test_name}"))
            passed_count += 1
        else:
            print(red(f"  \u2717  {test_name}"))
            failed_count += 1
            failures.append((test_name, actual, expected))

    # Summary
    print()
    total = passed_count + failed_count
    if failed_count == 0:
        print(green(f"\u2705 All {total} tests passed! Chai is brewing perfectly. \u2615"))
    else:
        print(red(f"\u274c {failed_count}/{total} tests failed.\n"))
        for name, actual, expected in failures:
            print(red(f"  --- {name} ---"))
            print(f"  Expected:\n    {expected}")
            print(f"  Got:\n    {actual}\n")

    sys.exit(1 if failed_count > 0 else 0)


if __name__ == "__main__":
    main()
