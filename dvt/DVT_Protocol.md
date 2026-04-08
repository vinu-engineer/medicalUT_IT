# Design Verification Test (DVT) Protocol

**Document ID:** DVT-001-REV-A  
**Project:** Patient Vital Signs Monitor  
**Version Under Test:** 2.0.0  
**Date:** 2026-04-07  
**Standard:** IEC 62304 §5.7, §5.8  
**Author:** vinu-engineer  
**Status:** Approved  

---

## 1. Purpose

This protocol defines the automated Design Verification Tests (DVT) for the
Patient Vital Signs Monitor software.  DVT confirms that the software meets its
Software Requirements Specification (SWR-001) before release.  All tests are
executed by the CI pipeline; results are captured in `dvt/results/`.

---

## 2. Scope

| Layer | Executable | Requirements |
|-------|-----------|--------------|
| Unit tests | `test_unit.exe` | SWR-VIT-001..007, SWR-ALT-001..004, SWR-PAT-001..008, SWR-SEC-001..004 |
| Integration tests | `test_integration.exe` | SWR-PAT-007, SWR-ALT-003, SWR-SEC-001..003 |

GUI-only requirements (SWR-GUI-001..010) are verified by manual inspection
and recorded in the GUI Verification Checklist (§4).

---

## 3. Entry Criteria

1. Source code is on the `main` branch and all CI checks are green.
2. CMake configures successfully with `BUILD_TESTS=ON`.
3. No open P1/P2 defects against the version under test.

---

## 4. GUI Verification Checklist

The following items are verified manually by a tester and recorded in the
DVT results artefact.  Each line maps to one or more SWR requirements.

| ID | Requirement | Test Action | Expected Result | Pass/Fail |
|----|-------------|-------------|-----------------|-----------|
| GUI-01 | SWR-GUI-001 | Launch application | Login window opens with title "Patient Vital Signs Monitor" | |
| GUI-02 | SWR-GUI-001 | Enter wrong password | Error message displayed, password field cleared | |
| GUI-03 | SWR-GUI-001 | Enter correct admin credentials | Dashboard opens, "ADMIN" role badge shown in header | |
| GUI-04 | SWR-GUI-002 | Observe dashboard | Vital tiles (HR, BP, Temp, SpO2) update every ~2 s in simulation mode | |
| GUI-05 | SWR-GUI-003 | Set HR > 150 manually, click Add Reading | HR tile turns red (CRITICAL), alert list updated | |
| GUI-06 | SWR-GUI-004 | Observe status banner with critical reading | Banner shows red "CRITICAL" text | |
| GUI-07 | SWR-GUI-005 | Code review: `hw_vitals.h` / `sim_vitals.c` | HAL interface present; sim back-end swappable | |
| GUI-08 | SWR-GUI-006 | Observe "* SIM LIVE" indicator in header | Flips to "SIM PAUSED" when Pause Sim pressed | |
| GUI-09 | SWR-GUI-007 | Login as admin, open Settings > Users | User list shown; Add/Remove/Set Password buttons present | |
| GUI-10 | SWR-GUI-008 | Verify application icon in taskbar | Icon matches resources/app.ico | |
| GUI-11 | SWR-GUI-009 | Settings > About tab | Version v2.0.0, IEC 62304 Class B, SHA-256 credential note | |
| GUI-12 | SWR-GUI-010 | Click "Sim: ON" to toggle off | All tiles show "N/A"; patient bar shows "DEVICE MODE" | |
| GUI-13 | SWR-GUI-010 | Click "Sim: OFF" to toggle on | Simulation resumes; vitals update; tiles show live values | |
| GUI-14 | SWR-GUI-010 | Restart app; check sim state | Persisted sim_enabled loaded from monitor.cfg | |
| GUI-15 | SWR-SEC-001 | Login with valid / invalid credentials | Only valid credentials succeed | |
| GUI-16 | SWR-SEC-003 | Change password (My Account) | New password required ≥ 8 chars; old password verified | |
| GUI-17 | — | Resize dashboard window | All painted zones scale; list boxes stretch; buttons reanchor | |
| GUI-18 | — | Maximise dashboard window | All zones fill screen; no clipping or overlap | |

---

## 5. Automated Test Cases

Automated tests are executed by the DVT workflow (`.github/workflows/dvt.yml`).
The following table maps each test suite to the SWR it verifies.

### 5.1 Unit Tests (`test_unit.exe`)

| Test Group | Count | SWR IDs |
|------------|-------|---------|
| VitalsTest (check_heart_rate, check_blood_pressure, check_temperature, check_spo2, alert_level_str, calculate_bmi, bmi_category) | 34 | SWR-VIT-001..007 |
| AlertsTest (generate_alerts, overall_alert_level, format_alert_message) | 28 | SWR-ALT-001..004 |
| PatientTest (patient_init, patient_add_reading, patient_is_full, patient_latest_reading, patient_current_status, patient_print_summary) | 47 | SWR-PAT-001..008 |
| UsersTest (init, authenticate, change_password, admin_set_password, add, remove, count, get_by_index, save) | 12 | SWR-SEC-001..004 |
| **Total** | **121** | |

### 5.2 Integration Tests (`test_integration.exe`)

| Test Group | Count | SWR IDs |
|------------|-------|---------|
| PatientMonitoringFlow (admit, add readings, check status) | 8 | SWR-PAT-007, SWR-PAT-008 |
| AlertEscalation (escalate + critical boundary) | 13 | SWR-ALT-003, SWR-ALT-004 |
| **Total** | **21** | |

---

## 6. Pass Criteria

| Criterion | Threshold |
|-----------|-----------|
| Unit test pass rate | 100 % (121/121) |
| Integration test pass rate | 100 % (21/21) |
| GUI checklist items | All marked Pass |
| Static analysis (cppcheck) | 0 errors, 0 warnings |
| SAST (CodeQL security-extended) | 0 open alerts |

---

## 7. Exit Criteria

1. All automated tests pass (as recorded in `dvt/results/dvt_report_<date>.txt`).
2. GUI checklist completed and signed off.
3. No regressions vs. previous release.
4. DVT results committed to `dvt/results/` in the release commit.

---

## 8. Traceability

Full requirement-to-test traceability is maintained in
`requirements/TRACEABILITY.md` (RTM-001-REV-E).

---

## Revision History

| Rev | Date       | Author        | Description              |
|-----|------------|---------------|--------------------------|
| A   | 2026-04-07 | vinu-engineer | Initial DVT protocol — v2.0.0 |
