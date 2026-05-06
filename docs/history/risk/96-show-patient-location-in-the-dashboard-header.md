# Risk Note: Issue #96 - Show patient location in the dashboard header

Date: 2026-05-06
Issue: #96 "Feature: show patient location in the dashboard header"
Branch: `feature/96-show-patient-location-in-the-dashboard-header`

## Proposed change

Add one optional patient location or bed label to the existing single-patient
dashboard surfaces so operators can confirm patient context at a glance. The
intended MVP is display-only: no alarm logic, NEWS2 logic, diagnosis support,
treatment guidance, or external system integration changes.

Repository observations supporting the change:

- `include/patient.h` `PatientInfo` currently stores only `id`, `name`, `age`,
  `weight_kg`, and `height_m`.
- `src/gui_main.c` `paint_patient_bar()` currently renders patient name, ID,
  age, BMI, and reading count, but no location context.
- `src/patient.c` `patient_print_summary()` currently prints name, ID, age,
  BMI, and readings, but no location field.
- `src/gui_main.c` initializes demo and admitted patients through
  `patient_init()` in multiple paths (`do_admit()`, startup simulation init,
  simulation-mode reinit, and buffer-rollover reset), so a new context field
  would need consistent init and reset behavior.

## Product hypothesis and intended user benefit

Hypothesis: adding a location or bed cue reduces the time needed to confirm the
active patient context and makes the pilot dashboard closer to common
central-station workflows.

Expected user benefit:

- faster visual confirmation that the operator is viewing the intended patient
- less reliance on memory for room or bed assignment during handoff or review
- better demo realism without changing clinical interpretation logic

## Source evidence quality

Evidence quality is adequate for narrow product-discovery scope and weak for
clinical-effectiveness claims.

- The issue cites a GE HealthCare CARESCAPE Central Station product page as
  evidence that patient identity and location context are commonly shown
  together in central monitoring products.
- That evidence is vendor marketing, not independent clinical evidence.
- The cited page could not be directly inspected from this environment because
  the public page returned an access-denied response, so the current rationale
  depends on the issue's summary of the source rather than first-hand review.

Conclusion: the evidence is sufficient to justify a narrow, non-copying,
display-only MVP. It is not sufficient to support claims that the feature
reduces wrong-patient events or improves clinical outcomes.

## Medical-safety impact

This change does not alter thresholds, alert generation, alarm limits, NEWS2,
or treatment guidance. Any safety impact comes from context presentation rather
than new clinical calculations.

Primary potential safety benefit:

- location context can help operators notice when the viewed patient does not
  match the expected room or bed assignment

Primary safety risks:

- stale or wrong location text could survive patient change, clear, or session
  reset and contribute to patient-context mix-ups
- users could over-trust room or bed text and give it equal or greater weight
  than patient name and ID
- ambiguous free-text abbreviations could be interpreted differently by
  different users
- an unset value could be mistaken for confirmed context if the empty state is
  not explicit

Overall medical-safety impact: low if the field remains a secondary,
display-only context cue with explicit reset rules. Risk rises to moderate if
the field is treated as authoritative identity data or persisted without clear
ownership and stale-data controls.

## Security and privacy impact

Location or bed context increases patient identifiability even though it does
not change clinical logic.

- No network sync, export, or remote display should be added in this MVP.
- No cross-session persistence should be added unless requirements are updated
  explicitly to define storage and stale-data behavior.
- The field must remain behind the existing authenticated session boundary.
- The field must clear or refresh on logout, patient clear, patient re-admit,
  simulation startup that auto-creates a patient, and any automatic reset path
  that reinitializes `PatientRecord`.

## Affected requirements or "none"

Existing requirements likely affected:

- `UNS-008` Patient Identification
- `UNS-010` Consolidated Status Summary
- `UNS-014` Graphical User Interface
- `SYS-008` Patient Demographic Storage if location is stored in `PatientInfo`
- `SYS-011` Patient Status Summary Display
- `SYS-014` Graphical Vital Signs Dashboard
- `SWR-PAT-001` and `SWR-PAT-006`
- `SWR-GUI-003` and `SWR-GUI-004`

New derived requirement text is likely needed for:

- optional location-field semantics and maximum length
- exact surfaces that show the field
- fallback text when the field is unset
- clear and reset behavior across patient/session transitions
- whether the field is session-only or persisted

## Intended use, user population, operating environment, and foreseeable misuse

Intended use:

- trained clinical staff use the field as a secondary context cue while viewing
  one active patient in the local dashboard and summary

User population:

- bedside clinicians, ward staff, and internal testers using the Windows
  workstation application

Operating environment:

- single-workstation pilot scope with local static storage and either simulated
  or future hardware-fed readings

Foreseeable misuse:

- using location as the primary patient identifier instead of cross-checking
  name and ID
- leaving the previous patient's location text in place when admitting a new
  patient
- entering ambiguous free-text notes that are not actually location data
- assuming a blank or placeholder location means the assignment is clinically
  verified

## MVP boundary that avoids copying proprietary UX

The MVP should remain narrow and implementation-led:

- one optional text field only
- plain labeled text on existing dashboard and summary surfaces
- no multi-patient board, ward map, bed-management workflow, or remote station
- no vendor-specific layout copying, icons, or central-station mimicry
- no persistence beyond the active session unless separately specified

## Clinical-safety boundary and claims that must not be made

The feature may support context confirmation. It must not claim to:

- verify patient identity by itself
- prove that the bed or room assignment is current or correct
- reduce wrong-patient risk without human cross-check of patient identifiers
- influence alarms, NEWS2, diagnosis, or treatment
- synchronize with ADT, EMR, or bed-management systems

## Whether the candidate is safe to send to Architect

Yes, with constraints. The candidate is safe to send to Architect because it is
display-only and the requested MVP is narrow. Design must keep location clearly
secondary to patient name and ID, default to an explicit unset state, and clear
or refresh the field at every patient/session boundary.

## Severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

| Item | Assessment |
| --- | --- |
| Severity | Moderate: stale or wrong location context could contribute to a patient-context mix-up, but the feature does not change clinical calculations or suppress alerts. |
| Probability | Possible without controls because the field would touch multiple admit, demo, and reset paths. |
| Initial risk | Medium |
| Key risk controls | Keep location visually secondary to patient name and ID; define an explicit unset state; clear or refresh the field on every patient/session init path; bound storage length under static-memory rules; keep the field out of alerting and scoring logic; defer persistence and external integration until separately specified. |
| Verification method | Unit tests for init/reset/truncation if the data model changes; GUI/manual verification for admit, clear, startup simulation, and rollover reset paths; summary/header parity checks; traceability updates for any changed or new requirements. |
| Residual risk | Low if controls are implemented and verified. |
| Residual-risk acceptability rationale | Acceptable for this pilot because the field remains an adjunct context cue and the primary identity signals remain patient name and ID. |

## Hazards and failure modes

| Hazard | Failure mode | Potential harm |
| --- | --- | --- |
| Wrong patient context is shown | Old location survives patient change, clear, or automatic reset | User acts on the wrong patient context during review or handoff |
| Location is over-trusted | UI gives bed/location equal or greater prominence than patient identifiers | Operator relies on room/bed shorthand instead of patient name/ID |
| Ambiguous location entry | Free-text abbreviations or notes are entered without constraints | Staff interpret context differently and misroute attention |
| Privacy exposure increases | Location is later persisted or exported without explicit controls | Patient-identifying context leaks beyond intended local use |

## Existing controls

- `patient_init()` centralizes patient-model initialization and currently clears
  all demographic/session fields.
- `do_admit()`, startup simulation init, and rollover reset already define the
  major patient/session transition points that a new field would need to follow.
- The existing dashboard already treats patient name and ID as the visible
  identity anchors.
- The production codebase already enforces static storage only, which supports
  bounded string handling for any new field.
- Authentication already gates access to the dashboard and patient summary.

These controls reduce implementation risk, but they do not yet define safe
location semantics.

## Required design controls

- Define whether the field is free-text, bed-only, or otherwise structured.
- Cap the stored length and guarantee null termination under `SYS-012`.
- Use explicit labeling such as `Location` or `Bed`; do not present the field
  as a substitute for patient identity.
- Show an explicit unset state rather than fabricating a value.
- Keep patient name and ID visually primary.
- Clear or refresh location on every path that calls `patient_init()` or
  otherwise replaces patient/session context.
- Use the same source field for dashboard and summary output to avoid drift.
- Keep the field out of alert generation, NEWS2, summaries of clinical status,
  and any automation logic.
- Defer persistence and external synchronization until source-of-truth,
  ownership, and stale-data rules are specified.

## Validation expectations

- unit tests for any `PatientInfo` or summary changes, including truncation and
  reset behavior
- GUI/manual verification that no stale location survives admit/refresh, clear
  session, startup simulation, or automatic buffer-rollover reset
- verification that the unset state is visually distinct from a confirmed
  location
- traceability updates for any changed or new UNS/SYS/SWR entries
- copy review to ensure the feature is described as context only, not as a
  verified identity or clinical signal

## Residual risk for this pilot

Residual risk remains that some operators may treat room or bed labels as a
shorthand identity cue in fast-paced workflows. For this pilot, that risk is
acceptable only if patient name and ID remain primary and if stale or unset
states are handled explicitly.

## Human owner decision needed, if any

Product owner and Architect should explicitly decide:

- whether the field is session-only or persisted
- whether the field is unrestricted free text or a narrower structured label
- what exact fallback copy is shown when location is unset
- whether the demo/default patient should start with no location or a clearly
  synthetic location value
