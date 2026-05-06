# Risk Note: Issue #53

Date: 2026-05-06
Issue: `#53` - Feature: show top alert cause beside the aggregate status pill

## proposed change

Add a compact, additive cause badge beside the existing aggregate status summary
so the operator can see which active parameter is currently driving a
`WARNING` or `CRITICAL` state without scanning every tile first.

The feature must remain presentation-only. It must reuse the current alert
classification outputs and must not change thresholds, NEWS2 scoring, alert
suppression, audible behavior, patient workflow, or the full active-alert list.

## product hypothesis and intended user benefit

The product hypothesis is that a small, always-visible explanation of the
aggregate state will reduce operator scan time on a busy dashboard and improve
recognition of the highest-priority active issue.

The intended user benefit is faster orientation, not new clinical insight. The
feature should help trained staff move from "patient is not normal" to "which
parameter needs review first" using existing data already produced by the
system.

## source evidence quality

The issue cites vendor product pages and one public IFU. Those sources are
useful as product-discovery context showing that alarm context, alarm history,
and quick alarm triage are established workflow patterns in patient monitoring.

Evidence quality for this exact feature is still low to moderate because the
external sources are largely marketing or high-level product documentation, not
comparative usability or safety evidence for this specific UI affordance.

Repository evidence is stronger for feasibility than for benefit. The current
code already computes an aggregate alert state and structured per-parameter
alerts in `src/vitals.c`, `src/alerts.c`, and `src/gui_main.c`.

## medical-safety impact

This feature does not introduce new clinical decision logic if it stays within
scope. It is an explanatory display of existing alert outputs.

The main medical-safety risk is misleading prioritization. A wrong or stale
"top cause" could make a clinician look first at the wrong parameter and delay
recognition of the most urgent active abnormality.

That risk is real because `generate_alerts()` in `src/alerts.c` emits alerts in
fixed parameter order, not in highest-severity order. A design that simply
shows the first generated alert could surface a warning-level heart-rate alert
while a critical SpO2 or respiration alert is also active.

Residual safety risk is acceptable for this pilot only if the badge stays
strictly additive, deterministic, and subordinate to the existing tiles and
full active-alert list.

## security and privacy impact

No new data classes, persistence, authentication paths, or network behavior are
introduced by the proposed feature.

The feature will only re-present patient data already visible on the dashboard.
No material privacy or cybersecurity change is expected if the work remains a
local display refinement.

## affected requirements or none

Existing intent appears sufficient at the user and system level if the feature
remains a presentation-layer refinement:

- `UNS-010` Consolidated Status Summary
- `UNS-014` Graphical User Interface
- `SYS-006` Aggregate Alert Level
- `SYS-011` Patient Status Summary Display
- `SYS-014` Graphical Vital Signs Dashboard

Likely software behaviors touched or relied upon:

- `SWR-VIT-005` Overall Alert Level
- `SWR-ALT-001` One Alert Per Abnormal Parameter
- `SWR-GUI-003` Colour-Coded Vital Signs Display

No new clinical threshold or algorithm requirement should be introduced by this
issue. The Architect should decide whether an SWR-level GUI wording update is
needed so the badge becomes explicitly testable once designed.

## intended use, user population, operating environment, and foreseeable misuse

Intended use remains unchanged: a Windows-based bedside monitoring dashboard
used by trained clinical staff to review current patient status and active
alerts.

Intended users are ward clinicians, bedside nurses, and other trained staff who
already interpret the existing alert tiles and alert list.

Operating environment remains the same local workstation dashboard described in
`README.md` and `docs/ARCHITECTURE.md`.

Foreseeable misuse includes:

- treating the badge as the only active issue and ignoring other simultaneous
  alerts
- assuming the badge identifies a clinical root cause rather than the currently
  displayed highest-severity parameter
- trusting a first-listed alert as the top cause even when alerts are not
  severity-sorted
- expecting the badge to change alarm priority, silence behavior, or NEWS2
  response guidance

## severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

- Severity: moderate, because a misleading summary could delay recognition of a
  more urgent abnormal parameter, but the existing parameter tiles and full
  alert list remain available as independent cues.
- Probability: low to moderate without controls, because the implementation
  could easily select the wrong cause if it reuses fixed-order alert output or
  hides simultaneous alerts.
- Initial risk: medium.
- Risk controls:
  - derive the displayed cause from explicit comparison of active alert
    severities, not from the first entry returned by `generate_alerts()`
  - keep the feature additive; never hide, collapse, or replace the existing
    alert list, severity badges, or tiles
  - document deterministic tie-breaking for multiple equally severe alerts so
    the displayed cause does not oscillate between renders
  - show concise factual content only, such as parameter plus measured value;
    do not show treatment advice, diagnosis language, or certainty claims
  - preserve required simulation/live indicators and other existing status cues
    so the new badge does not create a new ambiguity
- Verification method:
  - targeted unit tests or deterministic logic tests for multi-alert severity
    selection and tie cases
  - regression verification that aggregate alert classification itself is
    unchanged
  - manual GUI review in `NORMAL`, `WARNING`, and `CRITICAL` states, including
    multiple simultaneous alerts, no-alert state, missing RR, and long-label
    clipping cases
- Residual risk: low if all controls above are implemented.
- Residual-risk acceptability rationale: acceptable for this pilot because the
  feature summarizes existing alert outputs without changing underlying clinical
  logic and because the detailed alert views remain visible as the primary
  safety control.

## hazards and failure modes

- wrong parameter shown as the top cause because the implementation uses
  fixed-order alert output instead of highest-severity logic
- stale cause badge after a new reading if the status pill and badge update out
  of sync
- multiple simultaneous critical alerts reduced to a single badge in a way that
  encourages under-review of the others
- clipped or ambiguous text that hides the abnormal value or parameter name
- badge shown when there is no active alert, creating false urgency
- UI placement that obscures or conflicts with existing simulation/live status
  indicators

## existing controls

- `overall_alert_level()` already computes the aggregate alert state.
- `generate_alerts()` already produces structured per-parameter alert records.
- The dashboard already shows parameter-specific tiles with independent severity
  coloring.
- The dashboard already shows a full active-alert list in `IDC_LIST_ALERTS`.
- The issue scope explicitly states that thresholds, diagnosis logic,
  suppression, and audible behavior are out of scope.

## required design controls

- Keep the feature presentation-only and additive to current alert displays.
- Select the displayed cause using explicit highest-severity logic and
  documented tie-breaking.
- Do not infer "top cause" from `generate_alerts()` order.
- Do not remove or visually demote the existing alert list as part of this MVP.
- Keep wording generic and factual; avoid competitor wording, layouts, or
  proprietary banner styling.
- If the desired UX requires hiding simultaneous alerts, changing priority
  semantics, or altering clinical workflow, split that into a new issue and
  re-run risk review.

## MVP boundary that avoids copying proprietary UX

The MVP should be limited to a simple local badge or short text label derived
from existing alert data. It should not copy competitor split-screen layouts,
alarm-history workflows, notification phrasing, or vendor-specific color or
banner treatments.

A defensible MVP is "show one concise explanatory cause beside the current
aggregate state while leaving the detailed alerts list unchanged."

## clinical-safety boundary and claims that must not be made

The badge must be framed as an explanatory summary of current alert status, not
as a diagnosis, root-cause engine, treatment recommendation, or proof that only
one parameter needs attention.

The feature must not claim improved clinical outcomes, reduced alarm fatigue, or
faster intervention without human-reviewed evidence. It must not suggest that
secondary active alerts are less important than the displayed cause.

## AI/ML and decision-support impact

This candidate does not add, change, or remove an AI/ML model.

It does depend on existing deterministic rule-based alert outputs. The model
governance concerns are therefore limited to presentation accuracy and human
interpretation rather than training, tuning, or monitoring data.

No training or validation dataset is needed for the MVP if the feature simply
reuses existing deterministic alert logic. No PCCP is indicated for this issue.

If future design introduces heuristic ranking beyond the existing rule-based
severity model, that should be treated as a new decision-support change and
re-reviewed before implementation.

## validation expectations

- confirm by code review that no alert thresholds, NEWS2 logic, or alarm-limit
  behavior changes are introduced
- verify the chosen top-cause selection logic across mixed warning and critical
  combinations, including ties and RR-not-measured cases
- manually review the dashboard in representative simulation scenarios to
  confirm the badge matches the highest-severity active alert and does not hide
  the full alert list
- verify that the new UI element does not collide with existing status, layout,
  or localization constraints

## residual risk for this pilot

Residual risk is low if the feature remains a compact explanatory summary that
accurately reflects existing alert severity and preserves all current detailed
alerts.

Residual risk becomes medium if the design treats the badge as a replacement
for the alert list or uses non-deterministic ranking that can show a less
urgent parameter while a more urgent abnormality is active.

## whether the candidate is safe to send to Architect

Yes, with controls.

This candidate is safe to send to Architect because the issue is specific,
bounded, and explicit about preserving current clinical behavior. The required
controls are mostly about preventing misleading summarization, not about
introducing new medical logic.

## human owner decision needed, if any

No additional human owner decision is needed before design if the scope remains
presentation-only and additive.

Human review is needed before implementation if the design proposes any change
to alert prioritization semantics, suppression behavior, competitor-derived UX
copying, or any wording that could be interpreted as clinical advice.
