# Risk Note: Issue #101 - Export Current Trend Data To CSV

Date: 2026-05-06
Issue: #101 "Feature: export current trend data to CSV"
Branch: `feature/101-export-current-trend-data-to-csv`

## Proposed change

Add a user-triggered local export action that writes the currently retained
trend-review data for the active patient session to a deterministic UTF-8 CSV
file.

Repo observations that bound the change:

- `PatientRecord` retains up to `MAX_READINGS` sequential vital-sign readings
  in memory for the current session, with no heap allocation and no existing
  patient-data export path.
- `trend_extract_*()` helpers and `trend_direction()` already derive trend-view
  data from that retained history and are covered by `tests/unit/test_trend.cpp`.
- `VitalSigns` stores ordered measurements, but it does not currently store a
  wall-clock timestamp field.
- The current trend logic returns a direction for a retained series or window;
  it does not define a validated per-row "trend direction" value for each CSV
  row.

The change is therefore output-only with respect to clinical logic, but it
introduces a new persistence path for patient-linked data and must not invent
unsupported timing or interpretation semantics.

## Product hypothesis and intended user benefit

Hypothesis: when a clinician, reviewer, or tester is already inspecting trend
behavior in the dashboard, a narrow CSV export reduces retyping and creates a
portable review artifact without changing acquisition, thresholds, NEWS2, or
alert logic.

Expected user benefit:

- faster handoff or audit-style sharing of the exact currently retained trend
  window
- reproducible review evidence for DVT, testing, or supervised clinical review
- less manual transcription from the GUI

## Source evidence quality

Evidence quality is adequate for product-discovery scope and weak for any
clinical-effectiveness or workflow-efficiency claim.

- Mindray BeneVision Multi Patient Viewer Operator's Manual is the strongest
  cited source because it is an operator manual, not only marketing material,
  and it explicitly describes trend-data export with CSV output from a review
  workflow. It is still competitor documentation and does not validate safety
  or effectiveness for this pilot.
  Source: https://www.mindray.com/content/dam/xpace/en_us/service-and-support/training-and-education/resource--library/technical--documents/operators-manuals-1/centralized/H-046-021282-00_BeneVision_Multi_Patient_Viewer_Operators_Manual%28FDA%29-5.0.pdf
- Philips Horizon Trends product page supports the narrower claim that trend
  review surfaces are a standard monitoring workflow and that short-window and
  longer-window trend views are clinically familiar. It is marketing material,
  not evidence for export safety or clinical benefit.
  Source: https://www.usa.philips.com/healthcare/product/HCNOCTN172/horizontrendstrendmeasurementdisplay

Conclusion: the sources are sufficient to justify a narrow, non-copying MVP for
local trend export. They are not sufficient to justify broader claims around
medical-record completeness, enterprise interoperability, or clinical outcome
improvement.

## Medical-safety impact

This feature does not change vital-sign thresholds, NEWS2 scoring, alert limits,
or treatment guidance. The safety risk is in persistence and interpretation, not
in new clinical computation.

Primary safety benefits:

- preserves the exact currently retained trend-review values outside the live
  GUI when a user needs to hand off or inspect them elsewhere
- reduces transcription error relative to manual re-entry of visible values

Primary safety risks:

- exporting the wrong patient or wrong session after a clear, reset, or user
  context change
- omitting, reordering, or mislabeling values so the CSV understates or
  misstates deterioration or recovery
- exporting fields that imply unsupported precision, especially wall-clock
  timestamps or per-row trend-direction semantics that do not currently exist
- allowing users to mistake a bounded session export for a complete medical
  record or durable trend archive

Overall medical-safety impact: low-to-moderate if the feature remains a
user-triggered review artifact derived from existing in-memory data, and not a
new interpreted clinical workflow.

## Security and privacy impact

Security and privacy impact is material because this would be the first
patient-linked file-export path in the pilot.

- Exported CSV files can persist outside the authenticated session boundary even
  after logout or patient/session reset.
- Local files may be copied, emailed, synced, or opened on shared workstations.
- If the export includes patient identifiers, confidentiality and wrong-patient
  handling risk increase materially.
- The change must not add network sync, cloud upload, print/PDF generation, or
  hidden temporary exports in this MVP.

The safest MVP is a deliberate local export of the minimum necessary fields,
with clear disclosure that the file may contain patient data and outlive the
current session.

## Affected requirements or "none"

No current approved requirement explicitly authorizes patient-data CSV export.
Existing requirements provide context for retained trend data and current review
surfaces, but not for external file persistence.

Relevant baseline context:

- `UNS-009` Vital Sign History
- `UNS-010` Consolidated Status Summary
- `SYS-009` Vital Sign Reading History
- `SYS-011` Patient Status Summary Display
- `SWR-TRD-001` Trend sparkline and direction detection
- `SWR-PAT-006` `patient_print_summary()` current summary behavior

New requirements and traceability are needed for:

- the export trigger and availability conditions
- CSV schema, units, ordering, and encoding
- patient/session scope and reset behavior
- identifier inclusion or exclusion
- error handling, overwrite behavior, and partial-write handling
- any timestamp field or per-row trend-direction semantics, if those are kept

## Intended use, user population, operating environment, and foreseeable misuse

Intended use:

- a trained clinician, reviewer, or tester exports the currently retained trend
  readings for the active patient session as a local review artifact

User population:

- bedside clinical staff, internal testers, and reviewers using the local
  Windows desktop application

Operating environment:

- single-patient Win32 desktop workflow in the current pilot
- authenticated local session
- in-memory retained history capped at `MAX_READINGS`

Foreseeable misuse:

- treating the CSV as a complete longitudinal chart or legal medical record
- assuming the file remains current after the patient/session has changed
- inferring exact capture times even though current data storage is ordered but
  not timestamped
- assuming a per-row trend-direction field is clinically validated when current
  logic only defines series-level direction

## MVP boundary that avoids copying proprietary UX

The MVP should stay narrow and implementation-led:

- one explicit `Export Trend Data` action in the existing trend/review area
- fixed UTF-8 CSV output for the current patient and current retained session
- at most the currently stored reading window, not historical archives
- no vendor-specific review layouts, duration pickers, or full-disclosure pages
- no PDF, printing, EMR/HL7 integration, cloud sync, or multi-patient export

## Clinical-safety boundary and claims that must not be made

The feature may support review and handoff. It must not claim to:

- provide a complete or legally authoritative medical record
- capture all historical readings outside the retained session window
- represent exact clock times unless validated timestamp capture is added first
- provide per-row trend-direction interpretation unless that semantic is newly
  specified and verified
- replace the live dashboard, active alerts, or clinician judgment
- diagnose, predict, or recommend treatment

## Whether the candidate is safe to send to Architect

Yes, with explicit narrowing constraints.

The issue is safe to send to Architect because it is still a bounded export of
existing deterministic review data and does not change the clinical logic.
However, design should not silently implement two unsupported parts of the issue
body:

- wall-clock timestamps are not currently present in `VitalSigns`, so the CSV
  should use reading index as the canonical ordering field unless a real time
  source is added under new requirements
- current `trend_direction()` logic is series-level, not per-row, so the export
  should either omit row-level direction or express a validated session/window
  summary instead

If those constraints are preserved, the feature is appropriately narrow for
design.

## Severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

| Item | Assessment |
| --- | --- |
| Severity | Moderate: misleading or mis-scoped exports could contribute to wrong-patient review, privacy breach, or incorrect interpretation of deterioration/recovery, even though no thresholds or alerts change. |
| Probability | Possible without controls because this introduces a new persistence path and the issue text currently overreaches the implemented data model in two places: timestamps and per-row direction semantics. |
| Initial risk | Medium |
| Key risk controls | User-triggered local export only; export current in-memory session data only; fixed schema and units; mandatory reading-order field; no fabricated timestamp or per-row direction semantics; explicit patient-data disclosure; clear failure handling; least-necessary identifiers. |
| Verification method | Unit tests for serializer schema, ordering, units, zero/empty-state handling, and deterministic encoding; integration/manual tests for authenticated-session-only behavior, reset/logout boundaries, and spreadsheet/text-editor opening; traceability updates for new requirements. |
| Residual risk | Low if the export remains bounded, user-triggered, and semantically honest about what data is and is not present. |
| Residual-risk acceptability rationale | Acceptable for this pilot because the feature remains an adjunct artifact derived from existing review data, and because live monitoring and alert logic remain the primary safety surfaces. |

## Hazards and failure modes

| Hazard | Failure mode | Potential harm |
| --- | --- | --- |
| Wrong patient or stale session data is exported | Export action is available after patient/session change, or exported data is not clearly tied to the current session window | Reviewer acts on the wrong patient context or shares the wrong data |
| Confidentiality breach after local persistence | CSV containing patient-linked data is saved to an uncontrolled location or retained beyond expected scope | Unauthorized disclosure of sensitive patient information |
| Trend review is misrepresented | CSV rows are missing, misordered, or mislabeled, or units are unclear | Reviewer underestimates or misreads deterioration or recovery |
| False precision is introduced | Export includes invented timestamps or row-level trend-direction fields not backed by the current data model | User trusts unsupported time or interpretation semantics |
| Export silently fails or partially writes | The file is truncated, overwritten unexpectedly, or saved with inconsistent encoding | Review artifact is incomplete or corrupted without the user noticing |

## Existing controls

- Authenticated login is already required before patient data and monitoring
  surfaces are available.
- `PatientRecord` keeps the retained reading history bounded in local memory.
- `trend_extract_*()` and `trend_direction()` already provide deterministic,
  tested trend-review helpers for the existing history window.
- The product already distinguishes current-session data from durable
  configuration files; patient-linked exports are not part of the current
  baseline.
- Session reset and logout semantics already exist and can be reused to define
  when export availability must end.

These controls reduce implementation risk, but they do not by themselves define
safe export semantics.

## Required design controls

- Keep export strictly user-triggered. No auto-save, no background export, and
  no network-connected destination behavior in this MVP.
- Export only the current in-memory patient session window.
- Use deterministic UTF-8 CSV with a fixed column order, explicit units, and
  locale-independent numeric formatting.
- Include a mandatory `reading_index` field as the canonical ordering column.
- Do not add a wall-clock timestamp column unless timestamp provenance,
  retention, and verification are explicitly specified first.
- Do not add per-row trend-direction semantics unless new requirements define
  and verify them. A session-level trend summary is acceptable if clearly
  labelled as such.
- Disable export when there is no active patient or no retained readings.
- Terminate export availability when logout, patient clear, patient change, or
  session reset invalidates the current data.
- Minimize identifiers in the file. If any patient name or other text field is
  included, sanitize CSV/spreadsheet-sensitive content and justify why it is
  necessary.
- Warn the operator that exported files may contain patient data and may persist
  outside the application.
- Make save failures, cancellations, or overwrite decisions explicit; do not
  silently emit partial or ambiguous files.

## Validation expectations

- unit tests for CSV schema, header order, numeric formatting, encoding, and
  deterministic output across repeated exports of the same data
- tests for 0 readings, 1 reading, full `MAX_READINGS`, and `respiration_rate`
  absent (`0`) handling
- tests or manual verification that export is unavailable after logout, patient
  clear, patient change, and session reset
- manual GUI smoke check that exported CSV opens cleanly in both a text editor
  and a spreadsheet without ambiguous units or reordered rows
- traceability updates for new export requirements, especially if timestamps or
  trend-direction fields remain in scope

## Residual risk for this pilot

Residual risk remains that a human may share or rely on an exported CSV outside
its intended bounded-review context. For the pilot, that risk is acceptable only
if the export is deliberate, minimally scoped, and explicit about being a local
artifact rather than a complete clinical record.

## Human owner decision needed, if any

The product owner / architect should explicitly decide:

- whether exported files may include patient identifiers or should remain
  reading-only unless a stronger traceability need exists
- whether exact timestamps are required badly enough to justify adding a real
  timestamp source and corresponding requirements
- whether "trend direction" belongs in this MVP at all, and if so whether it is
  a session-level summary or a newly specified per-row semantic
