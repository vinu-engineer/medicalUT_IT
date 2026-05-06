# Design Spec: Issue 99

Issue: `#99`  
Branch: `feature/99-mark-pause-resume-gaps-reading-history-list`  
Spec path: `docs/history/specs/99-mark-pause-resume-gaps-reading-history-list.md`

## Problem

The dashboard currently shows reading history as a continuous list of stored
vital-sign rows with no explicit continuity disclosure:

- `src/gui_main.c:update_dashboard()` rebuilds `IDC_LIST_HISTORY` from
  `g_app.patient.readings[]` only, one rendered row per stored `VitalSigns`
  entry.
- The Pause control only toggles `g_app.sim_paused`; it does not record any
  durable history metadata about where monitoring stopped and later resumed.
- When simulation resumes, the next reading appears directly after the prior
  pre-pause row, which can make an interrupted monitoring session look like one
  uninterrupted run.

Because the current history rows do not include wall-clock timestamps, the
history list has no other way to disclose that continuity was interrupted.

## Goal

Add a narrow, read-only continuity marker to the dashboard reading-history list
so a reviewer can immediately see that live monitoring was paused and later
resumed.

The design must:

- keep raw reading storage and ordering unchanged
- keep alert generation, NEWS2, thresholds, and acquisition cadence unchanged
- render deterministically across repeated `update_dashboard()` repaints
- clear with the same patient/session boundaries that already clear the current
  reading history

## Non-goals

- Changing `PatientRecord` reading storage semantics or inserting synthetic
  pause rows into `g_app.patient.readings[]`.
- Inferring interruptions from elapsed time, missed timer ticks, or device
  silence heuristics.
- Showing exact paused duration or wall-clock timestamps that the system does
  not currently record.
- Changing active-alert behavior, session alarm event behavior, NEWS2 scoring,
  alarm limits, or any vital-sign classification logic.
- Adding persistence, export, audit logging, or cross-session retention for
  pause/resume metadata.
- Broadening the MVP to real-device disconnects or non-user-initiated
  acquisition failures in this issue.
- Copying vendor-specific dashed-line motifs, colors, or monitor layouts from
  competitor products.

## Intended Use Impact, User Population, Operating Environment, And Foreseeable Misuse

Intended use impact:

- The feature adds a continuity disclosure to the existing session-scoped
  reading-history review surface.
- It does not change the primary live-monitoring, alerting, or manual-entry
  workflows.

User population:

- Trained clinical users, internal testers, and reviewers using the Win32
  desktop dashboard.

Operating environment:

- The current local workstation application, primarily in simulation mode,
  using authenticated in-process session state and static storage.

Foreseeable misuse:

- Treating the marker as proof that the patient remained stable during the
  paused interval.
- Assuming the marker encodes exact paused duration or real clock time.
- Confusing the marker with a measured reading row or an active alert row.
- Expecting the marker to survive clear, logout, simulation disable, or the
  existing bounded session rollover behavior.

## Current Behavior

- `create_dash_controls()` creates three read-only list surfaces:
  `IDC_LIST_ALERTS`, `IDC_LIST_EVENTS`, and `IDC_LIST_HISTORY`.
- `update_dashboard()` clears and repopulates `IDC_LIST_HISTORY` solely from
  `g_app.patient.reading_count` and `g_app.patient.readings[i]`.
- Reading rows are rendered as plain text beginning with `#<reading-index>`.
- `IDC_BTN_PAUSE` toggles `g_app.sim_paused` and changes the button label
  between pause and resume text, but no pause boundary is stored for later
  rendering.
- The timer path (`WM_TIMER`, `TIMER_SIM`) stops appending new readings while
  `g_app.sim_paused` is true.
- Session-scoped state is cleared or reinitialized by:
  - `do_clear()`
  - `do_admit()` / `patient_init()`
  - simulation disable in `apply_sim_mode()`
  - logout in `IDC_BTN_LOGOUT`
  - automatic bounded rollover in the timer path when `patient_is_full()`
- The history list has no continuity marker today, so resumed monitoring can
  appear visually continuous even when observation was interrupted.

## Proposed Change

1. Keep pause-gap tracking in the GUI/application session layer, not in
   `PatientRecord`.
   Rationale: the interruption source for this MVP is the GUI's explicit Pause
   control, which is presentation-layer session metadata rather than patient
   domain data.
2. Add bounded session metadata in `g_app` (or a small GUI-support helper used
   by `g_app`) to track reading-history gap markers independently from raw
   readings.
   The minimal data model is:
   - a pending flag indicating that a pause boundary has been opened
   - a bounded list of marker anchors keyed to the 1-based reading index before
     which a marker must be rendered
3. Open a pending pause boundary only on the explicit user transition from
   running simulation to paused simulation.
   - Ordinary repaints must not create markers.
   - Re-clicking pause/resume without a real state transition must not create
     duplicate markers.
4. Materialize the marker only when a subsequent reading is successfully added
   after that pause.
   This keeps the marker anchored to an actual post-pause reading and avoids
   leaving orphan disclosure rows when the user pauses and then clears, logs
   out, disables simulation, or hits a session rollover before another reading
   arrives.
5. Treat the first successful post-pause reading append, regardless of source,
   as the anchor point for the marker.
   This keeps the behavior deterministic even if a user manually adds a reading
   while simulation is paused or immediately after resuming.
6. Render the marker as a distinct plain-text separator row before the anchored
   post-pause reading in `IDC_LIST_HISTORY`.
   Recommended wording direction:
   - `--- Monitoring resumed after pause ---`
   - or equivalent neutral wording that discloses interrupted continuity
   The row must not begin with `#<n>` and must not look like an alert severity
   banner.
7. Do not show paused duration, clock time, or inferred clinical status inside
   the marker row.
   The system does not currently record reliable timing evidence for that.
8. Support multiple pause/resume cycles within one bounded session by allowing
   multiple marker anchors, each inserted once and rendered in reading order.
9. Clear both pending and stored marker state whenever the current patient
   session is cleared or reinitialized:
   - `do_clear()`
   - `do_admit()`
   - logout
   - simulation disable
   - startup/session initialization paths that seed a fresh patient
   - automatic session rollover after the reading buffer limit is reached
10. Keep existing raw reading numbering unchanged.
    Example: if readings `#1` through `#5` existed before pause and the next
    stored reading is `#6`, the history list should show the marker row and
    then the normal `#6 ...` row; the marker itself is not a numbered reading.
11. Limit this MVP to the existing explicit simulation Pause/Resume workflow.
    Future real-device interruption handling can reuse the same rendering
    concept later, but this issue should not invent generalized outage
    detection without a distinct source-of-truth signal.
12. Prefer extracting the marker-tracking rules into a small non-Win32 helper
    if the implementer wants unit-testable trigger/reset coverage.
    If the logic stays inside `gui_main.c`, the approved verification should
    rely on manual GUI review plus supplemental GUI automation that inspects
    listbox rows.

## Files Expected To Change

Expected requirements and traceability files:

- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`
- `dvt/DVT_Protocol.md`
- `dvt/run_dvt.py`

Expected implementation files:

- `src/gui_main.c`
- `include/localization.h`
- `src/localization.c`

Possible helper/test-support files if the implementer extracts the pause-gap
rules from Win32 message handling:

- `include/history_gap_tracker.h`
- `src/history_gap_tracker.c`
- `tests/unit/test_history_gap_tracker.cpp`

Expected supplemental GUI automation updates:

- `dvt/automation/run_dvt.py`

Files expected not to change:

- `src/patient.c` and `include/patient.h`
- `src/vitals.c`
- `src/alerts.c`
- `src/news2.c`
- `src/alarm_limits.c`
- authentication and account-management files

## Requirements And Traceability Impact

- Existing requirements directly implicated:
  - `UNS-009` Vital Sign History
  - `UNS-014` Graphical User Interface
  - `SYS-014` Graphical Vital Signs Dashboard
  - `SWR-GUI-010` Simulation Mode Toggle
- Existing requirements expected to remain unchanged:
  - `SYS-009` raw reading storage order and capacity
  - `SWR-PAT-002` / `SWR-PAT-003` raw reading append/access semantics
  - `SWR-GUI-013` session alarm event review list semantics
  - all alerting, NEWS2, and alarm-limit requirements
- Recommended requirements update approach:
  - expand `SYS-014` to state that the dashboard discloses monitoring
    continuity interruptions in the reading-history surface when paused live
    monitoring later resumes
  - add a new GUI-level requirement, e.g. `SWR-GUI-014`, to define:
    - explicit pause/resume trigger source
    - one marker per completed pause/resume boundary
    - insertion before the first post-pause reading
    - distinction from reading rows and alert rows
    - reset behavior on clear/logout/sim-off/session rollover
- Traceability should map the new or revised GUI requirement to:
  - `src/gui_main.c`
  - localization string assets if new wording is introduced
  - manual GUI review as the approved verification method
  - optional supplemental GUI automation that confirms marker presence and
    non-duplication
- No new UNS entry is strictly required if the team treats this as a refinement
  of existing history/dashboard needs rather than a net-new intended-use claim.

## Medical-Safety, Security, And Privacy Impact

Medical safety:

- No change is intended to patient-state calculation, thresholds, alarms, or
  treatment guidance.
- The safety benefit is presentation honesty: the review surface better
  reflects that continuity of observation was interrupted.
- Main failure risks are:
  - missing marker after a real pause/resume
  - false marker when no pause gap occurred
  - duplicate marker across repeated repaints
  - wording that implies patient stability or precise timing during the gap

Security:

- No new authentication, authorization, or network surface is introduced.
- Marker state must remain within the current authenticated session boundary and
  clear when the existing session/patient state clears.

Privacy:

- No new data category, persistence path, or external transmission is added.
- The marker is derived only from existing session control state and the
  current patient-linked history surface.

## AI/ML Impact Assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: not applicable
- Input data: existing deterministic session control state plus stored readings
- Output: deterministic continuity marker rows in the GUI history list
- Human-in-the-loop limits: unchanged
- Transparency needs: the marker must be clearly distinguishable from a
  measured reading, but that is a deterministic UI labeling concern
- Dataset and bias considerations: not applicable
- Monitoring expectations: standard GUI verification only
- PCCP impact: none

## Validation Plan

Approved verification should stay aligned with the repo's existing GUI evidence
model.

Required validation scope:

- manual GUI review covering:
  - pause during live simulation, then resume, and confirm one marker appears
    before the first post-pause reading
  - repeated `update_dashboard()` repaints do not duplicate the same marker
  - a session with no pause/resume boundary shows no marker rows
  - clear, logout, simulation disable, patient re-admit, and automatic session
    rollover all clear stored/pending markers
  - multiple pause/resume cycles in one bounded session render in order
  - marker text is distinguishable from reading rows and active alerts
- supplemental GUI automation, if practical, should inspect `IDC_LIST_HISTORY`
  row text after pause/resume and assert that the marker appears exactly once
  at the expected boundary
- if the implementer extracts a pure helper module for gap tracking, add unit
  tests for:
  - state transition to pending on pause
  - one anchor creation on first post-pause reading
  - no duplicate anchors on extra repaints or extra resume clicks
  - reset clearing both pending and stored markers

Recommended validation commands:

```powershell
cmake --build build
ctest --test-dir build --output-on-failure
python dvt/run_dvt.py
python dvt/automation/run_dvt.py
```

Expected evidence updates:

- `dvt/DVT_Protocol.md` should add a manual GUI step or extend an existing
  simulation-control/history review step for the new continuity marker
- `dvt/run_dvt.py` should map the new or revised GUI requirement to the
  approved manual evidence path
- if supplemental automation is added, it should be described as supporting
  evidence rather than the sole approved claim for Win32 GUI semantics

## Rollback Or Failure Handling

- If implementation requires changing patient-domain storage, alert semantics,
  or generalized device-interruption detection, stop and split that work into a
  follow-on issue.
- If the listbox surface cannot make the marker clearly distinguishable from
  real readings using bounded text-only presentation, do not ship an ambiguous
  UI; keep the issue blocked pending a clearer UX decision.
- If validation cannot prove that reset boundaries clear marker state
  deterministically, revert the feature rather than leaving inconsistent session
  history metadata in the dashboard.
- Rollback is straightforward because the change is additive and session-local:
  remove the GUI marker-tracking/rendering logic and restore the pre-feature
  raw-reading history list.
