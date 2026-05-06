# Design Spec: Issue 94

Issue: `#94`  
Branch: `feature/94-show-alarm-response-time-in-the-session-alarm-review-log`  
Spec path: `docs/history/specs/94-show-alarm-response-time-in-the-session-alarm-review-log.md`

## Problem

The session alarm review log currently preserves only event transitions and
their summaries:

- `include/patient.h` defines `AlertEvent` with `reading_index`, `level`,
  `abnormal_mask`, and `summary`, but no timing fields on the current mainline.
- `src/patient.c` appends one event when the alert-state signature changes and
  formats rows as `#<reading> [<severity>] <summary>`.
- `src/gui_main.c` renders those rows as simple review strings.

Issue `#93` already defines the missing chronology seam by adding a
session-elapsed trigger time to each stored event. Even with that approved
design, reviewers still would not know how long a warning or critical interval
persisted unless they manually subtract adjacent trigger times.

The backlog title says "response time", but that wording is unsafe here. This
feature must describe session-local alarm duration or time-active metadata for
the stored event row, not clinician acknowledgement latency or staff
performance.

## Goal

Add a narrow, read-only `Alarm Duration` field to the session alarm review log
that:

- reuses issue `#93`'s session-elapsed trigger time as the only timing source
- shows how long each abnormal session alarm interval remained in effect
- remains deterministic, bounded, and session-scoped
- keeps live active alerts separate from historical review rows

## Non-goals

- Using the user-facing label `Response Time` unless a human owner explicitly
  approves and defines that term.
- Adding clinician acknowledgement, escalation workflow, operator notes, or
  workforce-performance metrics.
- Introducing wall-clock timestamps, audit-grade charting time, export, cloud
  sync, EMR integration, or persistence beyond the current session.
- Changing alert thresholds, NEWS2 logic, alarm limits, session-reset rules, or
  active-alert semantics.
- Redesigning the event log beyond what is needed to present a duration column
  clearly.

## Intended Use Impact, User Population, Operating Environment, And Foreseeable Misuse

Intended use impact:

- The feature improves retrospective review of session alarm transitions by
  showing the duration of abnormal intervals.
- It does not alter diagnosis, treatment guidance, live prioritization, or the
  active-alert workflow.

User population:

- Trained clinicians, testers, and reviewers using the local Windows desktop
  monitor.

Operating environment:

- Single-patient authenticated workstation use, with static in-process session
  storage and either simulator-fed or manually entered readings.

Foreseeable misuse:

- Reading the field as clinician response or acknowledgement time.
- Treating the duration as a legal charting record or enterprise alarm metric.
- Assuming recovery rows represent alarm-active time when they actually mark the
  start of a normal interval.
- Assuming an unresolved final row has a completed duration when the interval is
  still open.

## Current Behavior

- On the current mainline, `AlertEvent` stores no time metadata at all.
- Issue `#93` is already designed and queued for implementation with
  session-elapsed trigger time (`T+...`) as the approved timing model for the
  same event rows.
- `patient_add_reading()` records only state transitions:
  - first abnormal reading
  - severity change
  - abnormal-parameter-set change
  - recovery to normal
- A recovery row is stored as `ALERT_NORMAL` with `abnormal_mask == 0`, which
  means not every stored row represents an active alarm interval.
- The current GUI and `patient_print_summary()` expose review rows, but not
  duration semantics.

## Proposed Change

1. Treat issue `#93` as a prerequisite timing dependency.
   `#94` shall not invent a second timing source. It shall either land after
   `#93` or include the same stored session-elapsed trigger-time model in the
   same implementation sequence.
2. Use the neutral user-facing label `Alarm Duration` for the new field.
   `Time Active` is acceptable if UI space requires it. Do not render
   `Response Time` in product text for this MVP.
3. Define the duration only for abnormal event rows (`level != ALERT_NORMAL`).
   A stored abnormal row represents the start of an abnormal interval at that
   row's trigger time.
4. Derive the closed duration from adjacent event rows rather than storing a
   second independent timestamp:

```text
duration_seconds =
    next_event.trigger_elapsed_seconds - current_event.trigger_elapsed_seconds
```

   This duration closes when the next stored event occurs, whether that next row
   is escalation, a parameter-set change, or recovery to normal.
5. Require non-decreasing session-elapsed trigger times across stored rows.
   Duration math is second-resolution and deterministic. If adjacent stored
   times are equal, `00:00` is valid. Negative durations are invalid and should
   fail verification rather than being hidden in formatting code.
6. Render abnormal rows with no following event as `Open`, not as a fabricated
   completed duration. The implementation shall not derive this final value from
   the current wall clock or a repaint-time calculation.
7. Render `ALERT_NORMAL` recovery rows as `n/a` in the duration field because
   the row marks the start of the recovered state, not an active alarm
   interval.
8. Keep the field derived from aggregate event rows only. Do not imply
   parameter-specific timing precision when one row may represent multiple
   simultaneous abnormal parameters.
9. Extend the event review surface approved in `#93` to include:
   - `Trigger Time`
   - `Alarm Duration`
   - `Reading`
   - `Severity`
   - `Summary`
10. Keep the active-alert list as the primary live state surface. The duration
    column is historical review metadata only and must not replace present-tense
    live alarm messaging.
11. Update `patient_print_summary()` to print the same semantics as the GUI. A
    representative format is:

```text
T+00:14  00:42  #3 [CRITICAL] Abnormal parameters: Heart Rate, SpO2
T+00:56  n/a    #4 [NORMAL] Recovered to normal; all parameters within range
T+01:10  Open   #5 [WARNING] Abnormal parameters: Heart Rate
```

12. Preserve existing reset-disclosure behavior. If a previous session is
    cleared, its rows and any associated duration metadata disappear together;
    only the existing reset notice remains visible ahead of current-session
    rows.
13. If future export or report surfaces reuse this field, they shall reuse the
    same row-duration rules and placeholders from this issue rather than
    recomputing a different interpretation downstream.

## Files Expected To Change

Expected requirements and traceability files:

- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`

Expected implementation files:

- `include/patient.h`
- `src/patient.c`
- `src/gui_main.c`
- `include/localization.h`
- `src/localization.c`

Expected verification files:

- `tests/unit/test_patient.cpp`
- `tests/integration/test_patient_monitoring.cpp`
- `tests/integration/test_alert_escalation.cpp`
- `dvt/DVT_Protocol.md`

Dependency note:

- If issue `#93` has not yet landed in the implementation branch, its approved
  timing-seam files are prerequisite work and should be merged first or brought
  in as part of the same implementation sequence.

Files expected not to change:

- `src/vitals.c`
- `src/alerts.c`
- `src/news2.c`
- `src/alarm_limits.c`
- authentication and authorization behavior beyond existing session-clear paths

## Requirements And Traceability Impact

No new user need is required. This remains within existing `UNS-017` session
alarm event review scope.

Update existing system requirements:

- `SYS-020`
  Clarify that stored session alarm events carry the trigger-time metadata
  needed to derive abnormal-interval duration from adjacent rows.
- `SYS-021`
  Clarify that review surfaces present both trigger time and derived duration,
  and distinguish unavailable or open duration states explicitly.

Update existing software requirements:

- `SWR-PAT-007`
  Require duration semantics based on the stored trigger time of the current row
  and the next stored event row; require duration applicability only for
  abnormal rows.
- `SWR-PAT-008`
  Clarify that duration metadata disappears whenever the corresponding event
  history is cleared at a session boundary.
- `SWR-GUI-013`
  Require an `Alarm Duration` column or equivalent field with explicit `Open`
  and `n/a` rendering rules, while keeping active alerts separate.
- `SWR-PAT-006`
  Update the summary output expectations so the printed session-event section
  mirrors the GUI duration semantics.

Traceability updates should show:

- `UNS-017 -> SYS-020/SYS-021 -> SWR-PAT-006/SWR-PAT-007/SWR-PAT-008/SWR-GUI-013`
- automated coverage for closed-duration math, equal-second intervals,
  unresolved abnormal rows, recovery rows, and reset behavior

This issue should not create traceability for:

- clinician response-time measurement
- acknowledgement workflow
- staffing analytics
- persistent historical audit logging

## Medical-Safety, Security, And Privacy Impact

Medical safety:

- No change to classification thresholds, NEWS2, alarm limits, or active-alert
  generation.
- The main safety control is terminology and interval semantics: the displayed
  value must mean session-local alarm duration only.
- `n/a` on recovery rows prevents the UI from implying that a normal-state row
  is itself an alarm interval.
- `Open` on the final abnormal row prevents the UI from claiming the interval
  ended when it has not yet been closed by a subsequent event.

Security:

- No new permission model, service, export path, or network transport is
  introduced.
- The field remains inside the existing authenticated local session boundary.

Privacy:

- The feature adds patient-linked review metadata to local display surfaces
  only.
- Existing session reset, logout, and patient-change boundaries continue to
  bound retention.

## AI/ML Impact Assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: not applicable
- Input data: deterministic alert-event rows plus session-elapsed trigger time
- Output: deterministic derived duration formatting for review surfaces
- Human-in-the-loop limits: unchanged
- Transparency needs: explicit historical-vs-live and open-vs-closed interval
  distinction only
- Dataset and bias considerations: not applicable
- Monitoring expectations: standard software verification and GUI/DVT review
- PCCP impact: none

## Validation Plan

Automated validation should use explicit session-elapsed values, not wall-clock
delays.

Unit-test scope:

- abnormal row followed by recovery yields the expected closed duration
- abnormal row followed by escalation or changed abnormal set closes at the next
  row's trigger time
- equal-second adjacent events format as `00:00`
- final abnormal row renders `Open`
- `ALERT_NORMAL` recovery row renders `n/a`
- reset or reinitialization clears the prior rows and therefore clears their
  duration metadata
- summary formatting mirrors GUI semantics

Integration-test scope:

- warning -> critical -> normal sequence shows two closed abnormal durations and
  a recovery row with `n/a`
- abnormal-set changes without severity change still close the prior interval
  and open a new one deterministically
- automatic session rollover clears prior event/duration rows while preserving
  the reset disclosure

Manual GUI / DVT scope:

- the user-facing label is `Alarm Duration` or `Time Active`, not `Response Time`
- duration values are visually distinct from trigger time and active alerts
- the final unresolved abnormal row displays `Open`
- recovery rows display `n/a`
- the column layout remains readable at the supported minimum window size

Recommended validation commands:

```powershell
run_tests.bat
ctest --test-dir build --output-on-failure -R "Patient|Alert"
python dvt/run_dvt.py
```

## Rollback Or Failure Handling

- If implementation cannot reuse the `#93` session-elapsed trigger-time model,
  stop and split the work rather than adding a second ambiguous timing scheme.
- If the UI cannot make `Trigger Time` and `Alarm Duration` distinct and
  readable, do not fall back to misleading free-form concatenated text that
  implies more precision than exists.
- If product wants clinician-response semantics, open a separate issue with its
  own risk review instead of broadening this session-local review feature.
- Rollback is straightforward because the behavior is additive and read-only:
  remove the derived duration field and revert to the trigger-time-only event
  review surface.
