# Design Spec: Make simulation mode unmistakably non-clinical

Issue: #43
Branch: `feature/43-simulation-mode-not-for-patient-monitoring`
Spec path: `docs/history/specs/43-simulation-mode-not-for-patient-monitoring.md`

## Problem

The current simulation presentation is too subtle for a safety-sensitive demo
path. When `sim_enabled = 1`, the dashboard still resembles an operational
monitor because:

- the header only shows `* SIM LIVE` or `SIM PAUSED`
- the patient bar does not repeat a non-clinical warning when synthetic data is
  active
- the status banner uses a decorative rolling message rather than an explicit
  prohibition against clinical use
- the approved requirement set still encodes that weaker behavior, and
  `SWR-GUI-010` is currently traced inconsistently between
  `requirements/SWR.md` and `requirements/TRACEABILITY.md`

This creates a human-factors risk that a screenshot, handoff, or quick glance
could be mistaken for live patient-monitoring evidence.

## Goal

Define a narrow, presentation-only implementation plan that makes synthetic
monitoring unmistakably non-clinical anywhere the current dashboard surfaces
simulation state, while preserving existing monitoring workflow, alert logic,
NEWS2 behavior, timer behavior, and HAL boundaries.

## Non-goals

- No change to vital-sign generation, scenario sequences, alert thresholds,
  NEWS2 scoring, alarm-limit behavior, or patient-record semantics.
- No change to authentication, RBAC, password handling, configuration format,
  networking, persistence, or hardware integration.
- No new export, archive, or training-orchestration feature in this issue.
- No claim that clearer simulation labeling makes the product suitable for live
  patient monitoring.
- No copying of competitor layouts, branding, iconography, or proprietary demo
  UX.
- No broad CLI-output hardening in this issue; the scoped change remains the
  current Win32 dashboard simulation surfaces controlled by `sim_enabled`.

## Intended use impact, user population, operating environment, and foreseeable misuse

Intended use impact:

- The change hardens the distinction between synthetic and live-looking monitor
  states during demos, DVT runs, UI work, and internal review.
- It changes presentation only and does not widen clinical claims.

User population:

- developers
- testers
- demo operators
- reviewers

Operating environment:

- local Win32 desktop application
- simulation-backed sessions using the existing HAL simulation feed
- screenshots or review discussions derived from that running dashboard

Foreseeable misuse:

- simulation mode is mistaken for live monitoring because the monitor still
  looks clinically operational
- a paused simulation is mistaken for a frozen real-patient display
- a screenshot containing synthetic values is reused without obvious
  non-clinical context
- one simulation surface is hardened while another remains ambiguous

## Current behavior

- `src/gui_main.c` paints a small header indicator that reads `* SIM LIVE` or
  `SIM PAUSED` when simulation is active.
- `src/gui_main.c` uses the patient bar for demographics or waiting-state text,
  but that text does not itself state that the data is not for patient
  monitoring.
- `src/gui_main.c` builds a repeated rolling message from
  `localization_get_string(STR_SIM_MODE_MSG)` and scrolls it across the status
  banner.
- `src/localization.c` currently defines `STR_SIM_MODE_MSG` as
  `"SIMULATION MODE - Synthetic vital data"`, which is descriptive but not an
  explicit non-clinical prohibition.
- `requirements/SWR.md` currently approves `* SIM LIVE` / `SIM PAUSED` header
  wording in `SWR-GUI-010`.
- `requirements/SWR.md` currently approves a decorative rolling message in
  `SWR-GUI-011`.
- `requirements/SWR.md` and `requirements/TRACEABILITY.md` disagree on the SYS
  parent for `SWR-GUI-010`, so implementation and traceability are already out
  of alignment before any new work begins.

## Proposed change

### 1. Add an explicit SYS-level requirement for non-clinical simulation presentation

Add a new system requirement, `SYS-020`, to cover the simulation-warning
behavior directly instead of continuing to rely on mismatched traces through
unrelated SYS entries.

Recommended intent for `SYS-020`:

`When the system is presenting synthetic monitoring data, it shall display an
explicit non-clinical warning that the content is not for patient monitoring on
the primary simulation-facing dashboard surfaces. The warning shall remain
visible while simulation is live or paused and shall remain legible across the
supported alert-state color schemes.`

Recommended trace anchors:

- `UNS-014`
- `UNS-015`

No new UNS entry is required for this issue.

### 2. Revise `SWR-GUI-010` to cover persistent header and patient-context labeling

Keep the existing mode-toggle and persistence behavior in `SWR-GUI-010`, but
replace the current subtle simulation-label wording with explicit non-clinical
behavior:

- the header shall show a persistent simulation warning while `sim_enabled = 1`
- the live/paused state may still be shown, but it shall be secondary to the
  non-clinical warning and must not be the only indicator
- the patient bar shall also carry the simulation-only warning while synthetic
  data is active, including the "awaiting first reading" case
- when `sim_enabled = 0`, the dashboard shall return to device-mode messaging
  without leaving stale simulation wording behind

Recommended requirement trace:

- `SYS-015` for simulation-mode behavior and persistence context
- `SYS-020` for the warning requirement itself

This keeps patient-context labeling within the existing sim-mode requirement
instead of introducing a new SWR for the same dashboard surface.

### 3. Replace the decorative behavior in `SWR-GUI-011` with an explicit banner warning

Revise `SWR-GUI-011` in place. Do not renumber it and do not preserve the
current rolling greeting text as an approved behavior.

The revised requirement should require:

- an explicit warning phrase equivalent to
  `SIMULATION ONLY - NOT FOR PATIENT MONITORING`
- continuous visibility while `sim_enabled = 1`
- visibility in normal, warning, and critical alert backgrounds
- legibility that does not depend on color alone
- correct behavior while simulation is live, paused, resumed, and refreshed

The requirement should no longer mandate scrolling text, decorative separators,
or the current `sim_msg_scroll_offset` behavior. If implementation retains any
motion, the full warning still has to remain immediately understandable and
cannot disappear off-screen.

Recommended requirement trace:

- `SYS-020`

### 4. Drive all new wording through the localization layer

Do not solve this issue by hardcoding fresh English literals in `gui_main.c`.
Implement the warning text through the existing localization layer:

- update `include/localization.h` if separate string IDs are needed
- update `src/localization.c` for all supported languages
- preserve existing language persistence behavior in `monitor.cfg`

The design should prefer one canonical warning phrase reused across surfaces,
with separate strings added only where layout constraints require different
short and long forms.

### 5. Keep the code change local to dashboard presentation

Expected implementation scope is:

- `src/gui_main.c` for header, patient bar, and status-banner rendering
- `include/localization.h` and `src/localization.c` for string definitions
- requirements and traceability documents

Out-of-scope code paths include:

- `sim_vitals.c`
- `alerts.c`
- `patient.c`
- `news2.c`
- `app_config.c` logic beyond any existing localization or sim-mode wiring

If implementation discovers another synthetic-data review surface in the current
Win32 dashboard path that is not already covered by the files above, stop and
bring the issue back to design rather than silently broadening scope.

### 6. Repair traceability while changing the wording

Update the requirements set and RTM together so the approved behavior matches
the implemented behavior:

- add `SYS-020` to `requirements/SYS.md`
- revise `SWR-GUI-010` in `requirements/SWR.md`
- revise `SWR-GUI-011` in `requirements/SWR.md`
- update the forward and backward rows for `SWR-GUI-010` and `SWR-GUI-011` in
  `requirements/TRACEABILITY.md`
- add narrow revision-history entries in the touched requirements documents that
  state the change hardens non-clinical simulation labeling without changing
  monitoring logic

This issue should fix the pre-existing `SWR-GUI-010` trace mismatch instead of
carrying it forward.

## Files expected to change

- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`
- `include/localization.h`
- `src/localization.c`
- `src/gui_main.c`
- `tests/unit/test_localization.cpp` only if the implementation extends
  automated coverage for new string IDs or ordering assumptions

Files that should not change:

- `src/sim_vitals.c`
- `src/alerts.c`
- `src/patient.c`
- `src/news2.c`
- `include/patient.h`
- `.github/workflows/**`

## Requirements and traceability impact

Primary requirement impact:

- add `SYS-020` for explicit non-clinical simulation presentation
- revise `SWR-GUI-010` so the approved behavior covers persistent header and
  patient-bar warning text, not just `* SIM LIVE` / `SIM PAUSED`
- revise `SWR-GUI-011` so the approved behavior is an explicit non-clinical
  banner warning instead of a decorative rolling message

Traceability impact:

- `SWR-GUI-010` should no longer point to `SYS-004` / `SYS-005`
- `SWR-GUI-011` should no longer be justified as a decorative visual flourish
  under `SYS-005`
- `requirements/TRACEABILITY.md` must be updated in both its forward and
  backward sections so the new behavior has one defensible SYS parent chain

No new software module is required. No new Doxygen `@req` family is required.
The change should remain within the existing GUI requirement IDs, with one new
SYS parent added above them.

## Medical-safety, security, and privacy impact

Medical-safety impact:

- This is a human-factors safety improvement aimed at preventing synthetic data
  from being mistaken for real patient monitoring.
- The change is still safety-relevant because misinterpretation of synthetic
  values could delay or distort clinical response.
- The implementation must remain presentation-only. Any drift into alarms,
  NEWS2, thresholds, or hardware behavior is out of scope and should stop the
  work.

Security impact:

- none expected
- no new permissions, auth flows, crypto, or network paths

Privacy impact:

- none expected
- no new identifiers, storage paths, or patient-data sharing behavior

## AI/ML impact assessment

This issue does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: not applicable
- Input data: not applicable
- Output: not applicable
- Human-in-the-loop limits: not applicable
- Transparency needs: not applicable beyond normal UI wording review
- Dataset and bias considerations: not applicable
- Monitoring expectations: not applicable
- PCCP impact: none

## Validation plan

Downstream implementation validation should include both requirements review and
runtime verification.

Targeted regression commands:

```powershell
run_tests.bat
ctest --test-dir build --output-on-failure -R "Config|Localization|Patient"
python dvt/run_dvt.py
rg -n "SYS-020|SWR-GUI-010|SWR-GUI-011|SIMULATION ONLY|NOT FOR PATIENT MONITORING" requirements src include
```

Manual GUI verification:

- enable simulation and confirm the header warning is explicit before the first
  reading arrives
- confirm the patient bar warning remains visible with and without a current
  patient loaded
- confirm the status banner warning remains readable in normal, warning, and
  critical backgrounds
- pause and resume simulation and confirm the non-clinical warning stays visible
- disable simulation and confirm device-mode text returns without stale warning
  fragments
- switch across the supported languages and confirm the warning text fits the
  layout without clipping or truncation

If implementation adds new localization string IDs or changes enum ordering,
update automated localization coverage so the string tables remain aligned.

## Rollback or failure handling

Rollback is straightforward:

- revert the implementation commit if the exact warning phrase, SYS wording, or
  traceability updates are rejected during review
- if the warning cannot be made readable across supported languages and alert
  backgrounds without broader layout changes, stop and return the item to design
  instead of shipping a truncated or ambiguous warning
- if implementation discovers that stakeholders want CLI/demo transcript
  hardening in the same issue, split that work into a follow-up issue rather
  than silently widening this dashboard-scoped change
