# Design Spec: Issue 86

Issue: `#86`  
Branch: `feature/86-show-session-elapsed-time-in-the-patient-bar`  
Spec path: `docs/history/specs/86-show-session-elapsed-time-in-the-patient-bar.md`

## Problem

The current patient bar does not show how long the active local monitoring
session has been open.

- `src/gui_main.c` `paint_patient_bar()` currently renders patient name, ID,
  age, BMI, and reading count when simulation mode is active.
- There is no explicit session clock in `AppState`, `PatientRecord`, or any
  other runtime state.
- Operators must infer session freshness from reading count, simulator cadence,
  or memory, which is unsafe because those proxies are not equivalent to true
  elapsed local session time.

This matters because the dashboard already uses the patient bar as the main
context surface, but the app also has multiple session-reset boundaries:
admit/refresh, clear, logout, simulation disable, and automatic rollover when
`MAX_READINGS` is reached. Any elapsed-time display that is not tied to one
explicit session lifecycle will become ambiguous or stale.

## Goal

Add a narrow, read-only `Session` elapsed-time indicator to the patient bar so
an operator can tell how long the current local monitoring session has been
open at a glance.

The feature must:

- mean local session age only, not hospital admission duration
- use an explicit time source rather than reading count or simulator cadence
- reset or suppress cleanly on every session boundary
- remain presentation-layer only and avoid changes to clinical algorithms

## Non-goals

- Showing true hospital admission age, length of stay, chart time, or any
  persisted timeline outside the local app session.
- Changing vital-sign thresholds, NEWS2 logic, alarm limits, alert generation,
  session alarm event semantics, or treatment guidance.
- Adding persistence, export, reporting, networking, audit, or multi-patient
  timeline behavior.
- Introducing a trend widget, countdown, reminder, or any alert-like visual
  treatment for elapsed time.
- Reworking the existing patient/session model in `patient.c` unless a later
  issue explicitly broadens scope beyond the GUI-only patient bar.

## Intended Use Impact, User Population, Operating Environment, And Foreseeable Misuse

Intended use impact:

- The feature adds a read-only orientation aid in the dashboard header area.
- It does not alter the primary live-monitoring, alerting, or review workflow.

User population:

- bedside clinical staff, internal testers, and reviewers using the local
  Win32 workstation application

Operating environment:

- the existing single-patient desktop workflow with in-process state, local
  simulation/manual entry, and no authoritative hospital admission feed

Foreseeable misuse:

- treating the timer as true hospital admission duration
- assuming the timer proves uninterrupted monitoring across clear, logout, or
  rollover boundaries
- interpreting the timer as time since last clinician review, chart update, or
  physiologic change
- using the timer for charting, audit, or legal-record purposes

## Current Behavior

- `paint_patient_bar()` has three states today:
  - device mode message when `g_app.sim_enabled == 0`
  - patient context text when `g_app.sim_enabled == 1` and `g_app.has_patient`
  - "Awaiting first simulation reading..." when simulation is enabled and no
    patient is active
- `AppState` stores `has_patient`, `sim_paused`, and `sim_enabled`, but no
  session-start tick, no active-session timer flag, and no dedicated repaint
  timer for elapsed-time presentation.
- `do_admit()` calls `patient_init()` and sets `g_app.has_patient = 1`.
- `do_clear()` zeroes `g_app.patient`, clears `g_app.has_patient`, and repaints.
- `apply_sim_mode()` disables the dashboard timer and clears patient state when
  simulation is turned off; when simulation is turned on with no patient, it
  auto-creates a default session and starts data acquisition.
- Logout clears patient/session state and returns to the login screen.
- The simulation timer reinitializes the patient record when
  `patient_is_full()` becomes true and records an automatic reset disclosure via
  `patient_note_session_reset()`.
- `Pause Sim` stops reading acquisition, but it does not end the local session.

## Proposed Change

1. Define the new field explicitly as local session age and label it
   `Session`, not `Admission`.
2. Keep the elapsed-time state in the Win32 presentation layer (`AppState`),
   not in `PatientRecord` or any persistence file.
3. Add presentation-only session-clock state sufficient to answer two
   questions:
   - when the current local session started
   - whether an elapsed-time value is currently valid to render
4. Use one explicit monotonic time source for elapsed-time calculation, such as
   a Win32 monotonic tick API. Do not derive elapsed time from:
   - `reading_count`
   - simulator timer cadence
   - wall-clock timestamps entered or inferred from patient data
5. Define the session-start boundary as successful local session creation, not
   first successful reading. The timer should arm when a new session is created
   through:
   - `do_admit()`
   - dashboard startup when simulation mode auto-creates the default patient
   - `apply_sim_mode()` when simulation is enabled and a default patient
     session is created
   - automatic rollover when the dashboard reinitializes the patient after the
     previous bounded session fills
6. Define the session-end or timer-suppression boundaries as:
   - `do_clear()`
   - logout
   - simulation disable / device mode
   - any no-patient state where the app no longer intends the current session
     to be visible
7. Continue elapsed time across `Pause Sim`. Pause stops data acquisition, not
   the local session clock. If the timer were to freeze, the field would no
   longer mean session age.
8. Decouple elapsed-time repaint from reading acquisition. The current
   simulation timer only updates while acquisition is active and not paused, so
   the feature needs a lightweight UI refresh path that can repaint the patient
   bar on a regular cadence without generating readings or mutating patient
   data.
9. Render the value as compact text in the existing patient bar, for example:

   ```text
   Patient: James Mitchell | ID: 2001 | Age: 45 yrs | BMI: 25.5 (Overweight) | Session: 00:12:34 | Readings: 4 / 10
   ```

10. Use `HH:MM:SS` formatting with zero-padded minutes and seconds. Hours may
    expand beyond two digits if needed; the value must not wrap at 24 hours.
11. When no active session exists, suppress the elapsed value entirely rather
    than showing stale data. The existing device-mode and waiting-state copy
    should remain the visible message in those states.
12. Keep presentation neutral:
    - use normal patient-bar text styling
    - do not color the timer like an alert
    - do not present it as a badge, countdown, or recommendation
13. Treat automatic rollover as the start of a new bounded local session. The
    elapsed timer resets to zero for that new session. The existing
    `patient_session_reset_notice()` disclosure remains the rollover
    explanation surface; this issue should not add a second rollover-history
    mechanism.
14. Do not infer a new valid timer solely from `reading_count > 0`. A valid
    elapsed-time display must come from an explicit session-start boundary.

## Files Expected To Change

Expected requirements and traceability files:

- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`

Expected implementation files:

- `src/gui_main.c`

Expected verification files:

- `dvt/DVT_Protocol.md`
- any manual GUI verification checklist or evidence asset the repo uses for
  dashboard-only requirements

Files expected not to change for this issue:

- `include/patient.h`
- `src/patient.c`
- `src/vitals.c`
- `src/alerts.c`
- `src/news2.c`
- `src/alarm_limits.c`
- `src/sim_vitals.c`
- `src/app_config.c`

If implementation chooses to extract a pure helper for time formatting or timer
state transitions to improve automated coverage, that should remain a small
supporting refactor and not broaden the feature into a domain-level session
model change.

## Requirements And Traceability Impact

This issue does not need a new user need if the team treats it as a refinement
of existing dashboard and status-context needs.

Existing trace anchors most likely to apply:

- `UNS-010` consolidated status summary
- `UNS-014` graphical dashboard
- `SYS-011` patient status summary display
- `SYS-014` graphical vital signs dashboard
- `SWR-GUI-002` session management boundaries
- `SWR-GUI-003` dashboard presentation behavior
- `SWR-GUI-004` admit/refresh workflow
- `SWR-GUI-010` simulation/device-mode patient-bar behavior

Recommended traceability shape:

- add one new SYS-level requirement for session-context elapsed-time display in
  the dashboard patient bar
- add one or two new GUI-level SWRs covering:
  - display semantics and formatting
  - reset/suppression behavior across session boundaries and pause/device mode

Recommended verification mapping:

- manual GUI review / DVT for patient-bar rendering and reset behavior
- optional deterministic unit tests only if implementation introduces a pure
  helper outside Win32 message handling

Traceability must state clearly that:

- the timer is local-session context only
- no clinical algorithm or alert semantics change
- the value is sourced from explicit GUI session lifecycle, not sample count

## Medical-Safety, Security, And Privacy Impact

Medical safety:

- No change to thresholds, NEWS2 scoring, alarm generation, or treatment
  guidance is intended.
- Primary benefit is better session context at a glance.
- Primary risk is wrong-context interpretation if the timer is stale,
  ambiguous, or presented as broader than a local session clock.
- Residual safety is acceptable only if every reset boundary is handled
  consistently and the field stays labeled as `Session`.

Security:

- No new authentication, authorization, or network behavior is introduced.
- The field remains inside the current authenticated local-session boundary.

Privacy:

- No new PHI category or persistence path is introduced.
- The timer exposes only local session-duration metadata already coupled to the
  visible patient context.

## AI/ML Impact Assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: not applicable
- Input data: deterministic local session state only
- Output: formatted elapsed-session text only
- Human-in-the-loop limits: unchanged
- Transparency needs: session-scoped wording only; no AI explainability issue
- Dataset and bias considerations: not applicable
- Monitoring expectations: standard software verification and GUI evidence only
- PCCP impact: none

## Validation Plan

Recommended implementation validation:

```powershell
build.bat
run_tests.bat
```

Required GUI smoke checks:

- admit or auto-start a patient session and confirm the timer starts from the
  documented boundary
- let the session run and confirm the displayed value increases monotonically
- pause simulation and confirm elapsed time continues to advance while no new
  readings are acquired
- clear session and confirm the elapsed value disappears immediately and does
  not survive into the cleared state
- logout and confirm no prior session value is visible after returning to the
  login screen
- disable simulation and confirm device mode shows no stale session age
- re-enable simulation and confirm a new session starts from zero
- trigger automatic rollover at `MAX_READINGS` and confirm the timer restarts
  for the new bounded session while the existing reset disclosure explains that
  earlier session review data was cleared

Expected verification outcome:

- runtime changes remain limited to dashboard presentation and session-clock
  state
- no clinical calculations or persisted patient data paths change
- no stale elapsed value survives any documented reset boundary

## Rollback Or Failure Handling

- If implementation cannot keep the field explicitly session-scoped, do not
  fall back to an ambiguous `Admission` timer or a reading-count proxy.
- If keeping the timer accurate across pause and reset boundaries requires a
  broader rework of patient/session semantics, stop and split that rework into
  a follow-on issue.
- If layout constraints make the patient bar unreadable at the supported minimum
  width, prefer a shorter text rendering over adding a second row or expanding
  the feature into a dashboard redesign.
- Rollback is straightforward because the feature is additive and
  presentation-local: remove the session-clock state and patient-bar rendering,
  and restore the current patient bar behavior.
