# Design Spec: Show patient location in the dashboard header

Issue: #96
Branch: `feature/96-show-patient-location-in-the-dashboard-header`
Spec path: `docs/history/specs/96-show-patient-location-in-the-dashboard-header.md`

## Problem

Issue #96 requests an optional patient location or bed label so operators can
confirm patient context at a glance. The current single-patient flows expose
patient name, ID, age, BMI, and reading count, but they do not expose any room,
bed, or location cue.

The current implementation gap is concrete:

- `include/patient.h` `PatientInfo` stores only `id`, `name`, `age`,
  `weight_kg`, and `height_m`
- `src/gui_main.c` `paint_patient_bar()` shows patient context below the header
  but omits location
- `src/patient.c` `patient_print_summary()` prints name, ID, age, BMI, and
  readings but omits location
- all patient/session transitions still flow through `patient_init()` and
  related GUI reset paths, so any new context field must be cleared or refreshed
  consistently to avoid stale patient-context carryover

This is a context-display feature only. It must not expand into persistence,
multi-patient census behavior, alerting changes, NEWS2 changes, or external
system integration.

## Goal

Add one optional, session-scoped patient location field that can hold a short
room/bed/location label and render it consistently on the existing patient
context surfaces without changing clinical logic.

The intended outcome is:

- operators can enter a short optional location label when admitting or
  refreshing a patient
- the dashboard's header-adjacent patient context bar shows that location as a
  secondary cue next to the existing patient identity details
- `patient_print_summary()` shows the same location value using the same source
  field
- blank or missing location data renders as an explicit unset state rather than
  an implied verified assignment
- every patient/session reset path clears or replaces the field so stale
  context does not survive into the next patient session

## Non-goals

- No change to vital classification, alert generation, NEWS2 scoring, trend
  logic, configurable alarm limits, authentication, or localization behavior
  outside the new location strings needed for this feature.
- No multi-patient board, ward map, central-station clone, or bed-management
  workflow.
- No persistence to `monitor.cfg`, `users.dat`, or any new file format.
- No ADT, EMR, network sync, import/export, or remote-display integration.
- No claim that location verifies patient identity by itself.
- No vendor-specific layout copying or branded central-station mimicry.

## Intended use impact, user population, operating environment, and foreseeable misuse

Intended use impact:

- The feature adds a secondary patient-context cue to support local operator
  orientation within the existing single-patient dashboard and summary flows.
- The field is informational only. It does not alter therapy, diagnosis,
  alarms, risk scoring, or data acquisition.

User population:

- clinical users and internal testers operating the Win32 dashboard

Operating environment:

- a single authenticated workstation session using simulated or future
  hardware-fed vital signs

Foreseeable misuse:

- relying on location instead of cross-checking patient name and ID
- leaving stale location text in the admit form or patient record when
  switching patients
- entering free-text notes that are not actually room/bed/location context
- treating an unset value as a confirmed assignment

## Current behavior

Today the patient-context surfaces behave as follows:

- `patient_init()` initializes patient demographics but has no location
  parameter or storage
- `do_admit()` in `src/gui_main.c` gathers ID, name, age, weight, and height
  only
- `paint_patient_bar()` prints `Patient`, `ID`, `Age`, `BMI`, and `Readings`
  when a patient is active
- `patient_print_summary()` prints `Name`, `ID`, `Age`, `BMI`, and `Readings`
  before any vital-sign and alert detail
- `do_clear()`, demo-scenario setup, console demo paths in `src/main.c`, and
  automatic patient/session reinitialization paths do not manage any location
  field because none exists

## Proposed change

Implement the issue as a narrow, session-only patient-context enhancement with
these decisions:

1. Extend `PatientInfo` with one optional bounded string field,
   `location[MAX_LOCATION_LEN]`, where `MAX_LOCATION_LEN` should be a small
   static bound such as 32 bytes including the terminator.
2. Update `patient_init()` to accept a location argument in addition to the
   current identity and biometric fields. This signature change is intentional:
   it forces every patient/session initialization path to compile-break until it
   explicitly decides what location value to use.
3. Treat `NULL` or empty location input as "unset". Store an empty string in
   the record and render an explicit UI/text fallback such as `Location: Not
   set` rather than inventing a synthetic room/bed value.
4. Keep the field session-scoped only. Do not persist it to config or account
   storage, and do not retain it across logout, clear-session, automatic
   rollover reset, or fresh patient admission.
5. Add one labeled dashboard edit control for location in the admit/refresh
   area. The label should be `Location` and the value may hold either a room
   label, a bed label, or a combined short string such as `Ward 2 / Bed 5`.
6. Show the value in `paint_patient_bar()` rather than the already crowded top
   operator header. The patient bar is the existing patient-context strip, so
   it satisfies the "header glance" need without mixing patient context into
   role/auth/session controls.
7. Keep patient name and ID visually primary by ordering the patient-bar copy
   as patient identity first and location second.
8. Show the same location value in `patient_print_summary()` immediately after
   the patient ID section so the text summary and dashboard stay in sync.
9. Clear the visible location edit control as well as the stored
   `PatientRecord.info.location` on `do_clear()` and any path that resets the
   patient session. Demo and auto-created patients should use an explicit empty
   location unless a future issue adds clearly synthetic demo defaults.
10. Keep the field entirely out of `overall_alert_level()`, `generate_alerts()`,
    `news2_calculate()`, trend logic, alarm-limit logic, and session alarm
    event generation.
11. Preserve the existing static-memory discipline by using bounded stack/local
    copies in `patient_init()` for both `name` and `location` so self-reinit
    patterns remain safe when callers pass existing record buffers back in.

## Files expected to change

Expected production/documentation files:

- `include/patient.h`
- `src/patient.c`
- `src/gui_main.c`
- `src/main.c`
- `include/localization.h`
- `src/localization.c`
- `README.md`
- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`

Expected tests:

- `tests/unit/test_patient.cpp`
- `tests/unit/test_trend.cpp`
- `tests/unit/test_localization.cpp`
- `tests/integration/test_patient_monitoring.cpp`
- `tests/integration/test_alert_escalation.cpp`

Expected design-control files:

- `docs/history/risk/96-show-patient-location-in-the-dashboard-header.md`
- `docs/history/specs/96-show-patient-location-in-the-dashboard-header.md`

Files to inspect but not modify unless implementation discovers hidden coupling:

- `src/vitals.c`
- `src/alerts.c`
- `src/news2.c`
- `src/alarm_limits.c`
- `src/gui_users.c`
- `src/app_config.c`

## Requirements and traceability impact

This issue should extend existing requirements rather than introduce a new
clinical capability.

Planned requirement updates:

- `SYS-008` should add an optional bounded location field to the stored patient
  demographic set
- `SYS-011` should state that the formatted patient summary includes the
  location field or an explicit unset disclosure
- `SYS-014` should state that the dashboard patient-context display includes the
  location field as a secondary cue
- `SWR-PAT-001` should cover location-field initialization, truncation, null
  termination, and empty-state handling
- `SWR-PAT-006` should cover summary rendering of the location field
- `SWR-GUI-003` should cover patient-bar rendering of the location field while
  preserving patient name and ID as the primary identity anchors
- `SWR-GUI-004` should cover the new location input control and its clear/reset
  behavior across admit, demo, and session-reset flows

Traceability expectations:

- reuse existing `UNS-008`, `UNS-010`, and `UNS-014` links
- update RTM rows for the modified SYS/SWR entries
- update any `@req` comments that reference the touched patient/gui functions
- do not create new alerting, NEWS2, AI, or security requirement IDs for this
  display-only field unless implementation scope expands

## Medical-safety, security, and privacy impact

Medical-safety impact is low but real because stale location text could
contribute to wrong-patient context during review or handoff.

Required safety controls:

- location remains secondary to patient name and ID
- unset location is shown explicitly
- every patient/session initialization path clears or replaces the field
- no clinical calculation or alert behavior depends on the field

Security impact is low. The field remains inside the authenticated local
session and does not introduce new privileges, files, or network endpoints.

Privacy impact is low to moderate because room/bed context can increase patient
identifiability. That is why this design keeps the field session-only and
avoids persistence or export.

## AI/ML impact assessment

This issue does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: none
- Input data: none beyond a manually entered location string
- Output: none
- Human-in-the-loop limits: unchanged
- Transparency needs: unchanged
- Dataset and bias considerations: not applicable
- Monitoring expectations: unchanged
- PCCP impact: none

## Validation plan

Implementation validation should cover data-model safety, reset behavior, UI
display parity, and requirements updates.

Automated checks:

```powershell
run_tests.bat
```

Targeted regression checks:

```powershell
rg -n "patient_init\\(" src tests
git diff -- requirements/SYS.md requirements/SWR.md requirements/TRACEABILITY.md
```

Expected automated test updates:

- unit tests for `patient_init()` location truncation, empty-state handling, and
  self-reinit behavior
- unit tests for `patient_print_summary()` output containing either the entered
  location or the explicit unset disclosure
- localization smoke tests for the new label/unset strings if localization
  tables are extended
- integration tests updated only as needed to match the new `patient_init()`
  signature and any summary expectations

Manual GUI verification:

- admit a patient with a non-empty location and confirm patient bar rendering
- admit a patient with a blank location and confirm `Location: Not set`
- run `Clear Session` and confirm both stored state and visible input reset
- trigger both demo scenarios and confirm they do not inherit a stale location
- verify automatic rollover/session-reset paths do not retain the previous
  location
- verify logout/login starts from a clean session with no retained location

Scope guard:

- if implementation discovers a requirement to persist, sync, or validate the
  location against an external source, stop and open a follow-on design issue
  instead of widening this MVP silently

## Rollback or failure handling

If implementation causes header/patient-bar crowding, keep the location in the
patient bar and remove any attempt to add it to the top operator header rather
than compressing role/auth/session controls.

If compile churn from the `patient_init()` signature proves wider than expected,
do not revert to ad hoc setters. The signature change is the main guard against
stale-path omissions. Finish updating the known call sites or pause and
escalate with the exact blockers.

If reviewers reject the session-only decision and require persistence or
external synchronization, stop the implementation and return the issue to
design with that broader scope called out explicitly.
