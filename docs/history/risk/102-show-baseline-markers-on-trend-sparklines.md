# Risk Note: Issue #102 - Show Baseline Markers on Trend Sparklines

Date: 2026-05-06
Issue: #102 "Feature: show baseline markers on trend sparklines"
Branch: `feature/102-show-baseline-markers-on-trend-sparklines`

## Proposed change

Add a display-only baseline or reference marker to the existing vital-sign
trend sparklines so operators can compare the current session series against a
visual anchor without changing alarms, thresholds, NEWS2, or data capture.

Current repo evidence confirms that the existing trend feature is a bounded UI
overlay on session data only:

- `src/gui_main.c` renders each sparkline from the current patient record using
  `paint_sparkline()` and `trend_extract_*()` helpers.
- `src/trend.c` extracts historical values and computes trend direction only;
  it does not define or store any baseline, target, or deviation semantics.
- `tests/unit/test_trend.cpp` verifies extraction and direction logic, but no
  approved verification currently covers a reference-marker display behavior.

Process finding relevant to this assessment: open issue `#51` already covers
the same baseline-marker feature and is labeled `ready-for-implementation`.
Advancing `#102` independently would create duplicate product scope, split
traceability, and risk inconsistent display semantics for the same clinical UX
idea.

## Product hypothesis and intended user benefit

The product hypothesis is plausible and narrow. A stable visual anchor can make
it faster for a clinician or operator to see whether a current series is above,
below, or near a chosen reference without reading every prior point.

Expected user benefit:

- quicker visual interpretation of the existing sparkline trend
- less mental effort when distinguishing stable drift from meaningful change
- improved dashboard scanability without adding a new alerting path

## Source evidence quality

Source evidence is adequate for product-discovery scope and inadequate for any
clinical-effectiveness claim.

- Philips Horizon Trends official product page explicitly describes a deviation
  bar against a clinician-determined baseline and trend indicators over recent
  windows. This supports the general workflow pattern of reference-based trend
  viewing. Source: https://www.usa.philips.com/healthcare/product/HCNOCTN172/horizontrendstrendmeasurementdisplay
- Philips Horizon Trends brochure states that the display shows how
  measurements relate to baseline or target values and how they are trending.
  This supports the issue's proposed visual-comparison hypothesis, but it is
  vendor marketing material rather than validation evidence. Source:
  https://www.documents.philips.com/assets/20221130/5b2e161dc46f41b98485af5d0021c6a8.pdf
- Philips FAST SpO2 application note supports that IntelliVue monitors expose
  multiple trend views and retrospective review modes, but it does not by
  itself justify a baseline-marker MVP. Source:
  https://www.documents.philips.com/assets/20250210/64a30873c01245d3bb7cb28001801c43.pdf
- The cited CMU article is general workflow marketing and is not direct
  evidence for baseline-marker behavior. Source:
  https://www.usa.philips.com/healthcare/article/cmu-safeguards-patients-supports-frontline-teams

Conclusion: the product hypothesis is supported well enough for a narrow,
non-copying, display-only MVP. The sources do not define safe baseline
semantics, do not support diagnostic claims, and do not justify copying vendor
UX patterns beyond the high-level concept of a visual reference aid.

## Medical-safety impact

If implemented as a read-only overlay, this candidate does not need to change
measurement capture, clinical thresholds, alarm generation, or NEWS2 scoring.
The safety risk is therefore interpretive rather than computational.

Primary safety risks:

- a user interprets the marker as an alarm limit, therapeutic target, or
  clinician-approved goal when it is only a visual reference
- the baseline source is ambiguous or stale, causing the marker to imply a
  misleading patient trajectory
- the marker uses warning/critical colors and is confused with current alarm
  state
- duplicate issues `#51` and `#102` drive conflicting design assumptions about
  what "baseline" means

Overall medical-safety impact: low if the feature remains a constrained visual
aid with explicit semantics, but not acceptable to advance as a second
independent design item while the canonical feature issue already exists.

## Security and privacy impact

No new network path, credential path, external integration, or persistent data
store is required by the intended MVP. Security and privacy impact are low if
the feature remains session-local and display-only.

Controls still needed:

- do not persist any derived baseline outside the current patient session
- do not expose the marker through export, copy, or remote interfaces in this
  MVP without separate review
- keep the feature behind the existing authenticated GUI session

## Affected requirements or "none"

Existing approved requirements are likely affected:

- `UNS-009` Vital Sign History
- `SYS-014` Graphical Vital Signs Dashboard
- `SWR-TRD-001` Trend Sparkline and Direction Detection
- `requirements/TRACEABILITY.md`

The current requirement set covers sparkline rendering and trend extraction,
but it does not yet define:

- what the reference marker means
- when it is shown or hidden
- what data source establishes it
- how it is visually distinguished from alarm cues

A new or revised display requirement is needed before implementation.

## Intended use, user population, operating environment, and foreseeable misuse

Intended use:

- help a clinician or trained operator review current-session vital-sign trends
  on the dashboard more quickly

User population:

- clinical users and supervised operators of the patient monitor dashboard

Operating environment:

- the existing authenticated desktop dashboard in the current session-based
  monitoring workflow

Foreseeable misuse:

- assuming the marker is a clinical target or alarm threshold
- treating the marker as a patient-specific recommendation
- comparing a stale or cross-session reference to current live data
- assuming the marker changes the meaning of the validated alert tile colors

## Severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

- Severity: minor to moderate, because misleading visual comparison could delay
  or distort human interpretation even though validated alert logic remains
  unchanged.
- Probability: medium without controls, because the word "baseline" is
  clinically suggestive and multiple plausible semantics exist.
- Initial risk: medium.
- Required risk controls:
  - consolidate the work under one canonical issue; do not progress duplicate
    issue `#102` independently of `#51`
  - require a human-approved definition of the marker source before design
    begins
  - use neutral, non-alarm styling and explicit labeling as a reference aid
  - keep the marker session-bounded and reset it on patient clear/logout/new
    patient/session rollover
  - ensure the feature never changes alert classification, NEWS2, or alarm
    display state
  - hide the marker when no valid reference exists rather than inventing one
- Verification method:
  - update requirements and traceability before implementation
  - add automated tests for any new baseline-derivation logic
  - perform manual GUI review with stable, rising, falling, and reset/session
    transition scenarios
  - verify that alert states and NEWS2 outputs are unchanged by the feature
- Residual risk after the above controls: low.
- Residual-risk acceptability rationale: acceptable only as an adjunct display
  aid with explicit semantics and only after duplicate scope is consolidated.

## Hazards and failure modes

- marker implies a clinically meaningful target that was never configured
- marker remains visible after patient/session reset and references the wrong
  history
- missing or invalid baseline silently falls back to an arbitrary value
- styling overlaps with warning/critical alarm semantics
- duplicate issue execution creates inconsistent design notes, requirements, or
  validation scope for the same feature

## Existing controls

- the issue text explicitly states that thresholds, NEWS2, alarms, and
  patient-care workflow are out of scope
- `src/trend.c` is currently read-only extraction and direction logic with no
  baseline state or therapeutic recommendation path
- `src/gui_main.c` already confines sparkline rendering to bounded tile space in
  the authenticated dashboard
- `PatientRecord` history is bounded by `MAX_READINGS`
- `tests/unit/test_trend.cpp` already verifies trend extraction and direction
  behavior for the existing sparkline data path

## Required design controls

- close or consolidate duplicate issue `#102` into canonical issue `#51`
  before design proceeds
- define one approved reference semantic before implementation, for example a
  session anchor or an explicitly provided target; do not let implementation
  invent this behavior ad hoc
- avoid proprietary vendor-specific UX patterns such as a full deviation-bar
  workflow, horizon views, or branded terminology
- document exact show/hide rules, reset rules, and invalid-data behavior
- add clear legend or label text so the marker cannot be mistaken for an alarm
  limit
- require manual GUI verification that the marker remains visually distinct from
  alarm colors in NORMAL, WARNING, and CRITICAL tiles

## Validation expectations

- confirm the design names a single approved baseline source and reset rule
- confirm the changed-file scope does not alter thresholds, alerting, NEWS2, or
  data acquisition logic
- run automated regression for trend extraction and any new baseline logic
- manually inspect stable, rising, falling, and patient/session reset cases
- verify the marker is absent or explicitly unavailable when a valid reference
  cannot be established

## MVP boundary that avoids copying proprietary UX

The MVP should stay limited to one simple reference aid inside the existing
sparkline footprint. It should not copy Philips Horizon terminology, deviation
bars, multi-window trend workflows, histogram modes, export features, or
clinician-configured target management.

## Clinical-safety boundary and claims that must not be made

- do not claim improved diagnosis, treatment recommendation, or alarm accuracy
- do not imply the marker is a therapeutic target unless a clinician has
  explicitly set one in a separately reviewed workflow
- do not present the marker as a validated predictor of deterioration
- do not let the marker override or visually compete with current validated
  alert-state cues

## Whether the candidate is safe to send to Architect

No, not as issue `#102` currently stands.

The underlying feature concept can be safe to design later, but this issue
should not move forward independently because:

- it duplicates open issue `#51`, which already owns the same feature scope
- "automatic baseline" semantics remain insufficiently defined for a
  safety-sensitive UI element

## Residual risk for this pilot

Blocked until product ownership consolidates the duplicate and confirms the
reference-marker semantics that Architect is allowed to design against.

## Human owner decision needed, if any

- decide whether issue `#102` should be closed as a duplicate of `#51` or
  otherwise merged into that canonical work item
- define the intended baseline/reference semantic before any design work starts
- confirm that the MVP remains session-local, read-only, and non-alarm-bearing
