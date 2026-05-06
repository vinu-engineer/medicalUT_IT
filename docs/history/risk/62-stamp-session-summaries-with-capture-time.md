# Risk Note: Issue #62

Issue: `#62`  
Branch: `feature/62-stamp-session-summaries-with-capture-time`

## proposed change

Add one session-scoped summary-generation timestamp line to the existing
summary outputs in `patient_print_summary()` and the CLI end-of-session summary
path in `src/main.c`. The added field should describe when the summary artifact
was generated, not when each physiologic reading was acquired. Scope remains
metadata-only: no new persistence, no audit-log retention, no EMR/export
workflow, and no threshold, alert, or NEWS2 logic changes.

## product hypothesis and intended user benefit

The user problem is provenance ambiguity: a copied, pasted, or printed summary
without time context can be mistaken for the current live state or for a newer
summary than it is. The intended benefit is faster handoff and retrospective
review with less risk of stale-summary misuse. This is an operational clarity
feature, not a clinical decision feature.

## source evidence quality

Source quality is adequate for a narrow product-discovery decision and weak for
any broader safety or regulatory claim.

- Philips IntelliVue Information Center IFU (Release N, 2011) shows that
  recording annotations and alarm-review recordings include patient
  identifiers, bed label, and date/time. This is good workflow precedent that
  provenance fields are common in review artifacts, but it is vendor
  documentation rather than independent clinical evidence.
- Mindray BeneVision CMS Viewer product material available as of 2026-05-06
  describes historical review, patient summary views, and local report
  printing. This supports the hypothesis that timestamped review artifacts are
  useful, but it is marketing/product-sheet evidence rather than validated
  safety data.
- Repo evidence is stronger than competitor evidence for scope control:
  `src/patient.c` and `src/main.c` already produce bounded summary text, so the
  MVP can stay limited to one added metadata line rather than becoming a new
  reporting subsystem.

Taken together, the evidence is sufficient to approve a narrow provenance-only
design and insufficient to justify storage, audit, or clinical claims.

## medical-safety impact

Direct clinical-logic impact is none expected if implementation stays within
scope. The indirect safety effect is favorable: operators can better judge
whether a summary is stale during handoff or retrospective review. The main
medical-safety hazard is semantic confusion if the added time is interpreted as
the measurement time of the displayed vitals rather than the time the summary
text was generated.

## security and privacy impact

No new patient-data category, network path, credential flow, or access-control
surface should be introduced. The summary already contains patient-identifying
and clinical data; adding a timestamp only increases provenance detail.
Security and privacy risk stays low if the change remains stdout/UI-summary
only and does not add persistence, background export, host identifiers,
usernames, or hidden telemetry.

## affected requirements or "none"

Closest affected approved requirements:

- `SYS-011` Patient Status Summary Display
- `SWR-PAT-006` Patient Summary Display

No change is justified to alerting, threshold, NEWS2, authentication, or
data-integrity requirements. Design may need a narrow clarification that the
summary can include non-clinical provenance metadata without implying new
clinical behavior.

## intended use, user population, operating environment, and foreseeable misuse

- Intended use: tell trained users when a summary artifact was generated during
  a patient-monitoring session.
- User population: trained clinical staff, testers, reviewers, and operators
  using the Windows workstation or CLI demo output.
- Operating environment: local workstation session, copied console output,
  printed review artifact, or screenshot-based handoff.
- Foreseeable misuse:
  - treating the summary timestamp as the time the latest vitals were measured
  - treating the summary timestamp as proof of current live state
  - treating the field as a durable audit-log record or EMR-ready provenance
    trail
  - misreading locale-specific date formats or timezone assumptions

## severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

- Severity: low to moderate indirect harm. By itself the field does not change
  patient-state classification, but misread time context could contribute to
  delayed or incorrect review decisions.
- Probability without controls: low to medium because copied summaries and
  printed artifacts are plausible, and a vague label such as `Timestamp` would
  be easy to misread.
- Initial risk: low/medium.
- Required risk controls:
  - label the field explicitly as summary-generation time, not measurement
    time; prefer wording such as `Summary generated at`
  - use one shared formatter for both summary surfaces so meaning and format
    stay identical
  - use an unambiguous 24-hour format such as `YYYY-MM-DD HH:MM:SS` and state
    local time or UTC explicitly
  - keep the value session-scoped and render-time only; do not add persistence
    or audit-log semantics
  - keep all changes outside `vitals.c`, `alerts.c`, `news2.c`, alarm-limit
    logic, and patient-reading storage behavior
  - use bounded formatting buffers and existing static-memory constraints
- Verification method:
  - extend summary-output tests to confirm the timestamp line exists, is
    explicitly labeled as summary-generation time, and does not alter
    vitals/status text
  - add targeted CLI/console validation that both summary paths use the same
    label and format
  - inspect the changed file list to confirm summary-only scope
- Residual risk after controls: low.
- Residual-risk acceptability rationale: once the field is clearly labeled and
  kept metadata-only, the provenance benefit outweighs the minimal remaining
  confusion risk. No new clinical claims or automated decisions are introduced.

## hazards and failure modes

- The added time is labeled too loosely and a clinician assumes it is the
  physiologic capture time of the vitals.
- The field uses ambiguous locale formatting, causing human misread during
  handoff.
- The timestamp updates dynamically or is collected from more than one call
  site, producing inconsistent values within the same summary artifact.
- Implementation expands into storage, export history, or audit logging
  without fresh requirements and risk review.
- Code changes unintentionally touch alert, NEWS2, or reading-buffer logic
  because the summary path is not kept isolated.
- Product messaging overstates the feature as compliance-grade traceability
  rather than simple review-artifact provenance.

## existing controls

- The issue body already constrains scope to metadata only and explicitly
  excludes thresholds, alerts, NEWS2, long-term storage, EMR integration, and
  alarm-event logging.
- Current summary behavior is centralized in `patient_print_summary()` with
  CLI demo usage in `src/main.c`, which allows a contained change.
- Approved requirements already separate summary display from clinical
  classification and alert-generation logic.
- Existing patient-summary tests provide a natural place to verify output
  changes without touching clinical algorithms.
- Repository-wide static-memory constraints discourage broad formatting or
  persistence changes.

## required design controls

- Prefer the user-facing label `Summary generated at` or similarly explicit
  wording over bare `Capture time` or `Timestamp`.
- If the product owner insists on the phrase `capture time`, qualify it in the
  text so it is unmistakably the capture time of the summary artifact, not of
  the measurements.
- Use a single shared helper to format the timestamp for both
  `patient_print_summary()` and the CLI summary output.
- Freeze the value once per summary render so one artifact cannot contain
  drifting times.
- Keep the MVP to one added line in existing summary text. Do not add report
  headers, bed-label fields, alarm-review annotations, or other vendor-style
  report structure.
- If reliable clock acquisition fails, surface an explicit unavailable state or
  stop the change; do not fabricate a time.
- Document in design and tests that the feature is non-clinical provenance
  metadata only.

## MVP boundary that avoids copying proprietary UX

The safe MVP is one neutral provenance line added to the existing text summary.
It should not copy Philips or Mindray report layout, alarm-strip annotation
structure, patient window chrome, bed-label conventions, or broader historical
review workflows. The goal is to solve freshness ambiguity inside the repo's
existing summary surface, not to emulate a vendor report product.

## clinical-safety boundary and claims that must not be made

- Do not claim the timestamp proves the vitals are current, bedside-verified,
  or suitable for treatment decisions without live confirmation.
- Do not claim the timestamp is the measurement time of each displayed vital
  sign unless the implementation truly changes data modeling and receives fresh
  review.
- Do not claim audit-log, medico-legal, EMR, or regulatory traceability value
  from this MVP.
- Do not market the feature as improving diagnostic accuracy, alarm
  performance, or clinical outcome prediction.
- Bedside observation and normal clinical review remain the authoritative
  context for intervention decisions.

## validation expectations

- Unit tests should cover both presence and wording of the timestamp line in
  summary output.
- Tests or deterministic stubs should confirm format stability and prevent
  ambiguous or locale-dependent rendering.
- Regression checks should confirm no change to alert text, alert count,
  overall status, or reading-storage behavior.
- Manual console review should confirm the line is obvious in copied/pasted
  summaries and is not easily confused with measurement time.
- Diff inspection should show summary-surface files only, with no requirement
  to alter thresholds, alarms, auth, storage, or export subsystems.

## residual risk for this pilot

Low. The remaining risk is limited to human misunderstanding of a small
metadata field, not to changed monitoring logic. With explicit wording, a
stable format, and no persistence or clinical-logic expansion, the candidate is
acceptable for pilot design work.

## human owner decision needed, if any

A non-blocking product/architect decision is still needed on final wording and
time base:

- preferred label (`Summary generated at` is safer than bare `Capture time`)
- whether the field is rendered in local workstation time or UTC
- whether `SYS-011` and `SWR-PAT-006` are clarified directly or supplemented
  by a narrow provenance-display requirement

## whether the candidate is safe to send to Architect

Yes. The candidate is safe to send to Architect if the design preserves the
metadata-only boundary above and treats the timestamp as summary provenance
rather than clinical data.
