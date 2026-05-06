# Risk Note: Issue #58 - Show patient bed/location context in the dashboard header

- Date: 2026-05-06
- Issue: `#58`
- Branch: `feature/58-show-patient-bed-location-context-in-the-dashboard-header`
- Disposition: Safe to send to Architect if the MVP remains display-only and the controls below are carried into requirements and design.

## Proposed change

Add an optional, read-only bedside context field for the active patient and render it in the dashboard header/patient bar and patient summary. The field is intended to help an operator confirm bedside context during monitor reuse, patient transport, or room reassignment. It must not change vital-sign acquisition, alert generation, NEWS2 scoring, alarm limits, authentication, or any patient-care workflow.

## Product hypothesis and intended user benefit

The hypothesis is plausible and appropriately narrow: a visible bed/location cue can reduce operator hesitation and visual search when confirming whether the current dashboard belongs to the expected bedside context. The likely benefit is ergonomic rather than clinical-performance related.

This evidence supports only a local UI-context feature. It does not support automated bed tracking, caregiver assignment, EHR synchronization, transport workflow automation, or any claim that the software can verify patient identity or physical location.

## Source evidence quality

- Philips Patient Information Center iX public IFU: moderate-quality analogous evidence. The manual describes bed labels, selected-location information, and patient/bed context views in a central monitoring product. It supports the general product pattern that bedside context may be shown alongside patient information.
- GE HealthCare B1x5M public product page: low-quality product-discovery evidence. The page exposes a "Bed-to-Bed view" marketing asset, but it does not provide enough detail to derive UI behavior, safety expectations, or workflow claims.
- Repository evidence: strong feasibility evidence. `PatientInfo` currently contains only ID, name, age, weight, and height in `include/patient.h`; `do_admit()` populates those fields and `paint_patient_bar()` renders them in `src/gui_main.c`. That is sufficient to define the technical insertion points without guessing about clinical behavior.

Overall evidence quality is sufficient for a constrained MVP and insufficient for any broader workflow or interoperability scope.

## MVP boundary that avoids copying proprietary UX

- Limit the MVP to one optional manually entered context value such as `bed`, `room-bed`, or `location`.
- Display the value as plain text in existing product-owned surfaces only; do not copy competitor layout, terminology, icons, sector models, census views, or multi-bed workflows.
- Keep the value read-only in the header/summary surfaces after admit/refresh.
- Allow the value to be blank. If blank, show either nothing or a clearly non-alarming placeholder; do not fabricate a location.
- Exclude integration with ADT, EHR, RTLS, telemetry assignment, bed management, or automated transport state.

## Clinical-safety boundary and claims that must not be made

- The field is contextual metadata, not a source of truth for patient identity.
- The product must not claim that it confirms patient identity, validates bed assignment, tracks transport, or proves that a device is physically connected to the displayed patient.
- The value must not influence alarm routing, alert severity, aggregate status, NEWS2, trend logic, or any clinical recommendation.
- If the location is stale, missing, or incorrect, the system should fail safe by remaining informational only rather than asserting correctness.

## Medical-safety impact

Direct medical-safety impact is low because the proposed change does not alter physiologic calculations, alarm thresholds, or acquisition logic. The meaningful risk is indirect: an incorrect or stale location label could create false confidence that the displayed patient context is correct, which could contribute to wrong-patient review, delayed recognition of a mismatch, or handoff confusion.

Because the feature sits next to patient identity information, it should be treated as a safety-relevant UI change even though it is non-clinical metadata.

## Security and privacy impact

Bed and room context is patient-associated operational data and may constitute PHI in context. The feature does not need any new external data flow for the proposed MVP, but privacy scope increases if the new value is persisted, exported, logged, or shown on screenshots.

Required boundary:

- No new network transmission.
- No use in alerts, logs, or status banners beyond the intended patient-context display.
- If later persisted, exported, or synchronized, it must inherit the same protection and traceability treatment as other patient-identifying data.

## Affected requirements or "none"

No approved requirement currently covers bedside location context explicitly.

Likely downstream requirement impact:

- extend or clarify `UNS-008` if the product owner wants bedside context to be part of the patient-association story;
- extend `SYS-008`, `SYS-011`, and `SYS-014`, or add a narrowly scoped new requirement for optional contextual metadata display;
- add corresponding software requirements and verification for data entry, reset/clear behavior, rendering, truncation, and no-effect-on-clinical-logic regression.

## Intended use, user population, operating environment, and foreseeable misuse

- Intended use: trained clinical staff use the monitor on a Windows workstation to review a single active patient's live vitals and associated contextual information.
- User population: bedside clinicians, ward nurses, and other trained operators already expected by the approved UNS.
- Operating environment: ward, bedside, transport staging, or similar monitored clinical settings where the workstation may be reused for different patients.
- Foreseeable misuse:
  - operator assumes the location field is authoritative patient identity proof;
  - stale location remains visible after patient transfer, discharge, or monitor reuse;
  - manual entry typo or ambiguous abbreviation points to the wrong bed;
  - long labels are truncated so that nearby beds become indistinguishable;
  - downstream teams later reuse the field for routing or workflow logic without reevaluating risk.

## Severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

Primary hazardous situation: a stale or incorrect bed/location label is shown next to correct patient demographics, and the operator places undue trust in it.

- Severity: Moderate. The feature is not itself therapeutic or diagnostic, but wrong-patient context can contribute to clinically relevant confusion.
- Probability without controls: Low to occasional, because the value would be manually entered and the workstation is reused across sessions.
- Initial risk: Medium for pilot use.

Required risk controls:

1. Keep the field optional and informational only.
2. Reset the field on new admit, clear-session, logout, and any demo/simulation path that creates a fresh patient context.
3. Separate the field visually from patient identity so it is not mistaken for a verified identifier.
4. Use clear labeling such as `Bed/Location (optional)` or equivalent approved wording.
5. Bound length and truncation behavior so similar beds cannot become indistinguishable without a way to view the full value.
6. Do not feed the field into alarms, summaries of clinical status, trend logic, history ordering, or any routing/integration logic.
7. Update requirements and traceability before implementation so the feature is not delivered as undocumented UI behavior.

Verification method:

- unit tests for any `PatientInfo` or summary-format changes, including blank value, max-length input, truncation, and reset behavior;
- GUI/manual verification for admit with and without a location, new admit after a prior location, clear-session, logout/login, and simulation/demo scenarios;
- regression verification that alerts, NEWS2, alarm limits, and reading history behavior are unchanged.

Residual risk after controls: Low.

Residual-risk acceptability rationale: acceptable for this pilot because the feature remains non-authoritative context, defaults safely when absent, and does not alter clinical logic. The remaining risk is comparable to other demographic-display errors and is manageable with labeling, clearing, and regression controls.

## Hazards and failure modes

| Hazard | Failure mode | Potential effect |
| --- | --- | --- |
| Wrong-patient context cue | stale location survives reuse or transfer | operator reviews correct vitals but assumes wrong bedside context |
| Ambiguous location display | truncation, abbreviation, or formatting collision | nearby beds/rooms appear equivalent |
| Over-trust in metadata | UI implies verified assignment | operator treats context field as identity confirmation |
| Privacy leakage | field appears in logs, exports, or unintended displays | unnecessary exposure of patient-associated location data |
| Scope creep | later reuse in workflow/routing logic without reassessment | safety claim grows beyond validated evidence |

## Existing controls

- Authentication already gates access to patient-facing UI under `SYS-013`.
- The system already displays patient ID, name, age, BMI, and reading count in the active patient bar, which provides a primary identity context separate from the proposed field.
- `patient_init()` zero-fills the patient record and `do_clear()`/logout clear session state, which can support stale-data removal if the new field is incorporated consistently.
- No current architecture path automatically synchronizes location from an external system, which keeps the initial scope constrained.

## Required design controls

- Extend the patient data model in a way that preserves static-memory constraints and deterministic initialization.
- Define one canonical field meaning before implementation: `bed`, `room-bed`, or broader `care location`; avoid silently mixing concepts.
- Ensure blank and unknown states are explicitly designed and tested.
- Make the read-only display text consistent between GUI header/patient bar and any patient summary surface.
- Prevent the field from appearing in alert text, status banners, or any computed clinical output.
- Add traceable test evidence for stale-data clearing across `do_admit()`, `do_clear()`, logout, and simulation/demo flows.

## Validation expectations

- Design spec should name exact UI surfaces, field label, allowed character set/length, and blank-state behavior.
- Implementation should add focused automated tests for initialization and summary formatting.
- Manual GUI validation should include:
  - patient with no location;
  - patient with a normal short location;
  - patient with a long location near the length limit;
  - second patient admitted after the first to confirm no stale carry-over;
  - simulation and scenario actions to confirm they do not leak prior location state.
- Traceability updates should be included before merge, not left as post-hoc documentation cleanup.

## Residual risk for this pilot

Residual pilot risk is low if the MVP remains an optional display-only context cue with no automation or clinical logic coupling. The main residual exposure is human over-trust in a manually entered field. That exposure is acceptable only if the UI makes the field clearly secondary to primary patient identifiers and the value is cleared reliably when patient context changes.

## Human owner decision needed, if any

Two product decisions should be made before implementation, but neither blocks Architect from drafting a constrained design:

1. Choose the canonical semantics and label of the field: `Bed`, `Room/Bed`, or `Location`.
2. Decide whether a blank value should render as hidden/empty or as an explicit neutral placeholder such as `Not entered`.

## Whether the candidate is safe to send to Architect

Yes. Issue `#58` is safe to send to Architect provided the design stays within the display-only MVP boundary described above and carries forward the stale-data, labeling, privacy, and no-clinical-logic-coupling controls.
