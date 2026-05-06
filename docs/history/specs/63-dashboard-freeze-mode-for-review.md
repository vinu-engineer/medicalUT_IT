# Design Spec: Issue #63

Date: 2026-05-06
Issue: `#63` - Feature: add a dashboard freeze mode for review
Branch: `feature/63-dashboard-freeze-mode-for-review`
Risk note: `docs/history/risk/63-dashboard-freeze-mode-for-review.md`

## Problem

The current dashboard is always live while simulation mode is enabled. Every
timer tick appends a new reading to `g_app.patient`, repopulates the alert and
history list boxes in `update_dashboard()`, and repaints the patient bar, tiles,
and status banner from the newest patient state in `paint_patient_bar()`,
`paint_tiles()`, and `paint_status_banner()`.

That behavior is correct for monitoring, but it makes short review tasks harder
than they need to be. Operators cannot hold a stable screen long enough for
handoff discussion, screenshot capture, or brief review without using the
existing `sim_paused` state, which stops acquisition rather than preserving a
read-only snapshot.

## Goal

Add a bounded dashboard-local review mode that freezes the currently displayed
dashboard snapshot for a short period while live simulation acquisition and
derived status calculations continue in the background.

The implementation goal is:

- let an operator capture a stable review snapshot without changing patient data
- keep the frozen state unmistakably non-live and easy to exit
- avoid any change to acquisition, thresholds, alerts, NEWS2, persistence, or
  hardware abstraction semantics

## Non-goals

- no change to clinical classification thresholds or alert-generation logic
- no change to NEWS2 scoring or risk-band calculation
- no persistence of freeze state across settings changes, logout, or restart
- no export, sharing, screenshot automation, or audit-log feature
- no retrospective browsing, waveform review, or alarm-history tooling
- no support for future real-device monitoring paths in this first design

## Intended use impact, user population, operating environment, and foreseeable misuse

Intended use impact:
The feature adds a short-lived review aid to the existing Windows dashboard. It
does not add a new diagnostic, alarm-management, or monitoring claim.

User population:
Authenticated clinical and administrative users already permitted to access the
dashboard.

Operating environment:
The current Win32 GUI in simulation-backed dashboard mode on a Windows
workstation.

Foreseeable misuse:

- a frozen dashboard is mistaken for current patient state
- freeze is confused with the existing `SIM PAUSED` state
- an operator leaves freeze active while the patient status worsens
- an operator assumes freeze is available for future live-device monitoring
- manual-entry or demo actions are invoked while a frozen view is visible
- a screenshot of stale patient data is reused outside its review context

## Current behavior

- `dash_proc()` starts a `TIMER_SIM` timer when simulation mode is enabled.
- Each `WM_TIMER` tick reads from `hw_get_next_reading()`, appends to
  `g_app.patient`, advances the rolling status message offset, and calls
  `update_dashboard()`.
- `update_dashboard()` always resets and repopulates the history and alert list
  boxes from the live `PatientRecord`.
- `paint_patient_bar()`, `paint_tiles()`, and `paint_status_banner()` always
  render the newest live patient state.
- `IDC_BTN_PAUSE` toggles `g_app.sim_paused`, which stops simulation updates. It
  is not suitable as a review snapshot because it changes acquisition semantics.

## Proposed change

### MVP scope

Implement a separate review-only freeze state that is available only when all
of the following are true:

- `g_app.sim_enabled == 1`
- `g_app.sim_paused == 0`
- `g_app.has_patient == 1`
- at least one reading exists in `g_app.patient`

This keeps the first implementation within the currently verified simulation
path. If the product owner later wants freeze during HAL-backed live monitoring,
that should be handled as a follow-on issue with explicit safety approval.

### UI and state model

- Add a new dashboard header control labeled `Freeze Review`.
- When active, the control label changes to `Return Live`.
- Entering freeze captures a static snapshot of the currently visible dashboard:
  patient-bar content, six tiles, aggregate/status banner meaning, active alert
  list, and reading-history list.
- While freeze is active, the dashboard renders only that snapshot plus a
  dedicated frozen-state indicator.
- The frozen-state indicator must be visually distinct from both the normal
  alert banner colors and the existing `SIM PAUSED` status. It must include:
  - the word `FROZEN`
  - a clear statement that live monitoring continues in the background
  - snapshot age in seconds
- The frozen-state indicator should occupy a fixed, high-contrast banner region
  so it cannot be missed during screenshot capture or handoff review.

### Data and control flow

- Keep live patient data in `g_app.patient` as the single source of truth.
- Add a static freeze snapshot structure in `src/gui_main.c` that can hold:
  - a copied `PatientRecord`
  - copied alert-list text lines
  - copied history-list text lines
  - snapshot start tick/time
  - frozen baseline aggregate alert level
  - frozen baseline NEWS2 risk band
- On `Freeze Review`, copy the current visible dashboard state into that static
  snapshot and mark freeze active.
- While freeze is active, `WM_TIMER` must continue to:
  - acquire the next simulated reading
  - append it to the live `g_app.patient`
  - evaluate live alert severity and NEWS2 against the live record
- While freeze is active, `update_dashboard()` must not repopulate visible list
  boxes from the live record. The visible dashboard stays frozen until exit.
- Parent-window repainting may continue so the frozen-state age can update, but
  the repaint path must render from the snapshot, not from live patient data.
- On exit, clear the freeze state and run one normal `update_dashboard()` so the
  operator immediately sees the current live dashboard.

### Guardrails and automatic exit conditions

Freeze is read-only. While freeze is active:

- disable `Admit / Refresh`
- disable `Add Reading`
- disable `Clear Session`
- disable demo-scenario buttons
- disable `Pause Sim`
- disable `Settings`

Only `Return Live` and `Logout` remain available from the dashboard header.

Freeze must automatically exit when any of the following occurs:

- the operator selects `Return Live`
- 15 seconds elapse from freeze activation
- the user logs out
- simulation mode is turned off
- the dashboard session is cleared or re-admitted
- a live reading increases the aggregate alert severity relative to the frozen
  baseline
- a live reading increases the NEWS2 risk band relative to the frozen baseline

This gives implementers a concrete and safety-biased definition: freeze may hold
steady values for brief review, but it must not continue silently once the live
patient risk becomes more severe than the frozen snapshot.

### Behavior that must not change

- `hw_get_next_reading()` cadence and use
- `patient_add_reading()` behavior and storage semantics
- alert-generation rules in `alerts.c`
- aggregate alert computation in `vitals.c`
- NEWS2 computation in `news2.c`
- configuration persistence in `monitor.cfg`
- any role-based access rule

## Files expected to change

- `src/gui_main.c`
  - add freeze state, snapshot capture, new button handling, and frozen render
    path
- `include/localization.h`
  - add string IDs for the new control and frozen-state banner text
- `src/localization.c`
  - add translations for new freeze-related strings
- `requirements/UNS.md`
  - add a new review-snapshot user need
- `requirements/SYS.md`
  - revise dashboard-refresh wording and add a dedicated freeze-mode system
    requirement
- `requirements/SWR.md`
  - add GUI requirements for freeze capture, frozen-state indication, and exit
    guardrails
- `requirements/TRACEABILITY.md`
  - add forward and backward traceability rows for the new requirement IDs
- `dvt/DVT_Protocol.md`
  - add a GUI verification procedure for frozen-state visibility and exit rules

No domain-library source file should change for this feature.

## Requirements and traceability impact

Recommended requirement changes:

- Add `UNS-017 - Review Snapshot for Handoff and Screen Capture`
  - Need: an authenticated operator shall be able to hold a stable dashboard
    snapshot briefly for review while the system makes the stale state explicit.
- Revise `SYS-014` so automatic refresh remains the default dashboard behavior
  but explicitly excludes the new operator-invoked review snapshot state.
- Add `SYS-020 - Dashboard Freeze Review Mode`
  - The system shall provide an operator-invoked, time-bounded frozen dashboard
    snapshot that does not stop underlying acquisition, patient-record updates,
    aggregate alert calculation, or NEWS2 calculation.
- Add `SWR-GUI-013 - Freeze Review Snapshot Control`
  - Define control availability, snapshot capture, rendered frozen view, and the
    local non-persistent state boundary.
- Add `SWR-GUI-014 - Frozen-State Guardrails`
  - Define distinct frozen-state indication, 15-second timeout, disabled
    mutating controls, and automatic exit on increased live severity or NEWS2
    risk.

Traceability consequences:

- New rows are required in `requirements/TRACEABILITY.md` for `UNS-017`,
  `SYS-020`, `SWR-GUI-013`, and `SWR-GUI-014`.
- Existing rows for `UNS-014`, `UNS-015`, and `SYS-014` should be updated to
  reflect the bounded exception created by freeze mode.
- No traceability change is needed for `SWR-VIT-*`, `SWR-ALT-*`, or
  `SWR-NEW-001` because the feature depends on their existing live behavior
  rather than changing it.

## Medical-safety, security, and privacy impact

Medical safety:

- Primary hazard: stale-display misuse.
- Risk reduction in this design comes from short timeout, explicit `FROZEN`
  labeling, snapshot-age display, disabled mutating controls, and automatic exit
  when live risk worsens.
- The feature must be described as review-only and not as a primary monitoring
  mode.

Security:

- No new authentication, authorization, network, or storage path is added.
- The feature remains local to the existing dashboard session.

Privacy:

- The frozen dashboard may expose patient identifiers during screenshots.
- This design deliberately does not add export or sharing features.
- Operational handling of screenshots remains a site-process concern, not a new
  software capability.

## AI/ML impact assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: not applicable
- Input data: not applicable
- Output: not applicable
- Human-in-the-loop limits: not applicable
- Transparency needs: not applicable beyond normal GUI state indication
- Dataset and bias considerations: none
- Monitoring expectations: none beyond standard GUI verification
- PCCP impact: none

## Validation plan

- Manual GUI walkthrough:
  - enter freeze from a live simulated dashboard
  - verify the screen content stays stable while the frozen banner age advances
  - verify `Return Live` restores the current live dashboard
- Guardrail walkthrough:
  - verify timeout exits freeze after 15 seconds
  - verify `Pause Sim`, settings, demo buttons, and manual-entry actions are not
    available while frozen
  - verify logout clears freeze state
- Safety regression walkthrough:
  - start from a lower-severity frozen snapshot and confirm a higher live
    aggregate severity forces exit
  - start from a lower NEWS2 risk band and confirm a higher live NEWS2 band
    forces exit
- Requirements verification:
  - update `requirements/UNS.md`, `requirements/SYS.md`, `requirements/SWR.md`,
    and `requirements/TRACEABILITY.md`
  - add a DVT procedure that records the frozen-state indicator, timeout, and
    severity-escalation exit behavior

## Rollback or failure handling

- If snapshot capture fails or the freeze state becomes internally inconsistent,
  the implementation shall clear freeze state and fall back to the normal live
  dashboard on the next repaint.
- Because freeze state is not persisted, logout and application restart already
  restore the pre-feature live behavior.
- If validation cannot prove that acquisition and live severity evaluation
  continue while the dashboard is frozen, the feature should not ship and should
  be reverted by removing the new GUI control and requirement rows.
