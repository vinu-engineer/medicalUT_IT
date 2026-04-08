#!/usr/bin/env bash
# =============================================================================
# run_pipeline.sh — Run the full CI/CD pipeline locally
#
# Replicates the GitHub Actions pipeline stages on a local machine.
# Requires: cmake, ninja, gcc, g++, python3, gcov
# Optional: gcovr (pip install gcovr) for HTML coverage
#
# Usage:
#   ./scripts/run_pipeline.sh          # run all stages
#   ./scripts/run_pipeline.sh --skip-analysis  # skip cppcheck/CodeQL
# =============================================================================

set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$REPO_ROOT"

SKIP_ANALYSIS=false
if [ "${1:-}" = "--skip-analysis" ]; then
    SKIP_ANALYSIS=true
fi

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m'

stage() { echo -e "\n${BLUE}════════════════════════════════════════════════════════════${NC}"; echo -e "${BLUE}  Stage $1: $2${NC}"; echo -e "${BLUE}════════════════════════════════════════════════════════════${NC}\n"; }
pass()  { echo -e "${GREEN}  ✓ $1${NC}"; }
fail()  { echo -e "${RED}  ✗ $1${NC}"; exit 1; }
warn()  { echo -e "${YELLOW}  ! $1${NC}"; }

# ── Stage 1: Build ────────────────────────────────────────────
stage 1 "Build All"
cmake -S . -B build -G Ninja \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    -DBUILD_TESTS=ON \
    -Wno-dev 2>&1 | tail -3
cmake --build build 2>&1
pass "Build succeeded"

# ── Stage 2: Static Analysis ─────────────────────────────────
if [ "$SKIP_ANALYSIS" = false ]; then
    stage 2 "Static Analysis"
    if command -v cppcheck &>/dev/null; then
        cppcheck --enable=all --std=c11 \
            --suppress=missingIncludeSystem \
            --suppress=unusedFunction \
            --suppress=missingInclude \
            --include=include \
            src/vitals.c src/alerts.c src/patient.c \
            src/gui_auth.c src/gui_users.c 2>&1 | tail -5
        pass "cppcheck completed"
    else
        warn "cppcheck not installed — skipping"
    fi
else
    warn "Stage 2 skipped (--skip-analysis)"
fi

# ── Stage 3: Tests ────────────────────────────────────────────
stage 3 "Unit & Integration Tests"
mkdir -p build/results dvt/results

echo "Running unit tests..."
./build/tests/test_unit.exe \
    --gtest_output=xml:build/results_unit.xml 2>&1 | tail -5
cp build/results_unit.xml dvt/results/ 2>/dev/null || true
pass "Unit tests passed"

echo "Running integration tests..."
./build/tests/test_integration.exe \
    --gtest_output=xml:build/results_integration.xml 2>&1 | tail -5
cp build/results_integration.xml dvt/results/ 2>/dev/null || true
pass "Integration tests passed"

# ── Stage 4: Coverage ─────────────────────────────────────────
stage 4 "Code Coverage"
cmake -S . -B build_cov -G Ninja \
    -DCMAKE_C_COMPILER=gcc \
    -DCMAKE_CXX_COMPILER=g++ \
    -DBUILD_TESTS=ON \
    -DENABLE_COVERAGE=ON \
    -Wno-dev 2>&1 | tail -3
cmake --build build_cov 2>&1 | tail -3

./build_cov/tests/test_unit.exe --gtest_brief=1 2>&1 | tail -3
./build_cov/tests/test_integration.exe --gtest_brief=1 2>&1 | tail -3

mkdir -p coverage-report
GCDA_DIR="build_cov/CMakeFiles/monitor_lib.dir/src"
GCDA_COUNT=$(find build_cov -name '*.gcda' 2>/dev/null | wc -l)
echo "Found $GCDA_COUNT .gcda files"

if [ "$GCDA_COUNT" -gt 0 ]; then
    for gcda in "$GCDA_DIR"/*.gcda; do
        [ -f "$gcda" ] && gcov -o "$GCDA_DIR" "$gcda" 2>/dev/null
    done

    # Try gcovr if available
    if command -v gcovr &>/dev/null; then
        gcovr --root . --filter 'src/' \
            --xml coverage-report/coverage.xml \
            --html-details coverage-report/index.html \
            --print-summary 2>&1
        pass "Coverage report generated (gcovr)"
    else
        echo "gcovr not installed — using gcov text output"
        # Parse gcov output with Python
        python3 -c "
import glob, os
total = covered = 0
for f in sorted(glob.glob('*.gcov')):
    src = f.replace('.gcov','')
    if not os.path.exists(os.path.join('src', src)):
        continue
    fl = fc = 0
    for line in open(f):
        parts = line.split(':', 2)
        if len(parts) < 3: continue
        c = parts[0].strip()
        if c == '-': continue
        fl += 1
        if c not in ('#####', '0'): fc += 1
    total += fl; covered += fc
    pct = fc/fl*100 if fl else 0
    print(f'  {src:25s}  {fc:4d}/{fl:4d}  ({pct:.1f}%)')
pct = covered/total*100 if total else 0
print(f'\n  TOTAL: {covered}/{total} lines ({pct:.1f}%)')
"
        pass "Coverage summary generated (gcov)"
    fi
else
    warn "No .gcda files found — coverage unavailable"
fi

# ── Stage 5: DVT Report ──────────────────────────────────────
stage 5 "DVT Report"
python3 - << 'PYEOF'
import os, datetime, xml.etree.ElementTree as ET

date_str = datetime.datetime.now().strftime("%Y-%m-%d_%H%M%S")
report_path = f"dvt/results/dvt_report_{date_str}.txt"

SWR_MAP = {
    "VitalsTest": "SWR-VIT-001..007", "AlertsTest": "SWR-ALT-001..004",
    "PatientTest": "SWR-PAT-001..006", "NEWS2Test": "SWR-NEW-001..005",
    "AlarmLimitsTest": "SWR-ALM-001..006", "TrendTest": "SWR-TRD-001..004",
    "AuthTest": "SWR-SEC-001..003, SWR-GUI-001..002",
    "UserManagementTest": "SWR-GUI-007", "LocalizationTest": "SWR-GUI-012",
    "AppConfigTest": "SWR-GUI-010", "HalTest": "SWR-VIT-008",
    "MonitoringIntegration": "SWR-VIT + SWR-ALT (end-to-end)",
    "EscalationIntegration": "SWR-ALT + SWR-NEW (escalation)",
}

def parse(path):
    try:
        root = ET.parse(path).getroot()
        t = int(root.get("tests", 0))
        f = int(root.get("failures", 0)) + int(root.get("errors", 0))
        return t, t - f, f
    except: return 0, 0, -1

def suites(path):
    result = []
    try:
        for ts in ET.parse(path).getroot().findall('.//testsuite'):
            t = int(ts.get('tests', 0))
            f = int(ts.get('failures', 0)) + int(ts.get('errors', 0))
            result.append((ts.get('name', '?'), t, t - f, f))
    except: pass
    return result

# Try both naming conventions
for u_path in ["dvt/results/results_unit.xml", "build/results_unit.xml"]:
    if os.path.exists(u_path): break
for i_path in ["dvt/results/results_integration.xml", "build/results_integration.xml"]:
    if os.path.exists(i_path): break

u_t, u_p, u_f = parse(u_path)
i_t, i_p, i_f = parse(i_path)
overall = "PASS" if (u_f == 0 and i_f == 0) else "FAIL"

lines = [
    "=" * 80, "  PATIENT VITAL SIGNS MONITOR — DVT REPORT (IEC 62304 Class B)",
    "=" * 80,
    f"  Generated: {datetime.datetime.now().strftime('%Y-%m-%d %H:%M:%S')}",
    "=" * 80, "",
    "1. TEST RESULTS", "-" * 80,
    f"  Unit:        {u_p:3d}/{u_t:3d}  {'PASS' if u_f==0 else 'FAIL'}",
    f"  Integration: {i_p:3d}/{i_t:3d}  {'PASS' if i_f==0 else 'FAIL'}",
    f"  OVERALL:     {overall}", "",
    "2. SWR TRACEABILITY", "-" * 80,
    f"  {'Suite':<28s} {'Tests':>5s} {'Pass':>5s} {'Fail':>5s}  SWR Requirements",
    "  " + "-" * 76,
]
for name, t, p, f in suites(u_path) + suites(i_path):
    swr = SWR_MAP.get(name, "(unmapped)")
    lines.append(f"  {name:<28s} {t:5d} {p:5d} {f:5d}  {swr}")
lines += ["", "3. MANUAL VERIFICATION", "-" * 80,
    "  SWR-GUI-001..006  GUI layout (structured demo)",
    "  SWR-GUI-008..011  Window management, persistence",
    "  IEC 60601-1-8     Alarm display colors", "",
    "  Protocol: dvt/DVT_Protocol.md (DVT-001-REV-A)",
    "  Traceability: requirements/TRACEABILITY.md",
    "=" * 80]

report = "\n".join(lines)
print(report)
os.makedirs("dvt/results", exist_ok=True)
with open(report_path, "w") as f:
    f.write(report + "\n")
print(f"\nReport: {report_path}")
PYEOF
pass "DVT report generated"

# ── Summary ───────────────────────────────────────────────────
echo ""
echo -e "${GREEN}════════════════════════════════════════════════════════════${NC}"
echo -e "${GREEN}  Pipeline complete — all stages passed${NC}"
echo -e "${GREEN}════════════════════════════════════════════════════════════${NC}"
echo ""
echo "  Reports:"
echo "    dvt/results/dvt_report_*.txt"
echo "    coverage-report/index.html  (if gcovr installed)"
echo "    build/results_unit.xml"
echo "    build/results_integration.xml"
