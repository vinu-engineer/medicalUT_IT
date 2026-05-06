# Design Spec: Issue 83

Issue: `#83`  
Branch: `feature/83-surface-active-alert-source-in-status-banner`  
Spec path: `docs/history/specs/83-surface-active-alert-source-in-status-banner.md`  
Risk note: `docs/history/risk/83-surface-active-alert-source-in-status-banner.md`

## Problem

The current dashboard already exposes the aggregate alert state, but not the
parameter that is driving it:

- `src/gui_main.c` `paint_status_banner()` uses banner background color to show
  the latest aggregate severity, while the banner text path is reserved for
  device-mode messaging when simulation is off and for the rolling simulation
  message when simulation is on.
- `src/gui_main.c` `update_dashboard()` and `src/patient.c`
  `patient_print_summary()` expose detailed active-alert content only outside
  the banner.
- `src/alerts.c` `generate_alerts()` emits abnormal parameters in fixed
  parameter order, not highest-severity order, so the obvious shortcut of
  taking the first alert would sometimes name the wrong source.

As a result, a clinician or tester can see that the patient is in `WARNING` or
`CRITICAL` state from the banner color, but still has to scan the separate
active-alert list to identify the main abnormal parameter.

## Goal

Add one compact, deterministic source indicator to the existing status banner
so the dashboard can show which monitored parameter currently drives the active
warning or critical state, while:

- preserving the current device-mode and rolling simulation banner text paths
- reusing the existing alert-classification rules as the only source of truth
- keeping the banner cue supplementary to the active-alert list, not a
  replacement for it
- keeping the implementation surface narrow and testable

## Non-goals

- Changing thresholds, aggregate severity rules, NEWS2 scoring, alarm limits,
  or treatment guidance.
- Reordering the active-alert list or changing the `generate_alerts()`
  contract.
- Replacing the current device-mode message or rolling simulation message.
- Changing patient-summary content, event-log behavior, persistence, network
  transport, export, clipboard, or audit logging.
- Adding diagnosis, recommendation, acknowledgment, escalation, or workflow
  semantics to the banner.
- Expanding the localization surface for full parameter phrases in this issue.

## Intended Use Impact, User Population, Operating Environment, And Foreseeable Misuse

Intended use impact:

- The feature adds a quick orientation cue on a high-visibility surface for the
  latest reading only.
- It does not change the alert engine, the active-alert list, or the patient
  summary as the authoritative detailed views.

User population:

- Trained clinicians, testers, and reviewers using the local Windows
  workstation application.

Operating environment:

- The existing authenticated single-patient desktop workflow, operating in
  device mode or simulator-fed monitoring mode.

Foreseeable misuse:

- Treating the badge as the only abnormal parameter when several parameters are
  abnormal.
- Assuming the badge is a diagnosis or action recommendation rather than a
  shorthand source cue.
- Assuming the source badge can appear in device mode without live simulated or
  measured patient context.
- Misreading missing RR data as an RR-driven abnormality if the selection logic
  does not preserve the existing `respiration_rate == 0` behavior.

## Current Behavior

- `paint_status_banner()` uses the aggregate `AlertLevel` only for color.
- When simulation is disabled, the banner shows the approved device-mode text
  from `STR_DEVICE_MODE_MSG`.
- When simulation is enabled, the banner shows the approved rolling simulation
  text from `STR_SIM_MODE_MSG`.
- `patient_current_status()` exposes only the aggregate severity, not the
  parameter source.
- `generate_alerts()` emits abnormal parameters in fixed order:
  `Heart Rate`, `Blood Pressure`, `Temperature`, `SpO2`, `Resp Rate`.
- `overall_alert_level()` and `generate_alerts()` both preserve the current
  rule that `respiration_rate == 0` means "not measured" and must not create a
  false RR-driven abnormality.
- The active-alert list already presents the full current abnormal set from the
  latest reading and remains the only detailed alert surface inside the GUI.

## Proposed Change

1. Add one canonical helper in the alert/domain layer, for example in
   `alerts.c/.h`, that selects the single primary abnormal source from a
   `VitalSigns` snapshot without changing the existing `generate_alerts()`
   output contract.
2. The helper shall evaluate each monitored parameter with the existing
   classification rules:
   `check_heart_rate()`, `check_blood_pressure()`, `check_temperature()`,
   `check_spo2()`, and `check_respiration_rate()` when RR is measured.
3. The helper shall return "no source" when:
   - there is no patient
   - there is no latest reading
   - the aggregate state is `ALERT_NORMAL`
   - RR is the only candidate but `respiration_rate == 0`
4. Source selection shall use this deterministic policy:
   - choose the highest-severity abnormal parameter first
   - when multiple parameters share the same highest severity, break ties using
     the existing display order:
     `Heart Rate`, `Blood Pressure`, `Temperature`, `SpO2`, `Resp Rate`
5. The helper must not infer the source by taking `out[0]` from
   `generate_alerts()` unless the implementation explicitly enforces the same
   highest-severity-then-tie-break policy. Fixed-order alert output alone is
   not sufficient.
6. `paint_status_banner()` shall preserve the current text behavior:
   - simulation off: keep the current centered device-mode message unchanged
   - simulation on: keep the current rolling simulation message unchanged
7. When the latest patient state is `WARNING` or `CRITICAL` and the helper
   returns a source, `paint_status_banner()` shall draw a compact, fixed
   trailing badge inside the banner containing only the approved short source
   token:
   - `HR`
   - `BP`
   - `Temp`
   - `SpO2`
   - `RR`
8. The source badge shall remain visually subordinate to the active-alert list:
   it is a quick cue, not a full explanation. It must not imply that the named
   parameter is the only abnormality.
9. The banner paint path shall reserve or mask a fixed-width area for the badge
   so the rolling simulation text does not overwrite it and the badge does not
   obscure the required device-mode or simulation disclosure.
10. Badge clearing shall be automatic from the current patient/session state:
    no patient, no latest reading, normal state, logout, patient clear, or
    simulation-disabled mode shall produce no source badge.
11. The active-alert list and patient summary shall remain unchanged in this
    issue. If implementation pressure starts to widen scope into detailed text,
    localization, or summary changes, stop and split the work.

## Files Expected To Change

Expected implementation files:

- `include/alerts.h`
- `src/alerts.c`
- `src/gui_main.c`

Expected verification files:

- `tests/unit/test_alerts.cpp`
- `dvt/DVT_Protocol.md` if the team records this as a distinct GUI/manual check

Files expected not to change:

- `src/vitals.c`
- `src/patient.c`
- `src/localization.c`
- `include/localization.h`
- active-alert ordering and message wording in `generate_alerts()`

## Requirements And Traceability Impact

- This design intentionally preserves the currently approved banner text paths
  from `SWR-GUI-010` and `SWR-GUI-011` by adding a compact badge rather than
  replacing existing device-mode or rolling simulation text.
- Requirements directly reused or adjacent:
  - `SYS-005`
  - `SYS-006`
  - `SYS-011`
  - `SYS-014`
  - `SWR-VIT-005`
  - `SWR-VIT-008`
  - `SWR-ALT-001` through `SWR-ALT-004`
  - `SWR-GUI-003`
  - `SWR-GUI-010`
  - `SWR-GUI-011`
- No new threshold, classification, or NEWS2 requirement is expected.
- For a narrow MVP, implementation can proceed without immediate SYS changes if
  the existing device-mode and rolling simulation wording remains intact.
- If the team wants the source badge itself to become an explicitly approved UI
  behavior, a focused SWR and traceability update should be added later,
  likely as an extension of `SWR-GUI-003`, without claiming any change to the
  underlying clinical logic.

## Medical-Safety, Security, And Privacy Impact

Medical safety:

- No intended change to thresholds, aggregate alert level calculation, RR
  missing-data handling, NEWS2 behavior, or treatment guidance.
- Primary benefit is quicker first-glance orientation to the parameter that
  currently drives the aggregate abnormal state.
- Primary risk is a misleading or stale badge on a high-visibility banner.
- The design remains acceptable only if:
  - highest severity wins before any tie-break
  - RR missing-data behavior is preserved
  - the badge clears on normal or absent state
  - existing device-mode and simulation disclosures remain visible
  - the active-alert list stays the detailed source of truth

Security:

- No new authentication, authorization, or privilege boundary is introduced.
- The badge remains inside the existing authenticated desktop session.

Privacy:

- No new patient data category, persistence path, export path, or network path
  is introduced.
- The badge is derived only from patient data already shown in the dashboard.

## AI/ML Impact Assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: not applicable
- Input data: existing deterministic vital-sign values only
- Output: deterministic source badge token derived from approved rule-based
  alert logic
- Human-in-the-loop limits: unchanged from the current system
- Transparency needs: standard software verification only; no AI explainability
  concern is introduced
- Dataset and bias considerations: not applicable
- Monitoring expectations: standard unit and GUI verification only
- PCCP impact: none

## Validation Plan

Automated validation should focus on the new source-selection helper, not on
retesting the existing alert list contract.

Expected unit coverage:

- no source returned for fully normal vitals
- no source returned when RR is not measured and no other parameter is abnormal
- later critical abnormality beats earlier warning abnormality
- later warning abnormality does not beat an earlier critical abnormality
- deterministic tie-break when two parameters share the same highest severity
- RR can be selected only when it is measured and is the highest-severity or
  tie-break winner
- existing `generate_alerts()` ordering and output remain unchanged

Expected GUI/manual coverage:

- simulation mode still shows the rolling message while abnormal states add the
  fixed source badge
- device mode still shows the existing device-mode message and no source badge
- badge clears after patient clear, logout, or return to normal state
- multi-abnormal readings still require the active-alert list for full detail

Recommended validation commands:

```powershell
run_tests.bat
ctest --test-dir build --output-on-failure -R "Alert"
```

Expected validation outcome:

- source selection is deterministic and severity-first
- no stale badge persists across clear or reset conditions
- approved device-mode and simulation text paths remain visible
- the feature remains a bounded UI clarification rather than a new alerting
  workflow

## Rollback Or Failure Handling

- If implementation cannot preserve the current banner disclosures while adding
  a readable badge, stop and split the problem rather than replacing the
  approved message behavior inside this issue.
- If source selection requires changing `generate_alerts()` ordering or alert
  thresholds, stop and treat that as out of scope for this issue.
- If the badge causes confusion in manual review, rollback is straightforward:
  remove the new source-selection helper and badge paint path and restore the
  prior color-only banner behavior.
