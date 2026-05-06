# Risk Note: Issue #54

Date: 2026-05-06
Issue: `#54` - Feature: add a minimal operator action audit trail

## proposed change

Add a narrow operator action audit trail for non-clinical user actions such as
login/logout, patient admit or refresh, manual add-reading actions, settings
changes, and export attempts or completions.

The MVP should stay limited to metadata-only event capture, bounded local
persistence, and a read-only in-app review surface. This issue should not add
clinical notes, waveform capture, remote sync, background telemetry, or any
change to thresholds, NEWS2 logic, alert generation, or treatment workflow.

## product hypothesis and intended user benefit

The product hypothesis is credible: reviewers, QA staff, and incident
investigators currently have no direct operator-action history, so they must
infer user behavior from patient-state changes alone.

The intended user benefit is faster reconstruction of what a user did during a
session, especially around account access, patient refresh, manual reading
entry, settings changes, and review or export actions.

## source evidence quality

Source evidence quality is moderate for product discovery and low for clinical
safety claims.

The cited Philips and GE materials are vendor marketing or vendor
documentation. They are acceptable as proof that audit or review surfaces are a
normal product pattern, but they are not independent clinical evidence and they
must not be copied as UI, workflow, or safety justification.

The stronger evidence in this repository is internal: the current application
already emphasizes traceability, review, account control, and export-adjacent
workflows, while lacking any dedicated operator-action audit mechanism.

## MVP boundary that avoids copying proprietary UX

The MVP should use a generic local event list or table with a small fixed event
taxonomy. It should not copy competitor report layouts, branded terminology,
configurable surveillance dashboards, or broad "system log" feature sets.

For this pilot, the acceptable boundary is:

- local only
- metadata only
- bounded retention
- read-only review surface
- no signatures
- no remote administration
- no forensic or legal-record claims

## medical-safety impact

Direct clinical risk is low if the feature remains strictly non-clinical and
does not sit on the live monitoring or alerting path.

Indirect safety relevance is still real. A trustworthy operator-action history
can improve post-incident reconstruction and reduce ambiguity during review.
Conversely, a misleading or incomplete audit trail could create false
confidence about what happened during a monitoring session.

This feature becomes materially higher risk if implementation expands into
clinical documentation, per-sample monitoring logs, or workflow gating that can
delay login, patient admission, manual reading entry, or settings recovery.

## security and privacy impact

Security and privacy impact is the main risk driver for this issue.

A persistent audit trail can accumulate usernames, patient context, and action
history. If the design logs excessive detail, it can create a new local store
of sensitive information that exceeds the current minimum necessary scope.

The design must not log passwords, password hashes, free text, full vital-sign
payloads, report contents, or other sensitive values that are not required to
explain the operator action itself.

## affected requirements or none

No approved baseline requirement currently covers operator-action audit
logging.

New requirements will likely be needed for:

- audit event capture scope and exclusions
- bounded retention and overwrite behavior
- local persistence format and integrity checks
- audit-view access control
- audit export behavior, if export remains in MVP

Adjacent existing workflows that the future design will touch include
authentication, role-based access control, patient admit or refresh, settings
changes, and any existing or planned review or export surfaces.

## intended use, user population, operating environment, and foreseeable misuse

Intended use: provide a concise local history of important operator actions for
review and troubleshooting within the desktop monitoring application.

User population: trained clinical staff and designated local administrators
already using the application's authenticated desktop workflow.

Operating environment: a Windows workstation running the current local
application, with no new network dependency introduced by this feature.

Foreseeable misuse:

- treating the audit trail as a complete clinical record
- logging patient names or raw clinical values when an identifier would suffice
- assuming the log is complete after ring-buffer wraparound
- exporting audit content outside the intended local review context
- relying on the audit trail for medico-legal immutability claims

## severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

Severity: low for direct patient harm if the feature remains off the clinical
decision path, but moderate for privacy, incident review, and operator trust if
the trail is incomplete, misleading, or over-collects sensitive data.

Probability: medium before controls because the feature touches authentication,
patient/session context, settings, persistence, and review UI in multiple parts
of the application.

Initial risk: medium.

Risk controls:

- metadata-only schema with explicit forbidden fields
- fixed-size static ring buffer and bounded persisted file format
- best-effort logging that never blocks monitoring or user-management actions
- sequence numbering plus timestamping to preserve ordering
- read-only review UI for MVP
- role-based access to view, clear, or export the trail

Verification method:

- unit tests for record creation, truncation, wraparound, ordering, and schema exclusion
- persistence tests for save/load integrity and corruption handling
- GUI or integration tests for login/logout, admit/refresh, add-reading, settings, and export action capture
- negative tests proving that passwords, free text, and raw clinical payloads are not stored
- manual review that monitoring actions still succeed when audit persistence fails

Residual risk: low if the design remains metadata-only, locally bounded, and
non-blocking.

Residual-risk acceptability rationale: acceptable for this pilot only if the
feature remains adjunctive, does not alter bedside decision support, and fails
safe by preserving core monitoring behavior even when audit capture or
persistence fails.

## hazards and failure modes

- audit capture stores excessive patient or account detail and creates avoidable privacy exposure
- audit writes fail and block login, logout, admit, add-reading, settings, or export actions
- event ordering is ambiguous after clock changes or partial writes
- ring-buffer overwrite removes important context without clear operator expectation
- review UI shows editable or misleading records
- access control allows the wrong role to view, clear, or export the trail
- failed actions are logged as successful, or success and failure are not distinguished
- the team expands scope into a generic system log or clinical documentation store

## existing controls

- The current product is local-first and does not require a network service for normal operation.
- The codebase already enforces static-memory design constraints that fit a bounded event buffer.
- Authentication and role handling already exist in `gui_users.c`, `gui_auth.c`, and `gui_main.c`.
- Local persistence patterns already exist for configuration and user data.
- The issue explicitly constrains scope away from thresholds, NEWS2, alert logic, and treatment guidance.

## required design controls

- Define an explicit event schema before implementation. Minimum fields should be sequence number, local timestamp, actor identity, role, event type, target type, and result code.
- Exclude passwords, password hashes, free text, raw vital values, report contents, and arbitrary file paths from all stored audit records.
- Use a fixed-size static ring buffer plus bounded on-disk format. No heap-backed unbounded growth.
- Make audit capture best effort. Failure to append or persist an event must never prevent login/logout, patient admit or refresh, add reading, settings use, or monitoring updates.
- Record both success and failure outcomes for privileged actions when feasible, without storing sensitive input data.
- Keep the MVP review surface read-only. Any clear, purge, or export capability must be separately authorized and explicitly traced.
- If patient context is required, prefer a minimal local patient or session identifier rather than patient name.
- Keep the feature local only for this pilot. No remote sync, background callbacks, or cloud reporting.
- Add new requirements and traceability entries before implementation starts so the feature does not bypass the existing IEC 62304-style document chain.

## validation expectations

- `build.bat`
- `run_tests.bat`
- targeted `ctest --test-dir build --output-on-failure -R Audit|UsersTest|ConfigTest` once audit tests exist
- manual verification that the review surface is read-only and role-appropriate
- negative verification that no clinical thresholds, NEWS2 calculations, or alert behavior changed
- failure-path verification that monitoring remains usable if the audit file is missing, full, or temporarily unwritable

## clinical-safety boundary and claims that must not be made

The feature may support review and troubleshooting, but it must not be
presented as diagnostic support, treatment support, or a substitute for the
patient record.

The team must not claim that the audit trail is:

- a complete reconstruction of all monitor behavior
- an immutable medico-legal chain of custody
- a replacement for formal clinical documentation
- evidence that a clinical action was appropriate or timely

## whether the candidate is safe to send to Architect

Yes, with constraints.

This candidate is safe to send to Architect if the design stays within a
metadata-only, local, bounded, non-blocking MVP and the design controls above
are turned into requirements before implementation.

It should be sent back for re-scoping before design if it expands into free
text, remote sync, detailed clinical-data logging, or any dependency that can
delay or block core monitoring actions.

## residual risk for this pilot

Residual pilot risk is low after the above controls because the feature is
adjacent to review and accountability rather than live clinical decision logic.

The dominant remaining risk is privacy and misuse of the resulting log, not
direct patient harm. That residual risk is acceptable for this pilot only if
the team preserves data minimization, bounded retention, and strict separation
from the clinical record.

## human owner decision needed, if any

- Decide which roles may view the audit trail in MVP and whether export, if any, is admin-only.
- Decide the retention size and overwrite policy that is acceptable for this pilot.
- Decide the minimum patient or session identifier allowed in an event record.
- Decide whether audit export is in scope now or should be deferred until the base trail is proven.
- Decide how the product should surface audit-persistence failure to operators without blocking clinical workflow.
