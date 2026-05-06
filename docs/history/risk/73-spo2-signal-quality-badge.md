# Risk Note: Issue 73

Issue: `#73`
Branch: `feature/73-spo2-signal-quality-badge`

## proposed change

Add a compact, read-only signal-quality badge to the existing SpO2 dashboard
tile so operators can distinguish between a displayed SpO2 number that has a
known local quality state and one whose quality is unknown.

The intended MVP remains presentation-only. It does not change SpO2 thresholds,
NEWS2 scoring, alert generation, patient-record storage, alarm escalation, or
device connectivity behavior. The safest implementation boundary is a
deterministic UI mapping from a small explicit quality-state enum to badge text
shown beside the existing SpO2 value.

## product hypothesis and intended user benefit

The product hypothesis is that a compact signal-quality cue will reduce false
confidence in a raw SpO2 number and improve review ergonomics for trained
operators already using the live dashboard.

The intended user benefit is faster recognition that a displayed value may need
confirmation because signal confidence is poor or unknown, without requiring the
user to inspect a waveform or separate device screen.

## source evidence quality

Evidence quality is adequate for product-discovery direction but not for a
clinical claim.

- The issue cites public competitor IFU material as precedent that mainstream
  monitoring products surface signal-quality information near SpO2 workflows.
- The cited sources are still best treated as unverified product-discovery
  context here. They support "this kind of UI exists" more than "this exact UI
  improves outcomes".
- The repository evidence is stronger for local feasibility than for clinical
  effectiveness: the current Win32 dashboard already owns tile repainting,
  SpO2 presentation, and HAL-driven refresh.

Conclusion: the evidence is sufficient to justify a narrow, non-copying,
display-only MVP, but not sufficient to justify new clinical claims, waveform
inference, or automated decision support.

## medical-safety impact

This is not a direct diagnosis, treatment, threshold, or alarm-logic change,
but it does alter how clinicians may interpret a live oxygen-saturation value.

The primary safety risk is interpretive bias:

- A misleading "good" quality badge could create false reassurance around a
  low, stale, or artifact-prone SpO2 value.
- A misleading "poor" badge could cause an operator to discount a genuinely
  important low SpO2 reading or delay escalation.
- Reusing the existing alert color semantics too closely could blur the
  distinction between physiologic severity and measurement confidence.

Because the badge is display-only and the operator remains in the loop, the
direct software contribution to harm is bounded. The residual concern is still
meaningful because SpO2 is tied to hypoxaemia recognition under `UNS-004` and
the dashboard is designed for rapid interpretation under `UNS-014`.

## security and privacy impact

No new patient-data category, authentication path, privilege boundary, network
endpoint, or storage mechanism is required for the display-only MVP.

Security/privacy risk stays low if the feature uses only an in-memory quality
state and does not add logging, export, telemetry, or persistence. If future
scope introduces device-derived quality metadata, that metadata should remain
inside the existing HAL-to-GUI trust boundary and should not be treated as a
separate clinical record without traceability updates.

## affected requirements or "none"

Adjacent existing requirements likely affected by design and traceability:

- `UNS-004` SpO2 monitoring
- `UNS-014` graphical dashboard presentation
- `SYS-014` graphical vital-signs dashboard
- `SYS-015` hardware abstraction layer, if quality state originates from HAL
- `SWR-GUI-003` colour-coded vital-sign display
- `SWR-GUI-004` dashboard presentation/data-entry context

New requirement text will likely be needed to define the badge vocabulary,
fallback behavior for missing quality input, and the rule that signal quality
must not alter alert severity, NEWS2, or threshold classification.

## intended use, user population, operating environment, and foreseeable misuse

- Intended use: provide a secondary confidence cue alongside the displayed SpO2
  value on the existing monitoring dashboard.
- Intended user population: trained clinical staff using the pilot monitor.
- Operating environment: Windows workstation dashboard in simulated or future
  device-backed monitoring workflows.
- Foreseeable misuse: treating the badge as a diagnostic statement, assuming
  "good quality" means the patient is clinically stable, assuming "poor
  quality" means a low SpO2 value can be ignored, or assuming an absent quality
  source should default to a positive state.

## severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

- Severity: serious, because misinterpretation of oxygenation data can
  contribute to delayed recognition of hypoxaemia even though the feature does
  not change the numeric value itself.
- Probability before controls: low to medium, because UI ambiguity and stale or
  inferred quality states are plausible if the badge semantics are underdefined.
- Initial risk: medium.
- Risk controls:
  - Keep the feature display-only with no effect on thresholds, alerts, NEWS2,
    or downstream scoring.
  - Define an explicit finite quality-state vocabulary, including `Unknown`,
    and default to `Unknown` whenever the source is missing, stale, disabled,
    or not supported.
  - Do not derive signal quality heuristically from the SpO2 numeric threshold
    alone.
  - Use wording and styling that differentiate quality from alert severity; the
    badge must not reuse `NORMAL/WARNING/CRITICAL` semantics in a way that can
    be mistaken for physiologic status.
  - Update requirements and traceability before implementation.
- Verification method:
  - Requirement review confirming display-only scope and fallback behavior.
  - Unit tests for badge-state mapping, especially missing/stale input paths.
  - GUI verification that badge changes do not alter tile alert color, banner
    state, NEWS2 score, or manual/data-feed behavior.
  - Negative-path checks that device mode or unsupported inputs render
    `Unknown` rather than a positive quality state.
- Residual risk after controls: low.
- Residual-risk acceptability rationale: acceptable for this pilot if the badge
  remains a bounded UI cue, defaults fail-safe to `Unknown`, and is explicitly
  prevented from acting as a clinical classifier or alarm modifier.

## hazards and failure modes

- Badge shows a positive quality state for a stale or mismatched reading.
- Badge quality state is inferred from SpO2 numeric range rather than actual
  signal provenance.
- Badge uses the same red/amber/green semantics as alert severity and is
  misread as a second physiologic alarm channel.
- Missing or unsupported quality input silently defaults to a reassuring state.
- Badge remains visible in device-disabled or `N/A` states, implying confidence
  where no live source exists.
- Future hardware integrations provide non-standard quality fields that are
  mapped inconsistently across devices.
- Designers copy competitor wording/layout too closely instead of implementing a
  minimal independent MVP.

## existing controls

- The issue itself constrains scope to presentation-only behavior and excludes
  threshold changes, NEWS2 changes, alarm logic changes, waveform analysis, and
  hardware expansion beyond minimal badge-driving state.
- `SYS-014` and `SWR-GUI-003` already centralize tile rendering and repaint
  behavior in the dashboard.
- `SYS-015` already isolates acquisition behind the HAL, which is the right
  boundary if a future quality-state input is added.
- The current UI already distinguishes device-disabled `N/A` behavior from live
  simulated data, which is a useful fail-safe precedent for a future `Unknown`
  quality state.

## required design controls

- Introduce a small deterministic signal-quality enum with an explicit `Unknown`
  state and a documented source-of-truth owner.
- Keep signal quality orthogonal to clinical severity: no alarm suppression, no
  NEWS2 weighting, no threshold overrides, no alert-color replacement.
- If a color accent is used, keep it visually secondary to the existing tile
  alert background so the SpO2 severity color remains the dominant cue.
- Show quality as unknown or unavailable when no source metadata exists; never
  assume "good" as a default.
- Document that the MVP does not perform artifact detection, waveform quality
  analysis, diagnosis, or treatment recommendation.
- Define a non-copying UX boundary: simple badge text and local layout only,
  not a reproduction of competitor nomenclature, tables, or historical review
  workflows.

## MVP boundary that avoids copying proprietary UX

The MVP should be limited to a compact local badge within the existing SpO2
tile, using repository-native styling and simple deterministic text such as
`Quality: Good`, `Quality: Poor`, or `Quality: Unknown` once design chooses a
vocabulary.

It should not copy competitor layouts, historical quality rows, waveform review
flows, proprietary iconography, or undocumented scoring logic. The issue is
best framed as "surface a local confidence state" rather than "recreate a
Philips SQI experience".

## clinical-safety boundary and claims that must not be made

The feature must not claim that it:

- validates SpO2 accuracy,
- detects artifact unless a validated algorithm exists,
- replaces clinician judgment,
- changes alarm priority,
- determines treatment urgency,
- or proves that a value is clinically reliable for diagnosis.

The defensible pilot claim is narrower: the dashboard can display a local
signal-quality state associated with the presented SpO2 reading.

## whether the candidate is safe to send to Architect

Yes, with constraints.

This candidate is safe to send to Architect if design remains limited to a
display-only badge, introduces a fail-safe `Unknown` state, defines a
traceable deterministic mapping, and preserves full separation from alert
severity and scoring logic.

It should be blocked later if design expands into inferred signal analysis,
clinical claims, or device-agnostic quality scoring without a new intended-use
definition and validation plan.

## validation expectations

- Add or update requirement text for badge semantics, source behavior, and
  fail-safe fallback.
- Add unit coverage for the quality-state mapping and missing-input handling.
- Add GUI verification for: correct badge placement on the SpO2 tile; no impact
  on existing SpO2 alert colors/values; correct `Unknown` behavior in `N/A`,
  disconnected, or unsupported states; and correct refresh behavior when new
  readings arrive.
- Confirm that manual and simulated workflows continue to behave identically
  except for the added read-only cue.

## residual risk for this pilot

Low, provided the implementation does not invent clinical meaning beyond a
bounded quality-state indicator and fails safe to `Unknown` when the source is
uncertain.

The remaining pilot risk is mainly human-factors confusion, not autonomous
software action. That is acceptable for pre-design progression if the controls
above are carried into requirements and architecture.

## human owner decision needed, if any

No blocking human decision is needed to proceed to architecture for the
display-only MVP.

A human product/clinical owner decision is required before any later expansion
that would derive quality from sensor-waveform analysis, make performance or
accuracy claims, or use the badge to influence alarms, scoring, or clinical
workflow.
