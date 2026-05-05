## Risk Note: Issue 38

Issue: `#38`  
Branch: `feature/38-export-patient-session-review-snapshot`

## proposed change

Add a manual `Export Session Review` action to the authenticated GUI that writes
a deterministic local text or CSV artifact for the active single-patient
session. The requested scope is an output-only snapshot for handoff, design
verification evidence, and product demonstrations. It should package existing
session state only:

- patient demographics already held in `PatientRecord`
- current simulation/device context
- current alarm-limit context
- latest aggregate status and active alerts
- the bounded in-memory session history already stored in `readings[]`

The issue does not propose new thresholds, alarm behavior, NEWS2 logic,
diagnostic advice, background archival, network export, or multi-patient
central-station workflows.

## medical-safety impact

This is not a direct control-path feature, but it affects medical-safety
indirectly because exported artifacts can influence handoff, retrospective
review, DVT evidence, and human understanding of recent deterioration. The main
safety risks are not misclassification by core algorithms; they are context
errors:

- stale or incomplete exported data being mistaken for the current monitor state
- wrong-patient or wrong-session data being exported after clear/admit/reset
- truncation of the bounded session history being hidden from the reviewer
- divergence between live GUI state, `patient_print_summary()`, and the export
  artifact

If implementation remains session-scoped, read-only, and derived from existing
validated data structures, the direct medical-safety impact is low to moderate
and acceptable for pilot design work.

## security and privacy impact

Privacy impact is material because this feature introduces a new patient-data
persistence path. The issue explicitly asks for local text or CSV output in the
app directory, which means protected health information or demo-identifiable
data could remain on disk after the session ends.

Current repo controls show mixed precedent:

- `users.dat` is written with owner-read/write-only permissions in
  `src/gui_users.c`.
- `monitor.cfg` is resolved relative to the executable directory in
  `src/app_config.c`, but its current write path does not itself establish a
  restrictive patient-data export policy.

The export path therefore requires explicit security controls. Without them, the
highest credible non-clinical harm is unintended disclosure of local patient
data, mistaken attachment of the wrong snapshot, or reuse of stale files in DVT
or handoff discussions.

## affected requirements or "none"

Existing requirements that this feature touches by context:

- `UNS-008`, `UNS-009`, `UNS-010`, `UNS-013`
- `SYS-008`, `SYS-009`, `SYS-011`, `SYS-013`, `SYS-014`
- `SWR-PAT-006`, `SWR-GUI-003`, `SWR-GUI-004`, `SWR-GUI-010`

New or amended requirements will likely be needed for:

- manual session snapshot export behavior
- deterministic naming and overwrite rules
- file location and access-permission controls
- session-boundary and truncation disclosure
- validation of exported content parity with existing session summary data

## intended use, user population, operating environment, and foreseeable misuse

- Intended use: allow an authenticated operator to capture a bounded,
  deterministic review artifact from the active patient session for local
  handoff, DVT evidence, or product review.
- User population: trained clinical or demo users already authenticated into the
  Windows desktop application.
- Operating environment: local Windows workstation running the pilot GUI in
  simulation or device mode, with export limited to a local filesystem path.
- Foreseeable misuse:
  - treating the snapshot as a live monitor view rather than a point-in-time export
  - assuming the file is a complete longitudinal record beyond the bounded session
  - exporting the wrong patient after admit/refresh or clear-session transitions
  - sharing the local text/CSV file outside the pilot without privacy review
  - assuming the file is a formal medical record, EMR export, or regulatory report

## severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

| Item | Assessment |
| --- | --- |
| Severity | Moderate. An incomplete or wrong-patient snapshot could mislead handoff or review, and local plaintext export can expose patient-identifiable data. |
| Probability | Possible without controls because the feature adds a new persistence surface and duplicates presentation logic if implemented carelessly. |
| Initial risk | Medium |
| Key risk controls | Manual user-triggered export only; authenticated-session access only; derive all clinical values from existing validated session state; clearly mark the artifact as a session review snapshot; disclose `MAX_READINGS` bounds and any truncation; clear session identity correctly on admit/clear/logout; create files with restrictive local permissions; avoid network or background export. |
| Verification method | Unit tests for serializer content, deterministic naming, overwrite/truncation behavior, and reset boundaries; integration or DVT checks that exported fields match current GUI/session state and `patient_print_summary()` semantics; manual verification of file location/permissions on supported Windows setups. |
| Residual risk | Low if controls are implemented and validated. |
| Residual-risk acceptability rationale | Acceptable for this pilot because the feature remains an operator-invoked adjunct artifact, not a diagnostic engine or alert-control path, and because current live monitoring remains the primary safety surface. |

## hazards and failure modes

| Hazard | Failure mode | Potential harm |
| --- | --- | --- |
| Wrong-patient export | Snapshot survives patient clear/admit transitions or captures stale demographics | Handoff or review uses another patient's data |
| Incomplete retrospective picture | Export omits that history is bounded to `MAX_READINGS` or silently drops entries | Reviewer underestimates instability or assumes the record is complete |
| Stale export mistaken for live state | File lacks clear generation time, session boundary, or "snapshot" wording | Delayed or inappropriate human response during handoff or demo review |
| Privacy leakage | Text/CSV file is stored with weak filesystem protections or copied casually | Unauthorized disclosure of patient or demo data |
| Cross-surface drift | Export formatting diverges from `patient_print_summary()` or active-alert semantics | Conflicting evidence in DVT, review, or design discussions |
| Silent overwrite or ambiguous naming | Deterministic filename replaces older artifact without clear policy | Wrong file is reviewed or prior evidence is lost |

## existing controls

- Authentication is already required before access to patient data and
  monitoring functions.
- Session data is already bounded in memory via `PatientRecord.readings[]` and
  `MAX_READINGS`.
- `patient_add_reading()` rejects overflow instead of silently mutating older
  readings.
- `update_dashboard()` already derives history rows from `readings[]` and active
  alerts from `generate_alerts()`, reducing the need for new clinical logic.
- `do_clear()` resets the in-memory patient session, which gives a clear
  existing session-boundary hook.
- `users.dat` already demonstrates a restrictive local file-permission pattern
  that export code can reuse.

These controls reduce implementation risk, but they do not yet define a safe
export-retention policy or snapshot labeling policy.

## required design controls

- Keep export manual and operator-triggered. No automatic periodic export, sync,
  email, HL7, EMR, or background archival in this MVP.
- Reuse existing validated session state and alert logic. Do not add new
  clinical calculations, thresholds, or severity rules for the export format.
- Mark the artifact explicitly as `Session Review Snapshot` with generation
  timestamp, patient identity, session reading count, and data-format version.
- State the retention boundary in the artifact itself, including that history is
  limited to the current session and at most `MAX_READINGS` readings.
- Define deterministic naming that is predictable but not dangerously
  ambiguous. The overwrite policy must be explicit and testable.
- Apply local file-permission controls at least as strict as the existing
  `users.dat` creation path.
- Clear or separate session identity correctly across logout, clear-session, and
  patient re-admit flows so the export cannot mix sessions.
- Keep the snapshot read-only. No acknowledge, dismiss, or clinical action
  workflow should be attached to it.
- Prefer a single formatting source of truth or shared data-mapping layer so
  console summary and exported evidence do not drift.

## validation expectations

- unit tests confirming the export contains the intended existing fields only
- unit tests for deterministic filename generation and overwrite behavior
- unit tests or integration tests for clear-session, logout, and patient-reinit
  boundaries
- tests that disclose bounded history and do not silently claim more than
  `MAX_READINGS`
- parity checks between exported status/alerts/history and current GUI session
  data derived from `PatientRecord` and `generate_alerts()`
- manual verification that created files use the intended local path and
  permissions on supported Windows environments
- traceability updates for any new UNS/SYS/SWR entries introduced by export

## residual risk for this pilot

Residual risk remains that users may over-trust a convenient local artifact or
copy it outside the intended pilot workflow. Residual risk also remains around
Windows deployment differences if the chosen output directory cannot reliably
enforce private local access. For this pilot, the residual risk is acceptable
only if the UI and file contents clearly communicate snapshot scope and the
implementation applies restrictive local file handling.

## human owner decision needed, if any

Human owner decision is needed on two points before implementation finalization:

- whether storing patient snapshots in the executable/app directory is
  acceptable for supported Windows deployment contexts, or whether a different
  local-only path with stronger privacy properties is required
- whether deterministic naming should allow replacement of the current
  session's prior export or require an explicit user confirmation/retention rule

## product hypothesis and intended user benefit

The hypothesis is credible: operators, testers, and reviewers already have live
session views and a console summary, but they lack a reusable artifact for
handoff, DVT attachments, and side-by-side design review. A bounded single-click
snapshot could reduce manual transcription and make product-review evidence more
consistent without changing patient-monitoring behavior.

## source evidence quality

Source evidence quality is moderate for market/workflow relevance and low for
clinical-benefit claims.

- Philips IntelliVue Information Center brochure supports that retrospective
  review and export workflows are standard in monitoring systems, including
  trend/review applications and data export capabilities:
  `https://www.documents.philips.com/doclib/enc/fetch/2000/4504/577242/577243/577247/582646/583147/IntelliVue_Information_Center_Brochure.pdf`
- GE HealthCare CARESCAPE Central Station product pages support that review of
  current and past patient data, including post-discharge data and multi-hour
  trends, is a real monitoring workflow:
  `https://www.gehealthcare.com/en-ph/products/patient-monitoring/patient-monitors/carescape-central-station`
- Mindray operator documentation supports that trend review, printed reports,
  and CSV-style export workflows exist in centralized monitoring products:
  `https://www.mindray.com/content/dam/xpace/en_us/service-and-support/training-and-education/resource--library/technical--documents/operators-manuals-1/centralized/H-046-010879-00_BeneVision_CMS_Ops_Manual-16.0.pdf`

These are vendor marketing or operator materials, not independent clinical
evidence. They are adequate to justify product-discovery scope and a
non-copying MVP, but they must not be used to claim clinical effectiveness,
workflow safety, or regulatory equivalence.

## MVP boundary that avoids copying proprietary UX

The MVP should stay narrow and repo-native:

- single-patient export from the active local session only
- UTF-8 `.txt` or `.csv` only
- existing repo fields and terms only
- no imitation of competitor report layouts, brand terminology, central-station
  dashboards, full-disclosure viewers, or multi-patient workflows
- no HL7, EMR, PDF report packages, waveform export, or post-discharge archive

This preserves the product hypothesis without copying proprietary UX patterns or
overstating scope.

## clinical-safety boundary and claims that must not be made

The design must not claim that the export:

- is a live monitoring surface
- is a complete legal medical record
- is an EMR/HL7 integration
- adds diagnostic interpretation, treatment advice, or alarm acknowledgement
- is de-identified or safe for uncontrolled sharing
- preserves data beyond the current bounded local session unless that behavior
  is explicitly designed, reviewed, and requirement-backed later

## whether the candidate is safe to send to Architect

Yes. This candidate is safe to send to Architect if the design stays manual,
local-only, session-scoped, and output-only, and if privacy/file-permission
controls are treated as first-class requirements rather than implementation
details.
