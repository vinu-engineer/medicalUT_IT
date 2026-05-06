# Design Spec: Issue 37

Issue: `#37`  
Branch: `feature/37-session-alarm-event-review-log`  
Spec path: `docs/history/specs/37-session-alarm-event-review-log.md`

## Problem

The current dashboard and console summary show only two views of patient state:

- `src/gui_main.c` populates `IDC_LIST_HISTORY` with raw reading rows and
  `IDC_LIST_ALERTS` with alerts from only the latest reading.
- `src/patient.c` `patient_print_summary()` prints the latest vitals, the
  aggregate current status, and only the currently active alerts.

That means transient warning or critical episodes disappear from the concise
review surfaces as soon as the next reading improves. A clinician, tester, or
reviewer can reconstruct the sequence from raw readings, but not from a bounded
session alarm review that preserves clinically meaningful alert-state changes.

## Goal

Add a narrow, read-only session alarm event review log that:

- preserves alert-state transitions that occurred earlier in the current
  patient session
- reuses the existing validated alert engine as the single source of truth
- remains bounded by static storage and existing session-reset semantics
- exposes the same event history in both the GUI and `patient_print_summary()`

## Non-goals

- Changing vital-sign thresholds, NEWS2 logic, alarm-limit logic, or treatment
  guidance.
- Adding alarm acknowledgement, escalation workflow, clinician response timing,
  notifications, persistence beyond the current session, or multi-patient
  surveillance.
- Replacing the existing active-alert list or raw reading history.
- Introducing network transport, file export, EMR integration, or cloud sync.
- Creating a second alert-classification ruleset separate from
  `check_*()`, `overall_alert_level()`, and `generate_alerts()`.

## Intended Use Impact, User Population, Operating Environment, And Foreseeable Misuse

Intended use impact:

- The feature adds a review-only adjunct surface for historical alert-state
  changes within the active session.
- It does not change the primary live-monitoring or active-alert workflow.

User population:

- Trained clinical staff, internal testers, and reviewers using the local
  Windows workstation application.

Operating environment:

- The existing single-patient Win32 desktop workflow, using static in-process
  storage and either simulator-fed or manually entered readings.

Foreseeable misuse:

- Treating the event log as a complete longitudinal medical record.
- Confusing a historical event with a currently active alert.
- Assuming the log is complete across patient clear, logout, or automatic
  session reinitialization.
- Expecting acknowledgment, escalation, or response-tracking behavior from a
  read-only list.

## Current Behavior

- `PatientRecord` stores demographics plus up to `MAX_READINGS` raw
  `VitalSigns` entries, but no derived alert-event history.
- `patient_add_reading()` appends a reading or rejects it when the session
  buffer is full.
- `update_dashboard()` rebuilds:
  - a raw reading history list with one row per stored reading
  - an active-alert list generated only from `patient_latest_reading()`
- `patient_current_status()` reflects only the latest reading.
- `patient_print_summary()` prints only the current alert state and current
  active alerts.
- In simulation mode, the timer path calls `patient_init()` when
  `patient_is_full()` becomes true, which resets the session before the next
  reading is added.
- Manual clear, logout, simulation-off transitions, and new patient admission
  also reinitialize or clear the current patient/session state.

## Proposed Change

1. Extend the patient-session model with a bounded alert-event buffer stored
   inside `PatientRecord`, not in heap memory or external persistence.
2. Define one derived alert signature per successful reading:
   aggregate `AlertLevel` plus the set of abnormal parameters reported by the
   existing alert engine.
3. Append at most one event per successful `patient_add_reading()` call, and
   only when the new signature differs from the previous reading's signature.
4. Use these event rules:
   - first normal reading: no event
   - first abnormal reading in a session: create an event
   - abnormal-to-abnormal change with different severity or abnormal-parameter
     set: create an event
   - abnormal-to-normal recovery: create an event stating that active
     abnormalities cleared
   - unchanged signature across adjacent readings: no duplicate event
5. Keep the event buffer bounded by the existing session size:
   `MAX_ALERT_EVENTS` should equal `MAX_READINGS`, and event generation must be
   limited to one event per successful reading append. This avoids an
   independent event-overflow path and makes the reading/session limit the only
   retention boundary.
6. Each event record should store enough deterministic context for both GUI and
   console rendering:
   - session reading index (1-based)
   - resulting aggregate severity
   - abnormal-parameter signature or equivalent compact representation
   - a human-readable summary derived from current alert semantics
7. Prefer implementing event capture in `patient.c` so all ingestion paths
   (manual entry, simulator timer, scenario playback, CLI/demo flow) share one
   source of truth.
8. Add explicit patient-event accessors in `patient.h` rather than making GUI
   code rebuild event history from raw readings each repaint.
9. Add a dedicated GUI list labelled `Session Alarm Events` or equivalent,
   visually separate from both `Active Alerts` and `Reading History`.
10. Preserve the existing active-alert list as the live state surface. Historical
    entries must not use wording that implies they are current active alarms.
11. Extend `patient_print_summary()` with a `Session Alert Events` section using
    the same event records and ordering as the GUI list so DVT and console
    evidence stay aligned.
12. Clear the event log whenever the session is reinitialized:
    logout, manual clear, patient change/admit refresh, simulation disable, and
    automatic simulation rollover through `patient_init()`.
13. If implementation reveals that the GUI cannot fit a third list clearly in
    the current minimum window height, a modest layout adjustment is allowed,
    but the issue must remain a documentation-plus-read-only-review feature and
    must not expand into a broader dashboard redesign.

## Files Expected To Change

Expected requirements and traceability files:

- `requirements/UNS.md`
- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`

Expected implementation files:

- `include/patient.h`
- `src/patient.c`
- `include/localization.h`
- `src/localization.c`
- `src/gui_main.c`

Expected verification files:

- `tests/unit/test_patient.cpp`
- `tests/integration/test_patient_monitoring.cpp`
- `tests/integration/test_alert_escalation.cpp`
- `dvt/DVT_Protocol.md`
- `dvt/run_dvt.py`

Files expected not to change:

- Threshold-classification logic in `src/vitals.c`
- Alert-generation threshold rules in `src/alerts.c`
- NEWS2 logic in `src/news2.c`
- Alarm-limit logic in `src/alarm_limits.c`
- Authentication behavior beyond existing session clear boundaries

## Requirements And Traceability Impact

- Existing requirements directly impacted:
  - `UNS-009` Vital Sign History
  - `UNS-010` Consolidated Status Summary
  - `SYS-009` Vital Sign Reading History
  - `SYS-011` Patient Status Summary Display
  - `SYS-012` Static memory constraints
  - `SWR-PAT-002`, `SWR-PAT-003`, `SWR-PAT-006`
  - `SWR-GUI-003`, `SWR-GUI-004`
  - `SWR-ALT-001` through `SWR-ALT-004` as reused alert semantics
- New derived requirements are expected at SYS and SWR level for:
  - session alert-event capture trigger rules
  - event ordering and reading-index semantics
  - explicit clear/reset behavior
  - GUI presentation of historical events distinct from active alerts
  - console-summary parity with the GUI event list
- A new UNS-level statement is likely warranted if the team wants this
  retrospective event review called out as user-visible intended behavior rather
  than only as a refinement of reading history and summary requirements.
- Traceability must show that the event log is derived from existing alert
  semantics, not from newly invented clinical logic.
- This issue does not justify changing existing alert thresholds, NEWS2
  mappings, or alarm-limit acceptance criteria.

## Medical-Safety, Security, And Privacy Impact

Medical safety:

- No intended change to classification thresholds, alarm limits, NEWS2
  calculations, or treatment guidance.
- Primary benefit is retrospective visibility of transient instability during
  handoff, review, and DVT evidence.
- Primary risk is presentation confusion if historical entries are mistaken for
  current active alarms or if session-reset boundaries are unclear.
- The design remains acceptable only if the active-alert list stays primary and
  the historical log is clearly marked as session-scoped review data.

Security:

- No new authentication or authorization model is introduced.
- The event log must remain inside the current authenticated session boundary
  and clear whenever existing session/patient-reset paths clear patient data.

Privacy:

- No new network, cloud, or file-persistence path is introduced.
- The feature reuses patient-linked observations already present in the current
  dashboard and summary surfaces, so privacy impact is bounded to local display
  and local console evidence.

## AI/ML Impact Assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: not applicable
- Input data: existing deterministic vital-sign readings only
- Output: deterministic historical event records derived from rule-based alert
  logic
- Human-in-the-loop limits: unchanged from the current system
- Transparency needs: the UI must distinguish historical review events from
  active alerts, but this is a deterministic presentation concern, not an AI
  explainability concern
- Dataset and bias considerations: not applicable because no model training or
  inference is introduced
- Monitoring expectations: standard software verification and DVT only
- PCCP impact: none

## Validation Plan

Implementation should add targeted automated coverage plus GUI/DVT evidence.

Automated validation scope:

- unit tests for event creation on:
  - first abnormal reading
  - warning to critical escalation
  - critical to warning or normal recovery
  - abnormal-parameter-set change without severity change
  - repeated identical abnormal state with no duplicate event
  - patient/session reset paths clearing stored events
- integration tests proving:
  - a transient critical episode remains present in historical review after the
    latest reading returns to warning or normal
  - active alerts still reflect only the latest reading
  - GUI/summary event ordering matches reading order

Recommended validation commands:

```powershell
run_tests.bat
ctest --test-dir build --output-on-failure -R "Patient|Alert"
python dvt/run_dvt.py
```

Expected validation outcome:

- Documentation and code changes remain limited to requirements, patient/event
  handling, GUI presentation, localization, and verification assets.
- Automated tests confirm deterministic capture, reset, and no-duplicate rules.
- DVT or equivalent GUI evidence shows that a cleared critical event remains
  reviewable in the historical list while the active-alert list returns to the
  latest state only.
- Review evidence confirms that historical entries are clearly labeled and not
  visually interchangeable with active alerts.

## Rollback Or Failure Handling

- If implementation requires changing alert thresholds, NEWS2 behavior,
  persistence scope, or acknowledgment workflow, stop and split that work into
  a follow-on issue.
- If the team cannot make historical-versus-active status clear in the GUI
  within the current bounded scope, do not ship a confusing hybrid list; revert
  to the pre-feature state and keep the issue blocked pending UX clarification.
- If requirements updates cannot express the feature without over-claiming
  longitudinal retention or clinical decision support, narrow the wording to
  session-scoped review only.
- Rollback is straightforward because the intended runtime change is additive
  and session-local: remove the event-buffer/API/UI additions and restore the
  prior two-surface behavior of active alerts plus raw reading history only.
