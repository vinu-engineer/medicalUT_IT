# Design Spec: Issue 84

Issue: `#84`  
Branch: `feature/84-session-alarm-severity-summary-strip`  
Spec path: `docs/history/specs/84-session-alarm-severity-summary-strip.md`

## Problem

The dashboard already preserves session-scoped alert-state transitions in the
`Session Alarm Events` list, but a reviewer still has to scan each row to judge
whether the session was dominated by warning transitions, brief critical
episodes, or repeated recoveries.

Current surfaces expose:

- `src/gui_main.c` `update_dashboard()` populates `IDC_LIST_EVENTS` with one
  row per stored `AlertEvent`.
- `src/patient.c` stores only transition events, not a pre-aggregated severity
  summary.
- `IDC_LIST_ALERTS` remains a current-state surface driven only by the latest
  reading.

That means the repo already has the right data, but not the compact
session-review summary the issue requests.

## Goal

Add a small, read-only severity summary strip above `Session Alarm Events` that:

- counts the current session's stored `AlertEvent` rows by resulting severity
- stays deterministic and session-scoped
- preserves the detailed event list directly below it
- keeps current active alerts visually primary over retrospective review data
- makes the `NORMAL` bucket semantics explicit as recovery events, not all
  normal readings

## Non-goals

- Changing alert thresholds, NEWS2 scoring, configurable alarm limits, alert
  generation, or event-capture trigger rules.
- Counting total abnormal readings, abnormal dwell time, or alarm burden by
  duration.
- Adding export, filtering, sorting, clipboard support, persistence, or
  multi-patient rollup.
- Replacing the `Session Alarm Events` list or the `Active Alerts` list.
- Changing `patient_print_summary()` unless implementation later proves console
  parity is required by a human owner.

## Intended Use Impact, User Population, Operating Environment, And Foreseeable Misuse

Intended use impact:

- The feature adds a review-only visual summary of already-stored session event
  transitions.
- It does not change live monitoring, active-alert behavior, or clinical
  decision logic.

User population:

- Trained clinicians, reviewers, and internal testers using the local Win32
  workstation application.

Operating environment:

- The existing single-patient desktop workflow with static in-process storage,
  using manually entered or simulated readings.

Foreseeable misuse:

- Reading the strip as a count of all abnormal readings rather than alert-state
  transitions.
- Reading `NORMAL` as proof that the patient was stable for most of the
  session.
- Assuming counts include events from before a visible session reset.
- Looking at the strip instead of the current `Active Alerts` surface when the
  patient's latest state is still abnormal.

## Current Behavior

- `patient_add_reading()` appends a bounded `AlertEvent` only when the session
  moves into a first abnormal state, changes abnormal signature/severity, or
  recovers to normal.
- The first normal reading of a session creates no event.
- A `NORMAL` `AlertEvent` currently means recovery to normal, as shown by the
  stored summary text `Recovered to normal; active abnormalities cleared`.
- `update_dashboard()` renders the reset disclosure, then the stored event rows,
  in `IDC_LIST_EVENTS`.
- There is no aggregated count surface above the list.

## Proposed Change

1. Keep the stored `AlertEvent` list as the single source of truth. The summary
   strip must derive only from the current session's existing event rows.
2. Add a small deterministic aggregation helper in `patient.c` / `patient.h`
   that counts stored events by `AlertLevel`:
   - `ALERT_NORMAL`
   - `ALERT_WARNING`
   - `ALERT_CRITICAL`
3. Define that helper as counting resulting event severities only. It must not
   infer total abnormal readings, duration, or any severity not present in the
   stored event log.
4. In `src/gui_main.c`, insert a compact read-only strip between the
   `Session Alarm Events` label and the existing `IDC_LIST_EVENTS` listbox.
5. Preferred copy:
   - caption: `Session event counts`
   - three buckets: `NORMAL (recoveries)`, `WARNING`, `CRITICAL`
6. If the final control layout cannot fit three separate compact buckets
   cleanly at minimum window size, fall back to one single-line read-only row
   with equivalent text, for example:

```text
Session event counts: NORMAL (recoveries) 1 | WARNING 3 | CRITICAL 2
```

7. Preserve the existing reset disclosure and event list directly below the
   strip. The strip must not hide or replace the session-reset notice.
8. Keep the active-alert banner and `IDC_LIST_ALERTS` visually primary for
   current live risk. The summary strip may reuse severity colors lightly, but
   must not look more urgent than the live alert surfaces.
9. Localize any new user-facing strings through the existing localization layer
   rather than hard-coding new English UI text in `gui_main.c`.
10. Limit the implementation scope to presentation and deterministic aggregation.
    If implementation appears to require changes to event-capture rules, split
    that into a separate issue instead of widening this one.
11. A modest dashboard layout adjustment is acceptable:
    - add approximately one short row of vertical space for the strip
    - shift the event/history list controls downward or slightly reduce their
      heights
    - if clarity suffers at the current minimum height, raise `WIN_CH`
      modestly rather than cramming the review surfaces

## Files Expected To Change

Expected implementation files:

- `src/gui_main.c`
- `src/localization.c`
- `include/localization.h`

Expected support files if the aggregation helper is added:

- `src/patient.c`
- `include/patient.h`

Expected verification files:

- `tests/unit/test_patient.cpp` for any new aggregation helper

Conditional documentation updates if the team wants the new visible GUI behavior
captured in the formal baseline immediately:

- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`

Files expected not to change:

- `src/vitals.c`
- `src/alerts.c`
- `src/news2.c`
- `src/alarm_limits.c`
- session event-capture trigger semantics in `patient_add_reading()`

## Requirements And Traceability Impact

- The feature is adjacent to existing session-review requirements rather than a
  new clinical algorithm:
  - `SYS-021` Session Alarm Event Review Presentation
  - `SWR-GUI-013` Session Alarm Event Review List
  - `SWR-PAT-008` reset/disclosure visibility semantics
- `SYS-020` and `SWR-PAT-007` should remain unchanged because this issue does
  not alter how session alert events are generated.
- For MVP, no new clinical-behavior requirement is required if the project is
  comfortable treating the strip as a narrow presentation refinement of the
  existing review surface.
- If the repo policy requires every new visible behavior to be explicitly
  traceable, extend `SYS-021` and `SWR-GUI-013` to state that the dashboard
  also shows counts of current-session event rows by resulting severity and
  that `NORMAL` represents recoveries-to-normal.
- Any traceability update must avoid over-claiming. The strip does not measure
  total abnormal readings, duration, alarm burden, or longitudinal history
  across resets.

## Medical-Safety, Security, And Privacy Impact

Medical safety:

- No change to thresholds, NEWS2, configurable alarm limits, alert generation,
  or treatment guidance.
- Primary risk is interpretive: a compact summary can be misread as stronger
  evidence than the detailed event rows.
- Required controls:
  - derive counts only from stored `AlertEvent` rows
  - define `NORMAL` explicitly as recovery events
  - keep active alerts visually primary
  - preserve session-reset disclosure

Security:

- No new authentication, authorization, network, or persistence path is added.
- The strip remains inside the current authenticated session boundary.

Privacy:

- No new data class or export path is introduced.
- The feature reuses already-displayed patient session data on the local
  workstation only.

## AI/ML Impact Assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: not applicable
- Input data: existing deterministic `AlertEvent` records only
- Output: deterministic severity counts
- Human-in-the-loop limits: unchanged
- Transparency needs: the UI copy must explain the recovery semantics of the
  `NORMAL` bucket, but this is not an AI explainability concern
- Dataset and bias considerations: not applicable
- Monitoring expectations: normal software verification only
- PCCP impact: none

## Validation Plan

Automated validation:

- If a patient-level aggregation helper is added, unit tests should cover:
  - no events
  - warning-only sessions
  - warning to critical escalation
  - recovery to normal increments the `ALERT_NORMAL` bucket only when a
    recovery event exists
  - reset clears counts with the current session

GUI/manual validation:

- run the existing patient-monitoring workflow and confirm the strip equals the
  visible event rows grouped by resulting severity
- execute a warning -> critical -> normal sequence and confirm counts display
  `WARNING 1`, `CRITICAL 1`, `NORMAL (recoveries) 1`
- confirm the first normal reading of a new session does not increment the
  `NORMAL` bucket
- confirm a visible session-reset notice does not leave stale counts from the
  previous session
- confirm current active alerts remain distinct from the retrospective strip

Recommended commands:

```powershell
run_tests.bat
ctest --test-dir build --output-on-failure -R "Patient"
```

Documentation/traceability validation if formalized:

- ensure any requirement text describes transition-event counts only
- update `requirements/TRACEABILITY.md` only if the implementation also updates
  the formal requirement baseline

## Rollback Or Failure Handling

- If the implementation cannot make the `NORMAL` bucket semantics explicit,
  ship neither a bare `NORMAL` badge nor a misleading summary; instead block
  implementation until the copy is clarified.
- If the summary strip disagrees with the rendered event rows, treat that as a
  release blocker and revert to the pre-strip review surface.
- If the change starts to require new event-generation semantics, split that
  work into a follow-on issue rather than broadening this display-only feature.
- Rollback is straightforward because the intended runtime change is additive
  and local to the session review surface: remove the strip and any helper/API
  additions, leaving the existing event list intact.
