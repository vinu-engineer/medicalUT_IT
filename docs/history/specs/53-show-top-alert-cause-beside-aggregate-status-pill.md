# Design Spec: Show top alert cause beside the aggregate status pill

Issue: `#53`
Branch: `feature/53-show-top-alert-cause-beside-aggregate-status-pill`
Spec path: `docs/history/specs/53-show-top-alert-cause-beside-aggregate-status-pill.md`

## Problem

The current dashboard tells the operator that the patient is in a non-normal
aggregate state, but it does not provide a compact explanation of which active
parameter is currently driving that state.

Today the operator must infer cause by scanning one or more of:

- the colour of the vital tiles in `src/gui_main.c`
- the full active-alert list populated by `update_dashboard()`
- the status banner colour, which reflects aggregate severity but does not show
  a concise cause label in live simulation mode

This is a workflow gap, not a clinical-logic gap. The code already computes:

- per-parameter classifications in `src/vitals.c`
- aggregate severity via `overall_alert_level()`
- structured active alerts via `generate_alerts()`

The safety concern is summary accuracy. `generate_alerts()` emits alerts in a
fixed parameter order, not in highest-severity order. A naive "show the first
alert" implementation could surface a warning-level item while a different
parameter is critical. The design must therefore add an explanatory summary
without changing thresholds, priority semantics, or hiding the full alert list.

## Goal

Add a compact, additive dashboard summary that shows:

- the current aggregate status as a visible pill
- a neighboring cause badge that names the highest-severity active parameter and
  its current value when the aggregate status is `WARNING` or `CRITICAL`

The result should reduce scan time for trained operators while preserving the
existing alert list, tiles, and underlying classification behavior.

## Non-goals

- No change to alert thresholds, NEWS2 scoring, alarm limits, suppression, or
  audible behavior.
- No change to `overall_alert_level()` semantics.
- No reordering, hiding, collapsing, or replacement of the existing active
  alert list in `IDC_LIST_ALERTS`.
- No new diagnosis language, treatment advice, or root-cause claims.
- No change to authentication, persistence, CI, release artifacts, or hardware
  interfaces.
- No attempt to infer a clinically smarter priority order than the currently
  approved severity model.

## Intended use impact, user population, operating environment, and foreseeable misuse

Intended use remains unchanged: a local Windows bedside monitoring dashboard for
reviewing current patient state and active alerts.

User population remains trained clinical staff such as bedside nurses and ward
clinicians who already interpret the existing colour-coded tiles and alert list.

Operating environment remains the current Win32 GUI described in
`docs/ARCHITECTURE.md`. No networked, mobile, or remote workflow is added.

Foreseeable misuse to guard against:

- treating the displayed cause badge as the only active alert
- interpreting the displayed cause as a diagnosis or root cause rather than a
  summary of the highest-severity currently displayed abnormal parameter
- assuming the tie-break order between equally severe alerts implies a new
  clinical priority policy
- expecting the badge to appear in `NORMAL`, no-patient, or device-mode states

## Current behavior

Current relevant behavior is split across the GUI and alert modules:

- `paint_status_banner()` in `src/gui_main.c` uses aggregate severity only to
  choose banner colour during simulation mode. It does not render a dedicated
  aggregate-status pill or a top-cause badge.
- `paint_patient_bar()` renders patient identity and session context, but no
  concise aggregate-status summary.
- `update_dashboard()` fills `IDC_LIST_ALERTS` with the full active-alert list
  and leaves it visible as the detailed source of alert context.
- `generate_alerts()` in `src/alerts.c` emits one alert per abnormal parameter
  in a fixed order. That order is appropriate for list generation but is not a
  severity-ranking API.
- `overall_alert_level()` already provides the correct aggregate status for the
  latest reading and already handles `respiration_rate == 0` as "not measured".

Current consequence: the operator can see that the patient is `WARNING` or
`CRITICAL`, but not the top displayed cause without a second scan step.

## Proposed change

Implement the feature as an additive status-summary cluster in the
patient/status strip above the tiles, while leaving the existing rolling status
banner and the full alert list intact.

### 1. Add a dedicated aggregate-summary cluster to the patient/status strip

Update `paint_patient_bar()` in `src/gui_main.c` so the strip is split into two
zones:

- a left zone for the existing patient or device-mode summary text
- a right-aligned status cluster containing:
  - an aggregate-status pill using the existing `NORMAL` / `WARNING` /
    `CRITICAL` colour mapping
  - a neighboring cause badge shown only when an active alert is present

This placement avoids conflict with the existing rolling simulation banner in
`paint_status_banner()` and keeps the new summary in a stable, always-visible
status area above the vital tiles.

The left text should be width-constrained and ellipsized so it cannot overlap
the new pills.

### 2. Select the displayed cause using explicit severity-first logic

Introduce a small deterministic helper in the alert path, preferably in
`src/alerts.c` with any needed declaration in `include/alerts.h`, that returns
structured summary data for the highest-severity active parameter.

Design constraints for the helper:

- It must derive the answer from existing alert/classification outputs only.
- It must not duplicate or alter threshold logic.
- It must not treat the first alert returned by `generate_alerts()` as the top
  cause without first comparing severities across all active alerts.
- It should return machine-readable structured data, such as parameter identity
  plus severity, so the GUI can format a compact badge without parsing
  human-readable alert prose.

Recommended selection rule:

1. Generate or inspect all active abnormal parameters for the latest reading.
2. Choose the highest `AlertLevel` present.
3. If multiple parameters share the same highest severity, choose the first in
   the existing dashboard display order for stability:
   `Heart Rate`, `Blood Pressure`, `Temperature`, `SpO2`, `Resp Rate`.

The spec must explicitly state that equal-severity tie-breaking is a UI-stability
rule only. It does not create a new clinical priority policy.

### 3. Render concise factual cause content

When a top cause exists, the neighboring badge should display only short factual
content derived from the latest reading, for example:

- `HR 155 bpm`
- `BP 182/121 mmHg`
- `Temp 39.8 C`
- `SpO2 88%`
- `RR 26 br/min`

Do not display:

- normal-range prose
- diagnosis language
- treatment guidance
- certainty claims such as "root cause"

The badge should be hidden when:

- there is no admitted patient
- simulation is disabled and no live reading is present
- the aggregate status is `NORMAL`
- all active alerts have cleared

### 4. Preserve existing alert-detail views and repaint behavior

Do not change the current role of:

- the active-alert list in `IDC_LIST_ALERTS`
- the per-parameter tiles in `paint_tiles()`
- the aggregate banner colour behavior in `paint_status_banner()`

The new status cluster is additive. `update_dashboard()` and the paint path must
refresh the cause badge on the same cycle as the tiles and alert list so the
display cannot become stale after a new reading, scenario step, clear-session,
or simulation toggle event.

### 5. Keep the implementation narrow and local

The preferred implementation surface is:

- `src/gui_main.c` for rendering and layout
- `src/alerts.c` and `include/alerts.h` for deterministic top-cause selection
- `tests/unit/test_alerts.cpp` for helper verification

No change is expected in:

- `src/vitals.c`
- `src/news2.c`
- `src/alarm_limits.c`
- HAL files
- authentication or persistence modules

## Files expected to change

Expected implementation files:

- `src/gui_main.c`
- `src/alerts.c`
- `include/alerts.h`
- `tests/unit/test_alerts.cpp`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`

Possible documentation-only follow-up if the implementer needs to record the
manual verification detail more explicitly:

- `dvt/DVT_Protocol.md`

Files that should not change for this issue:

- `src/vitals.c`
- `src/news2.c`
- `src/alarm_limits.c`
- `include/hw_vitals.h`
- `.github/workflows/**`
- release packaging or installer assets

## Requirements and traceability impact

Existing user and system requirements are sufficient for scope:

- `SYS-006` Aggregate Alert Level
- `SYS-011` Patient Status Summary Display
- `SYS-014` Graphical Vital Signs Dashboard

No new UNS or SYS entry is expected for this issue if the feature remains
presentation-only.

At the software-requirement level, the GUI requirement should be updated so the
new behavior is explicitly traceable and reviewable. Preferred approach:

- extend `SWR-GUI-003` to state that the dashboard status area shall display an
  aggregate-status pill and, when active alerts exist, a compact top-cause
  badge derived from the highest-severity current parameter while preserving the
  full active-alert list

Traceability updates should then flow to `requirements/TRACEABILITY.md` using
the same existing SYS parents. No change should be made to `SWR-ALT-001`
ordering semantics; the list-generation order remains valid and should not be
reframed as a clinical priority ranking.

Supporting deterministic helper tests in `tests/unit/test_alerts.cpp` should be
added even if the accepted requirement-level verification remains GUI review.

## Medical-safety, security, and privacy impact

Medical-safety impact is low to moderate and entirely tied to summary accuracy.
The feature is safe only if it remains additive and deterministic.

Primary safety risks:

- showing a warning-level cause while a different parameter is critical
- showing stale cause data after a repaint-triggering event
- encouraging users to ignore simultaneous active alerts

Required controls:

- severity-first top-cause selection
- deterministic tie-breaking
- unchanged aggregate alert classification
- unchanged full alert list visibility
- concise factual wording only

Security impact is none expected. No authentication, authorization, or external
interfaces change.

Privacy impact is none expected. The badge re-presents patient values already
shown on the local dashboard and introduces no new storage or transmission path.

## AI/ML impact assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

The feature reuses existing deterministic rule-based alert outputs only. There
is no model purpose, no model input/output contract, no human-in-the-loop
change, no dataset or bias consideration, no model monitoring requirement, and
no Predetermined Change Control Plan impact.

## Validation plan

Implementation validation should include both deterministic logic checks and GUI
review.

Automated checks:

- build the repository with the standard commands from `CLAUDE.md`
- run at minimum:
  - `build/tests/test_unit.exe`
  - `build/tests/test_integration.exe`
- add focused unit coverage for the new top-cause helper covering:
  - all-normal vitals returns no cause
  - one abnormal parameter returns that parameter
  - mixed warning and critical parameters always choose the critical parameter
  - equal-severity ties choose the documented stable order
  - `respiration_rate == 0` never becomes the displayed cause

Manual GUI review:

- `NORMAL` state: aggregate pill shown, cause badge hidden
- `WARNING` state with one active alert: cause badge matches the abnormal tile
- `CRITICAL` state with multiple active alerts: cause badge matches the
  highest-severity active alert and the full list still shows all alerts
- tie case with two equally severe active alerts: displayed cause is stable
  across repeated repaints and timer ticks
- no-patient, clear-session, simulation paused, and device-mode states
- long patient names and long left-strip text do not overlap the pill cluster
- supported UI languages still render without strip overlap or clipping severe
  enough to obscure status meaning

Regression review:

- confirm that alert list content and order are unchanged
- confirm that `overall_alert_level()` behavior is unchanged
- confirm that NEWS2 score display behavior is unchanged

## Rollback or failure handling

If implementation cannot add the new summary cluster without colliding with
existing patient-strip content or violating current status-banner behavior,
remove the partial UI change and return to the pre-feature state rather than
repurposing the rolling banner ad hoc.

If deterministic highest-severity selection cannot be implemented without
changing approved alert semantics, stop and move the issue back for design
review instead of shipping a first-alert approximation.

If reviewer feedback concludes that equal-severity tie-breaking needs new human
factors guidance or a different clinical priority model, split that work into a
new issue. This issue should remain limited to presentation of the existing
severity model.
