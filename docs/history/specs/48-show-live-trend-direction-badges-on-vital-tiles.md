# Design Spec: Show live trend direction badges on vital tiles

Issue: #48
Branch: `feature/48-show-live-trend-direction-badges-on-vital-tiles`
Spec path: `docs/history/specs/48-show-live-trend-direction-badges-on-vital-tiles.md`

## Problem

The live dashboard already exposes the last bounded session history in two
forms:

- `src/gui_main.c` paints per-tile sparklines for heart rate, systolic blood
  pressure, temperature, SpO2, and respiration rate.
- `src/trend.c` already computes `TREND_RISING`, `TREND_FALLING`, and
  `TREND_STABLE` from those same bounded value series.

Operators still have to read the whole sparkline to infer direction. That adds
friction in exactly the at-a-glance workflow the tiles are meant to support.

The design constraint is narrow and safety-driven:

- the cue must stay supplementary to the current numeric value, tile severity
  color, alerts list, and status banner
- the cue must not appear when live data is unavailable or when the sample
  history is too short to support a meaningful directional label
- the cue must not change NEWS2, alerting, thresholds, persistence, export, or
  any other clinical or system behavior

The risk note at
`docs/history/risk/48-show-live-trend-direction-badges-on-vital-tiles.md`
already approves Architect work within that display-only MVP boundary.

## Goal

Add a compact trend-direction badge to the five live vital tiles only, using
the existing trend extraction and `trend_direction()` helper, while keeping the
feature presentation-only and fully traceable.

## Non-goals

- No change to vital-sign thresholds, alert generation, aggregate alert level,
  NEWS2 scoring, AVPU handling, or simulator sequencing.
- No badge on the NEWS2 tile in this issue.
- No configurable trend window, no longer-horizon trend view, and no export,
  logging, telemetry, or persistence of badge state.
- No new predictive, diagnostic, or clinically validated claim.
- No rework of the existing sparkline algorithm or its 5 percent hysteresis
  threshold.
- No dashboard layout redesign beyond the small badge addition.

## Intended use impact, user population, operating environment, and foreseeable misuse

Intended use impact is limited to display: the dashboard gains a secondary cue
that summarizes recent direction of already displayed live values.

User population remains the same as the current Win32 dashboard: trained
clinical staff or pilot operators reviewing a session-scoped bedside, ward,
handoff, or demo dashboard.

Operating environment remains the existing dashboard path in `src/gui_main.c`,
fed by the current in-memory `PatientRecord` history and either simulation mode
or a future HAL-backed live feed using the same `VitalSigns` structure.

Foreseeable misuse that the implementation must resist:

- treating the badge as equivalent to severity or alarm priority
- reading `STABLE` as clinically normal or safe
- reading a missing badge as stable instead of unavailable or insufficient data
- assuming the NEWS2 tile carries the same trend semantics
- over-trusting a cue derived from too little history

## Current behavior

Current GUI and trend behavior is already well bounded:

- `paint_tile()` in `src/gui_main.c` renders a tile label, current value,
  sparkline strip, and a bottom severity badge (`NORMAL`, `WARNING`,
  `CRITICAL`).
- `paint_tiles()` extracts sparkline data through `trend_extract_hr()`,
  `trend_extract_sbp()`, `trend_extract_temp()`, `trend_extract_spo2()`, and
  `trend_extract_rr()`.
- The sparkline renders only when at least two data points are available.
- `trend_direction()` returns a direction for integer series with `count >= 2`,
  but the GUI does not currently consume that output.
- The blood-pressure sparkline is systolic-only today because the GUI already
  uses `trend_extract_sbp()`.
- Respiration rate is displayed as unavailable when
  `VitalSigns.respiration_rate == 0`, but the extracted RR sparkline data path
  still carries those zero entries because `trend_extract_rr()` writes them.
- The NEWS2 tile is rendered in the sixth slot and currently receives no
  sparkline input.

## Proposed change

Implement the feature as a presentation-layer enhancement with explicit
suppression rules and no new clinical semantics.

### 1. Add an explicit badge state model

Add a small badge state with four outcomes:

- `HIDDEN`
- `RISING`
- `FALLING`
- `STABLE`

The preferred implementation is a tiny pure helper that maps current dashboard
context to one of those four states so the logic is unit-testable outside the
Win32 paint path. The helper should live in presentation-support code, not in a
new clinical or predictive module.

Recommended inputs:

- simulation/live availability
- current-value availability for the tile
- extracted sample count for the tile
- `TrendDir` returned by `trend_direction()`

The paint path in `src/gui_main.c` should remain responsible only for choosing
text, color, and placement.

### 2. Reuse the existing extraction path and trend helper

Each badge must be derived from the same value series already used for the tile
sparkline so the two cues cannot disagree because of different source data.

Per tile:

- Heart Rate: use `trend_extract_hr()`
- Blood Pressure: use `trend_extract_sbp()` and document that the badge reflects
  systolic direction, matching the existing sparkline
- Temperature: use `trend_extract_temp()`
- SpO2: use `trend_extract_spo2()`
- Respiration Rate: use `trend_extract_rr()`

Do not add a second trend heuristic, smoothing pass, or alternate data window
in this issue.

### 3. Require a stricter minimum sample count for badges than for sparklines

Keep the existing sparkline threshold unchanged: the line may continue to render
with two data points.

Require at least three extracted samples before any trend badge is shown. This
is an intentional safety and usability distinction:

- two points are enough to sketch a line
- two points are too little evidence for a stronger directional label

The badge should therefore be hidden when `sample_count < 3` even if
`trend_direction()` could technically return `RISING` or `FALLING`.

### 4. Hide the badge whenever the current tile is not truly live

The badge must be `HIDDEN` for all of the following:

- simulation/device mode is off and the tile shows `N/A`
- no patient session is active
- the tile has no current value yet after a cleared or newly admitted session
- respiration rate is currently not measured (`respiration_rate == 0`)
- the tile is the NEWS2 tile

This rule is more important than any historical direction result. Current
unavailability must suppress the badge, not produce a stale or misleading
direction.

### 5. Keep the badge visually subordinate

The badge must not compete with the tile's primary safety cues. The preferred
layout is:

- keep the existing bottom severity badge unchanged
- keep the existing numeric value unchanged
- add the trend badge as a smaller, right-aligned cue in the label row or
  another similarly secondary position that does not crowd the value or badge
  rows

Visual rules:

- do not use a second red/amber/green color mapping that could be confused with
  alert severity
- encode direction primarily by text and/or a simple non-proprietary token
- if color is used at all, it should follow the tile's existing foreground
  contrast rather than introduce new clinical meaning
- if the badge cannot fit without overlapping the label or value on smaller
  widths, hide the badge before degrading the primary data display

### 6. Keep NEWS2 explicitly out of scope

Do not compute, render, or imply a trend badge for NEWS2 in this issue.

The NEWS2 tile should remain exactly as it is now:

- no sparkline input
- no trend badge
- no change to score calculation or risk-band rendering

### 7. Localize any new user-visible text

If the implementation uses text labels such as `UP`, `DOWN`, or `STABLE`, those
labels should be routed through the existing localization layer instead of being
hardcoded into `gui_main.c`.

If the team instead chooses a language-neutral token for this MVP, document the
mapping in the requirement text and keep the token set ASCII-safe.

## Files expected to change

Expected implementation files:

- `src/gui_main.c`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`

Likely implementation files if a testable badge-state helper is introduced:

- `src/trend_badge.c`
- `include/trend_badge.h`
- `CMakeLists.txt`
- `tests/CMakeLists.txt`
- `tests/unit/test_trend.cpp` or a new focused unit test file for badge-state
  mapping

Possible implementation files if text labels are localized:

- `src/localization.c`
- `include/localization.h`
- `tests/unit/test_localization.cpp`

Files that should not change in this issue:

- `src/vitals.c`
- `src/alerts.c`
- `src/news2.c`
- `src/patient.c`
- `src/sim_vitals.c`
- persistence/config files and release workflows

## Requirements and traceability impact

This issue changes GUI behavior but should not change any clinical or system
calculation behavior.

Recommended requirements approach:

- keep `SYS-014` as the system-level parent because this is a dashboard display
  enhancement
- add a focused GUI requirement, preferably `SWR-GUI-013`, for trend badge
  visibility, state meanings, suppression rules, and explicit NEWS2 exclusion
- keep `SWR-TRD-001` as the underlying deterministic trend-extraction and
  direction-computation dependency unless implementation discovers that the
  existing wording must be revised for consistency

Recommended `SWR-GUI-013` scope:

- the five live vital tiles may display a compact trend badge derived from the
  existing trend helper
- badge states are `RISING`, `FALLING`, `STABLE`, or hidden
- the badge is hidden for insufficient history, unavailable current values,
  device mode, and the NEWS2 tile
- the blood-pressure badge reflects systolic direction, matching the existing
  sparkline
- the badge is supplementary and does not alter severity color or banner logic

RTM impact is expected in:

- forward traceability for the new or revised GUI requirement
- backward traceability to the new automated and manual verification evidence
- GUI verification inventory in repo-level documentation if a new SWR ID is
  added

## Medical-safety, security, and privacy impact

Medical-safety impact is low to medium because the feature changes presentation
of clinically relevant data but not the underlying classification or alerting
logic.

Main risk to control:

- an operator could overweight the badge relative to the numeric value or
  current severity state

Primary controls in this design:

- explicit hidden state for unavailable or insufficient data
- stricter badge threshold than sparkline threshold
- no NEWS2 badge
- no second severity color map
- reuse of the existing deterministic trend helper rather than a new heuristic

Security impact is none expected. No new authentication path, storage location,
network interface, or access-control behavior is introduced.

Privacy impact is none expected. No new patient-data field, export path, or
telemetry path is introduced.

No additional human-owner clinical acceptance criteria are required so long as
implementation stays within this display-only scope. Human approval is required
before implementation if the design expands into configurable windows, NEWS2
trend semantics, or alarm/escalation meaning.

## AI/ML impact assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: none
- Input data: existing deterministic vital-sign history only
- Output: a presentation-layer badge derived from rule-based trend direction
- Human in the loop limits: unchanged from the existing dashboard
- Transparency needs: the badge must remain clearly supplementary and
  non-predictive
- Dataset and bias considerations: none beyond the existing deterministic
  session history already displayed in the sparkline
- Monitoring expectations: standard GUI verification only
- PCCP impact: none

## Validation plan

Implementation should add both automated and manual evidence.

Automated verification:

- unit-test the badge-state helper for:
  - hidden when `sample_count < 3`
  - hidden in device mode
  - hidden when no current value is available
  - hidden for respiration rate when the latest reading is `0`
  - correct mapping of `TREND_RISING`, `TREND_FALLING`, and `TREND_STABLE`
- retain and rerun the existing trend helper tests so the badge feature does not
  silently change `trend_direction()` behavior

Suggested commands:

```powershell
cmake -S . -B build -G Ninja -DBUILD_TESTS=ON
cmake --build build --target test_unit patient_monitor_gui
build/tests/test_unit.exe --gtest_filter=Trend*:TrendBadge*:LocalizationTest.*
```

Manual GUI verification:

- verify no badge in device mode when tiles show `N/A`
- verify no badge before three readings exist
- verify rising, falling, and stable badges on known scripted histories
- verify the badge and sparkline agree for the same tile history
- verify the respiration-rate badge is hidden when the latest reading is not
  measured
- verify the NEWS2 tile never shows a badge
- verify warning and critical tiles remain readable and the badge does not
  outrank the severity label or numeric value

Regression expectations:

- no changes to alert thresholds, alert generation, NEWS2 scoring, persistence,
  localization selection, or HAL interfaces

## Rollback or failure handling

If implementation cannot satisfy the hidden-state and visual-subordination rules
without broadening scope, stop and return the issue to design review rather than
shipping a weaker cue.

Rollback is straightforward:

- revert the implementation commit(s)
- restore the dashboard to sparkline-only behavior
- keep the risk note and this design spec as project history

If requirement wording or localization scope becomes contentious during
implementation, keep the issue out of `ready-for-review`, comment with the
specific blocker, and move it back to design clarification instead of inventing
new clinical semantics.
