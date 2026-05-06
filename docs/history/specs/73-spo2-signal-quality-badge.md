# Design Spec: add a SpO2 signal-quality badge

Issue: `#73`
Branch: `feature/73-spo2-signal-quality-badge`
Spec path: `docs/history/specs/73-spo2-signal-quality-badge.md`
Risk note: `docs/history/risk/73-spo2-signal-quality-badge.md`

## Problem

The current dashboard shows the SpO2 tile as a numeric value plus the existing
physiologic alert-severity color and status badge. That means a displayed
number such as `92%` or `98%` looks equally authoritative whether the source
has an explicit local quality state or no quality information at all.

Issue `#73` asks for a compact, display-only cue that helps operators
distinguish between:

- a displayed SpO2 value with a known quality state
- a displayed SpO2 value whose quality is unknown

The current code has no place to store or render that distinction:

- `src/gui_main.c` renders the SpO2 tile from `VitalSigns.spo2` and
  `check_spo2(v->spo2)` only
- `include/vitals.h` does not carry any SpO2 quality metadata with the reading
- `src/sim_vitals.c` produces only raw vital values, not signal-quality state

If implementation tried to add the badge from ad hoc GUI-only state, it would
risk drifting out of sync with the numeric reading pulled from
`patient_latest_reading()`. The design therefore needs a narrow data contract
that keeps the quality state attached to the same reading as the displayed
SpO2 value.

## Goal

Add a small read-only signal-quality badge to the existing SpO2 tile so the UI
can show `Quality: Good`, `Quality: Poor`, or `Quality: Unknown` for the
displayed value without changing any clinical classification behavior.

The intended outcome is:

- the SpO2 tile continues to show the existing numeric value, trend sparkline,
  and physiologic severity state
- a second, visually subordinate cue shows the local SpO2 quality state
- the quality state travels with each `VitalSigns` reading so the badge matches
  the displayed value
- missing, unsupported, stale, manual, or otherwise untrusted quality input
  fails safe to `Unknown`
- signal quality does not change thresholds, alert generation, NEWS2 scoring,
  aggregate status, alarm limits, or simulation/device mode semantics

## Non-goals

- No waveform analysis, artifact detection, or inferred quality scoring.
- No change to `check_spo2()`, `overall_alert_level()`, `generate_alerts()`,
  `news2_calculate()`, or alarm-limit behavior.
- No new diagnostic or treatment claim.
- No new persistence, export, audit logging, telemetry, or network API.
- No new manual-entry control in the MVP; manual readings may default to
  `Unknown`.
- No badge on non-SpO2 tiles.
- No reuse of `NORMAL/WARNING/CRITICAL` wording or dominant alert colors for
  the quality cue.

## Intended use impact, user population, operating environment, and foreseeable misuse

- Intended use impact: adds a secondary confidence cue beside the displayed
  SpO2 value on the existing dashboard.
- User population: trained clinical staff using the Windows desktop monitor.
- Operating environment: current simulated monitoring workflow and future
  device-backed HAL workflows on the same dashboard.
- Foreseeable misuse: reading `Good` quality as proof of clinical stability,
  reading `Poor` quality as permission to ignore a low SpO2 value, or assuming
  absent quality metadata should imply a positive state.

This remains a display-only UX aid. It must not be framed as a diagnostic
assessment or a substitute for clinician judgment.

## Current behavior

Today the SpO2 path behaves as follows:

- `paint_tiles()` in `src/gui_main.c` formats the SpO2 number from `v->spo2`
  and computes only the physiologic severity via `check_spo2(v->spo2)`.
- The generic `paint_tile()` renderer shows one bottom badge containing
  `NORMAL`, `WARNING`, or `CRITICAL`.
- `VitalSigns` currently carries heart rate, blood pressure, temperature,
  SpO2, and respiration rate only.
- `SIM_SEQUENCE` in `src/sim_vitals.c` contains no signal-quality metadata.
- When simulation is disabled, the tile shows `N/A`; when no reading exists,
  the tile shows `--`.

As a result, there is no explicit way to distinguish "numeric value with known
quality state" from "numeric value with unknown quality state," and any future
quality cue would be forced either to infer quality from the numeric value
alone or to keep detached UI state outside the stored reading.

## Proposed change

Implement the feature as a bounded dashboard and data-contract enhancement with
these design decisions.

### 1. Add explicit SpO2 quality metadata to the reading

Extend `include/vitals.h` with a small enum that represents only the MVP
states:

```c
typedef enum {
    SPO2_QUALITY_UNKNOWN = 0,
    SPO2_QUALITY_GOOD,
    SPO2_QUALITY_POOR
} SpO2SignalQuality;
```

Append a new `spo2_signal_quality` field to `VitalSigns` after
`respiration_rate`.

Why append it there:

- `SWR-VIT-008` currently states that `respiration_rate` is the sixth member of
  `VitalSigns`
- adding the new field after `respiration_rate` preserves that existing
  requirement while still letting the quality state travel with each reading

Producer rule:

- every producer of `VitalSigns` must set `spo2_signal_quality` explicitly
- if the source does not support quality metadata, the field must be set to
  `SPO2_QUALITY_UNKNOWN`

This keeps the badge state paired with the exact SpO2 reading stored in patient
history and prevents stale or mismatched GUI-only quality state.

### 2. Keep the quality mapping deterministic and separate from clinical severity

Add a tiny string helper for the enum in `src/vitals.c` and declare it in
`include/vitals.h`, for example:

```c
const char *spo2_signal_quality_str(SpO2SignalQuality quality);
```

Expected behavior:

- `GOOD` -> `"Good"`
- `POOR` -> `"Poor"`
- `UNKNOWN` or any invalid value -> `"Unknown"`

This helper is metadata-only. It must not change or influence:

- `check_spo2()`
- `overall_alert_level()`
- `generate_alerts()`
- `news2_calculate()`
- alarm-limit evaluation

The badge vocabulary stays orthogonal to physiologic severity. A low SpO2 can
still have `Good` signal quality, and a normal SpO2 can still have `Poor` or
`Unknown` quality.

### 3. Source the MVP quality state conservatively

For the MVP:

- `src/sim_vitals.c` should populate an explicit quality state for every entry
  in `SIM_SEQUENCE`
- the simulated quality states must not be derived mechanically from the SpO2
  percentage thresholds alone
- at least one scenario should demonstrate a low SpO2 value with `Good`
  quality and at least one non-critical SpO2 value with `Poor` or `Unknown`
  quality so the UI proves the two concepts are independent
- manual `Add Reading` flow in `src/gui_main.c` should set
  `SPO2_QUALITY_UNKNOWN`
- future HAL implementations should set the field from device metadata when
  supported, otherwise `UNKNOWN`

This keeps the MVP deterministic and testable without inventing waveform
analysis or a hidden scoring algorithm.

### 4. Render a secondary badge only on the SpO2 tile

Update `src/gui_main.c` so only the SpO2 tile gets an additional compact badge.
The simplest acceptable implementation is either:

- extend `paint_tile()` with optional secondary-badge inputs, or
- add a small SpO2-specific wrapper around the existing tile rendering

Required UI behavior:

- show `Quality: Good`, `Quality: Poor`, or `Quality: Unknown` when a numeric
  SpO2 value is present
- keep the existing bottom severity badge (`NORMAL/WARNING/CRITICAL`) unchanged
- keep the new quality badge visually subordinate to the tile background and
  severity cue
- do not use the same dominant red/amber/green semantics as physiologic alert
  severity
- hide the quality badge entirely when the tile value is `N/A` or `--`

Preferred layout:

- place the quality badge near the SpO2 label or upper-right region of the
  tile so it reads as metadata, not as a replacement for the existing severity
  badge

This preserves the existing dashboard structure and avoids a broad visual
redesign.

### 5. Preserve all existing clinical and operational behavior

The feature must remain presentation-only:

- no change to SpO2 thresholds or alert escalation
- no change to NEWS2 inputs or score calculation
- no change to aggregate banner logic
- no change to trend extraction
- no change to patient-record capacity or ordering
- no change to device mode other than suppressing the badge when there is no
  reading to annotate

The implementation must not fall back to inferring quality from the SpO2
number itself. If no trustworthy quality source exists, show `Unknown` or hide
the badge when no reading exists.

## Files expected to change

Expected implementation files:

- `docs/history/specs/73-spo2-signal-quality-badge.md`
- `docs/history/risk/73-spo2-signal-quality-badge.md` (already present; inspect
  and update only if implementation reveals a new risk)
- `include/vitals.h`
- `src/vitals.c`
- `include/hw_vitals.h` (documentation comment update for the expanded reading
  contract)
- `src/gui_main.c`
- `src/sim_vitals.c`
- `src/main.c` (initializer updates only, if needed)
- `requirements/UNS.md`
- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`
- `README.md` (if the user-facing dashboard feature list is kept current)

Expected test files to change:

- `tests/unit/test_vitals.cpp`
- `tests/unit/test_patient.cpp`
- `tests/unit/test_hal.cpp`
- `tests/unit/test_news2.cpp` (initializer updates only, unless extra guards are added)
- `tests/unit/test_trend.cpp` (initializer updates only)
- `tests/unit/test_alerts.cpp` (initializer updates only)
- `tests/integration/test_patient_monitoring.cpp`
- `tests/integration/test_alert_escalation.cpp`

Files that should not change in behavior:

- `src/alerts.c`
- `src/news2.c`
- `src/alarm_limits.c`
- `src/trend.c`
- authentication, persistence, localization, CI, and release workflows

## Requirements and traceability impact

This issue should not be implemented as an undocumented UI tweak. The quality
cue changes how a live SpO2 value is interpreted and needs explicit
traceability.

Recommended requirement updates:

- add a new user need, provisionally `UNS-017`, stating that operators need a
  visible signal-quality cue for the displayed SpO2 value
- add a new system requirement, provisionally `SYS-020`, requiring the system
  to present a read-only SpO2 signal-quality state that defaults fail-safe to
  `Unknown` when unavailable and does not affect clinical severity logic
- add a new software requirement, provisionally `SWR-GUI-013`, defining:
  - the approved quality vocabulary (`Good`, `Poor`, `Unknown`)
  - the rule that the badge appears only on the SpO2 tile when a value is
    present
  - the fallback to `Unknown` for unsupported/manual/missing quality metadata
  - the requirement that the quality cue not alter alert colors, alert text,
    NEWS2, or threshold classification

Adjacent requirements that should be updated, not replaced:

- `SYS-015` and `SWR-GUI-005` to document that HAL producers must populate
  `VitalSigns.spo2_signal_quality` or set it to `UNKNOWN`
- `SWR-VIT-008` should remain satisfied by keeping `respiration_rate` as the
  sixth member of `VitalSigns`
- `SWR-GUI-003` may need wording refresh so the new cue is explicitly
  secondary to the existing colour-coded severity tiles

Traceability updates expected:

- new RTM row(s) for the new requirement(s)
- updated automated test references for enum/string mapping and reading-copy
  preservation
- test-count summary updates if new test cases are added

## Medical-safety, security, and privacy impact

Medical-safety impact is bounded but real because the feature changes how a
live oxygenation value may be interpreted.

Primary safety concerns:

- false reassurance if a stale or unsupported value is shown with a positive
  quality cue
- false dismissal if a clinically important low SpO2 value is paired with a
  negative quality cue and the user over-discounts the number
- confusion if the quality cue looks too similar to the existing physiologic
  severity colors

Required safety controls:

- explicit `Unknown` default
- no quality inference from the SpO2 numeric threshold
- quality text and styling distinct from `NORMAL/WARNING/CRITICAL`
- no badge when there is no current reading to annotate
- no downstream use of quality for alarms, NEWS2, or patient status

Security impact is low. No new privilege boundary, credential path, network
surface, or file format is required.

Privacy impact is none expected. The MVP adds only in-memory quality metadata
to a live vital-sign reading and does not add export or persistence.

## AI/ML impact assessment

This issue does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: none
- Input data: none beyond existing deterministic vital-sign readings
- Output: none
- Human-in-the-loop limits: unchanged
- Transparency needs: unchanged
- Dataset and bias considerations: not applicable
- Monitoring expectations: unchanged
- PCCP impact: none

## Validation plan

Implementation validation should cover both the data contract and the GUI
behavior.

Automated validation:

```bat
run_tests.bat
ctest --test-dir build --output-on-failure
```

Required automated coverage additions:

- `tests/unit/test_vitals.cpp`
  - string mapping for `GOOD`, `POOR`, `UNKNOWN`
  - invalid enum value returns `Unknown`
- `tests/unit/test_patient.cpp`
  - `patient_add_reading()` preserves `spo2_signal_quality` when copying a
    `VitalSigns` sample
- `tests/unit/test_hal.cpp`
  - simulation feed populates only approved quality states
  - `hw_init()` reset preserves the same paired reading and quality sequence
- existing unit and integration tests that use `VitalSigns` aggregate
  initializers must be updated so the new field is always explicit

Manual GUI verification:

- the SpO2 tile renders the quality badge and no other tile does
- the badge shows `Good`, `Poor`, and `Unknown` in deterministic scenarios
- the existing SpO2 severity color and bottom severity badge remain unchanged
- a low SpO2 value can still show `Good` quality in the designated demo case
- a non-critical SpO2 value can show `Poor` or `Unknown` quality in the
  designated demo case
- simulation-disabled (`N/A`) and no-reading (`--`) states suppress the badge
- manual `Add Reading` produces `Quality: Unknown`
- NEWS2 score, aggregate banner, and alert list are unchanged by the quality
  state

Recommended focused commands during implementation:

```powershell
rg -n "VitalSigns v = \\{|SIM_SEQUENCE|spo2_signal_quality|check_spo2\\(" src include tests
git diff --stat
```

## Rollback or failure handling

If implementation discovers that the `VitalSigns` contract change causes more
scope churn than expected, do not fall back to a detached global GUI variable
for the quality badge. That would reintroduce the stale-pairing risk this
design is intended to avoid.

Preferred fallback order:

1. keep the issue on the explicit reading-boundary design and finish the
   initializer/test updates
2. if the scope becomes too large for one implementation issue, split out the
   `VitalSigns`/HAL contract update as a precursor issue
3. do not ship a heuristic badge derived from the SpO2 numeric value alone

If no trustworthy quality source exists for a given reading path, render
`Unknown` or suppress the badge when there is no reading, rather than
presenting a positive quality state.
