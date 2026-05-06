# Risk Note: Issue 57

Issue: `#57`
Branch: `feature/57-read-only-active-alarm-limit-summary-dashboard`

## proposed change

Add a compact, read-only dashboard summary of the currently active alarm limits
for the monitored parameters so authenticated operators can confirm the active
threshold context without opening the Settings window. The intended scope is a
presentation-layer addition only. It must not change alarm thresholds, alarm
generation, NEWS2 scoring, persistence rules, or any edit workflow.

## product hypothesis and intended user benefit

The product hypothesis is credible and bounded: when operators review a live
dashboard during monitoring or handoff, they benefit from seeing the active
alarm-limit context beside the vital-sign and alert summary instead of
navigating into Settings. The expected user benefit is reduced navigation
friction and faster confirmation of whether an observed alert is being judged
against default or customized thresholds.

## source evidence quality

Source evidence quality is adequate for product-discovery direction, not for
clinical claims. The cited Mindray marketing page and Philips symbols page are
vendor UX references only. The cited Mindray operating manual is the strongest
source because it is procedural documentation rather than marketing, but it is
still evidence of common UI patterns, not evidence that showing alarm limits
improves patient outcomes. Together, the sources are sufficient to justify a
generic, non-copying read-only summary concept.

## medical-safety impact

This is a presentation-only candidate, but it is safety-relevant because
displayed threshold context can influence how an operator interprets the
meaning of a warning or critical state. The primary hazard is not a new
clinical algorithm; it is operator misinterpretation if the summary is stale,
incomplete, ambiguous about patient/session context, or visually mistaken for
an editable control. If implemented correctly, the feature can reduce context
switching without changing certified domain behavior.

## security and privacy impact

No new patient-data category, network path, or persistence mechanism is needed.
Privacy impact is low because the summary uses the same authenticated dashboard
context that already exposes vital signs and alerts. The main security and data
integrity risk is display correctness: a stale or incorrectly sourced summary
could misrepresent the active alarm configuration. The feature must remain
read-only, session-scoped, and free of export, print, or unauthenticated
exposure behavior.

## affected requirements or "none"

Existing foundations are relevant, but the approved requirement set does not
yet define a dashboard alarm-limit summary surface. The feature should trace to
existing user-need and dashboard foundations such as `UNS-010`, `UNS-014`,
`SYS-014`, `SWR-GUI-003`, `SWR-ALM-001`, and `SWR-GUI-009`, and it should add
new downstream requirement text for the read-only summary itself during design.

## intended use, user population, operating environment, and foreseeable misuse

Intended use: show the active numeric alarm-limit context for the current
dashboard session as a quick reference during monitoring and handoff.

User population: trained clinical staff already authorized to use the monitor
dashboard, including bedside clinicians and ward operators.

Operating environment: authenticated Windows workstation sessions running the
live dashboard in simulation mode today and equivalent monitor sessions in the
same application architecture later.

Foreseeable misuse:
- the operator assumes the summary is editable or that it confirms a save
  action by itself
- the operator assumes the summary reflects a different patient, preset, or
  unsaved Settings state
- the operator treats the summary as a clinical recommendation instead of a
  passive display of configured thresholds
- the implementation crowds or visually weakens existing alert banners, tiles,
  or active-alert text

## severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

Severity: moderate, because an incorrect threshold summary could contribute to
delayed or inappropriate interpretation of an alarm context even though the
underlying alarm engine remains unchanged.

Probability: low to medium before controls, because UI-state mismatch, stale
refresh, ambiguous labeling, or summary truncation are plausible if the design
is casual.

Initial risk: medium for a pilot presentation feature.

Risk controls:
- render only the active loaded values from the single runtime source of truth
  (`g_app.alarm_limits` or its design successor), not from fixed clinical
  thresholds, copied text, or reconstructed defaults
- make the summary visibly read-only and visually distinct from editable
  Settings controls
- refresh the summary on dashboard initialization and whenever alarm limits are
  applied or reset in Settings
- preserve the prominence of active alerts, status banner text, and current
  vital tiles; the summary must not obscure or downgrade higher-priority safety
  signals
- if a default/custom badge is included, define and verify the authoritative
  default baseline first; otherwise omit that badge from the MVP
- use the same parameter order, units, and naming as the existing Settings and
  alert-limit model to reduce interpretation drift

Verification method:
- targeted automated checks for formatting and any default/custom comparison
  logic added during implementation
- manual GUI verification that the summary matches the loaded Settings values
  on startup, after Apply, and after Reset Defaults
- regression validation that no alarm threshold logic, persistence behavior, or
  alert generation behavior changed
- layout review confirming the summary remains readable without masking status
  banner or active-alert information

Residual risk: low after the controls above.

Residual-risk acceptability rationale: acceptable for this pilot because the
feature remains read-only, does not alter clinical thresholds or decision
logic, and can be bounded to a display-only supplement to already available
Settings information.

## hazards and failure modes

- the dashboard shows stale alarm-limit values after the user changes Settings
- the summary renders factory defaults or fixed clinical thresholds instead of
  the active loaded configuration
- the UI implies the summary is editable or saved when it is not
- a default/custom indicator is computed against the wrong baseline and creates
  false confidence about the active preset
- truncation, condensed formatting, or unit omission causes the operator to
  misread the displayed threshold
- the new summary displaces or visually competes with active alert text or
  status banners

## existing controls

- the issue scope explicitly excludes threshold changes, diagnosis logic, alarm
  acknowledgement, and persistence changes
- the repo already stores active alarm limits in a dedicated structure and
  loads them at dashboard startup via `alarm_limits_load(&g_app.alarm_limits)`
- the Settings panel already exposes the editable alarm-limit controls and
  save/reset actions, reducing pressure to invent new edit behavior on the
  dashboard
- the architecture separates presentation-layer work from the alarm-limit logic
  and other clinically relevant domain behavior
- the implementation and tests already establish runtime alarm-limit defaults
  and save/load behavior in `src/alarm_limits.c` and
  `tests/unit/test_alarm_limits.cpp`

## required design controls

- keep the MVP to a compact read-only summary of active values already held in
  memory; no inline editing, no extra persistence, and no workflow changes
- bind the display to the active runtime limit object and update it from the
  same events that change that object
- show an explicit read-only cue and keep the summary visually subordinate to
  active alerts and status severity indicators
- prefer simple numeric text over vendor-like mimicry; do not copy competitor
  layout, iconography, or workstation tile structure
- document a new requirement and traceability additions for the dashboard
  summary before implementation is treated as complete
- treat the optional default/custom state as a gated design decision: current
  repository evidence is inconsistent because `requirements/SWR.md` describes
  different factory defaults than the implemented and tested defaults in
  `src/alarm_limits.c` and `tests/unit/test_alarm_limits.cpp`; do not ship a
  comparator badge until that baseline is explicitly chosen

## validation expectations

- build and run the existing automated test suite referenced in the issue after
  implementation
- add or update focused tests for any formatter/state-comparison helper used by
  the summary
- perform a GUI smoke check in simulation mode confirming that displayed alarm
  limits match Settings values before and after Apply and Reset Defaults
- confirm the feature remains read-only, uses no heap allocation, and does not
  alter alarm-limit save/load or alert-generation semantics
- confirm the changed file list stays in presentation, traceability, and test
  scope appropriate for a display-only feature

## MVP boundary that avoids copying proprietary UX

The MVP should be limited to a generic textual or badge-style summary using the
project's own labels, units, spacing, and layout. It should not reproduce
vendor tile compositions, proprietary symbols, monitor facsimiles, or branded
workflow metaphors. A simple summary strip that states active ranges is enough
to test the user hypothesis.

## clinical-safety boundary and claims that must not be made

The feature must not claim to improve alarm detection accuracy, reduce adverse
events, validate alarm settings, recommend threshold choices, or provide any
clinical decision support. It may only claim to expose the currently active
configured limits more conveniently within the authenticated dashboard.

## whether the candidate is safe to send to Architect

Yes. This candidate is safe to send to Architect as a bounded presentation-only
feature provided the design keeps the feature read-only, preserves the
prominence of existing alert signals, and either omits the default/custom badge
from the MVP or first resolves the authoritative default baseline.

## residual risk for this pilot

Low residual pilot risk remains from possible operator over-trust in the new
summary or a stale-refresh defect, but those risks are manageable with the
design and verification controls above. The feature is appropriate for pilot
design work because it does not expand clinical logic or automation scope.

## human owner decision needed, if any

Only if Product wants a default/custom indicator in the MVP. In that case, the
owner or architect must explicitly choose the authoritative default baseline
before design finalization because the current requirement text and current
implemented/tested defaults do not match. If Product accepts a numeric
read-only summary without that comparator badge, no additional human decision is
needed to move to design.
