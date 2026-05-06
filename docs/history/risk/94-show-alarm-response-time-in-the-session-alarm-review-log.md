# Risk Note: Issue #94 - Show Alarm Response Time In The Session Alarm Review Log

Date: 2026-05-06
Issue: #94 "Feature: show alarm response time in the session alarm review log"
Branch: `feature/94-show-alarm-response-time-in-the-session-alarm-review-log`

## Proposed change

Add a read-only per-row timing field to the existing session alarm review log so
reviewers can tell how long a recorded session alarm event remained active.

Current repo observations that shape the risk:

- `include/patient.h` `AlertEvent` currently stores `reading_index`, aggregate
  `AlertLevel`, `abnormal_mask`, and `summary`, but no explicit start time, end
  time, or duration field.
- `src/patient.c` appends session alarm events only when the aggregate severity
  or abnormal-parameter set changes, so each row represents a session-scoped
  alert-state interval rather than an acknowledged clinician action.
- Issue #93 already proposes trigger-time metadata for the same review surface,
  which is a likely prerequisite or shared design input for safe duration
  rendering here.

This candidate is acceptable only if the added field is defined as deterministic
session timing metadata for the stored event row. It must not imply clinician
acknowledgement, escalation performance, or staff response compliance.

## Product hypothesis and intended user benefit

Hypothesis: adding a visible duration to each stored session alarm row will make
retrospective review easier because reviewers can distinguish brief transient
events from sustained instability without reconstructing duration from reading
order alone.

Expected user benefit:

- clearer handoff and tester review of how long a warning or critical state
  persisted
- less manual inference from raw reading rows
- better alignment between the session alarm log and operational questions about
  persistence versus one-sample noise

## Source evidence quality

Evidence quality is adequate for product-discovery scope and weak for any claim
about clinical effectiveness or operator-response improvement.

- Drager Alarm History Analytics publicly describes comparing alarm response
  times across hospital units and severities. This is good evidence that timing
  context is a real monitoring workflow, but it is enterprise alarm-management
  marketing and not a bedside session-log design reference.
  Source: https://www.draeger.com/en-us_us/Products/Alarm-History-Analytics
- Mindray BeneVision CMS public manual material states that alarm time is shown
  for latched physiological alarms. This supports the idea that alarm timing is
  a common review datum, but it does not justify copying UX or using
  "response time" to imply human acknowledgement.
  Source: https://www.mindray.com/content/dam/xpace/en_us/service-and-support/training-and-education/resource--library/technical--documents/operators-manuals-1/H-046-010879-00-BeneVision-CMS-Operators-Manual-R3-14.0.pdf

Conclusion: the sources are sufficient to justify a narrow, non-copying MVP for
session-local alarm-duration metadata. They are not sufficient to support claims
about staff performance, alarm-management effectiveness, or clinician response
latency.

## Medical-safety impact

This change does not alter thresholds, NEWS2 logic, alarm limits, or active
alert generation. Risk is entirely in interpretation and presentation.

Primary safety benefit:

- reviewers can see whether a stored warning or critical session event was brief
  or sustained, which may improve retrospective understanding of recent
  deterioration

Primary safety risks:

- the label "response time" may be interpreted as clinician response rather than
  alarm-active duration
- duration may be computed incorrectly if the design assumes timestamps that are
  not actually stored or conflates reading order with elapsed time
- unresolved events may be shown with a misleading completed duration instead of
  an explicit active/incomplete state
- multi-parameter event rows may overstate precision if the UI implies
  parameter-specific timing when the record is really an aggregate event-state
  interval

Overall medical-safety impact: low-to-moderate if kept as a clearly labeled
read-only review aid, but not acceptable if the wording or math suggests it is
measuring clinician response or a precise enterprise audit metric.

## Security and privacy impact

Security and privacy impact is limited but real because timing metadata remains
patient-linked session review data.

- No new network, cloud, or export path should be introduced in this issue.
- The timing field must stay behind the current authenticated local-session
  boundary.
- Reset, logout, patient change, and bounded-session rollover behavior must
  clear or disclose prior timing context the same way the session review log
  itself is bounded.
- The feature must not create a durable medical-record claim or a workforce
  performance record.

## Affected requirements or "none"

Existing requirements likely affected:

- `UNS-017` Session Alarm Event Review
- `SYS-020` Session Alarm Event Review Storage
- `SYS-021` Session Alarm Event Review Presentation
- `SWR-PAT-007` Session Alarm Event Capture
- `SWR-PAT-008` Session Alarm Event Access and Reset
- `SWR-GUI-013` Session Alarm Event Review List

New derived requirements are needed for:

- timing-source semantics for each stored event row
- duration calculation rules, including the final unresolved row
- explicit rendering for unresolved or unavailable duration values
- reset/rollover behavior for any stored timing metadata
- wording that distinguishes event-active duration from clinician response time

## Intended use, user population, operating environment, and foreseeable misuse

Intended use:

- trained clinicians and internal reviewers inspect session-scoped historical
  alarm-event rows and their duration metadata during local review

User population:

- bedside clinical staff, testers, and reviewers using the Windows workstation
  pilot

Operating environment:

- local single-patient Win32 workflow with static in-process session storage and
  either simulation or manually entered readings

Foreseeable misuse:

- interpreting "response time" as staff response or alarm acknowledgement time
- treating session-row duration as a complete legal longitudinal record
- assuming `--` or missing duration means no risk rather than unresolved or
  unavailable timing data
- assuming each aggregate event row represents one individual alarm source when
  it may represent a combined abnormal-state signature

## MVP boundary that avoids copying proprietary UX

The MVP should remain narrow:

- local session-only timing metadata for the existing alarm-event rows
- simple text duration or elapsed-session display only
- no central-station dashboards, staffing analytics, acknowledge counts, or
  enterprise alarm-management widgets
- no export, no cloud sync, no EMR integration, and no alarm workflow actions

## Clinical-safety boundary and claims that must not be made

The feature may support retrospective review. It must not claim to:

- measure clinician response, acknowledgement, or SOP compliance
- replace the active alert list or live monitor view
- provide a complete medical record outside the current bounded session
- prove that an alarm was clinically insignificant because its duration appears
  short
- provide parameter-level timing precision if the underlying stored row is only
  an aggregate event-state transition

## Whether the candidate is safe to send to Architect

Yes, with constraints. The candidate is safe to send to Architect because it is
read-only and does not change clinical algorithms, but design must first define
the timing source and row semantics precisely. Architect should treat terminology
control as part of safety, not as a cosmetic choice.

## Severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

| Item | Assessment |
| --- | --- |
| Severity | Moderate: a misleading duration or mislabeled "response time" could distort handoff or review understanding of recent instability and could falsely imply staff-response performance. |
| Probability | Possible without controls because the current model lacks explicit timing fields and the wording is easy to misread. |
| Initial risk | Medium |
| Key risk controls | Define deterministic row-duration semantics; prefer neutral terminology such as `Alarm Duration` or `Time Active`; show unresolved rows explicitly; reuse session boundaries/reset disclosure; keep live alerts primary. |
| Verification method | Unit tests for timing math and unresolved rows, integration checks against session event ordering/reset behavior, and GUI review that the label and placeholder text do not imply clinician response or active status. |
| Residual risk | Low if timing semantics, wording, and reset behavior are explicit and verified. |
| Residual-risk acceptability rationale | Acceptable for this pilot because the feature remains a bounded review aid and does not modify live alarm generation or treatment guidance. |

## Hazards and failure modes

| Hazard | Failure mode | Potential harm |
| --- | --- | --- |
| Human-response confusion | UI says "response time" and users read it as clinician acknowledgement latency | Incorrect assumptions about alarm handling performance or clinical compliance |
| Wrong duration math | Start/end rules are undefined or inferred from reading order instead of explicit timing semantics | Reviewers overestimate or underestimate how long instability persisted |
| False precision for aggregate events | A row with multiple abnormal parameters displays one duration as if it were per-parameter timing | Misinterpretation of which alarm condition actually persisted |
| Stale timing after reset | Duration data survives patient clear, logout, or rollover | Wrong-patient or wrong-session review narrative |
| Silent incomplete final row | Active or unresolved event shows as completed duration instead of `Active`, `Open`, or `--` | Reviewer believes an alarm ended when it did not |

## Existing controls

- The current alarm-event log is already bounded by static session storage.
- Existing alert semantics are centralized in `check_*()`,
  `overall_alert_level()`, and `generate_alerts()`.
- Existing session-reset behavior and disclosure already define retention
  boundaries for session review data.
- Issue #93 has already identified trigger-time metadata as a related need for
  the same surface.

These controls reduce scope risk but do not yet define safe timing semantics for
this candidate.

## Required design controls

- Define each displayed value as the duration of the stored session event row,
  not clinician response time.
- Prefer a neutral label such as `Alarm Duration` or `Time Active` unless a
  human owner explicitly approves and defines `response time`.
- Do not implement duration math until the timing source is explicit:
  trigger time plus next-change time, or an equivalent session-elapsed marker.
- Render unresolved/current rows as `Active`, `Open`, or `--` rather than
  fabricating an end time.
- Preserve current session-reset and retention disclosure behavior for any new
  timing metadata.
- Keep the feature read-only; no acknowledgement, dismissal, or escalation UI.
- If later summary/export surfaces reuse this field, they must use the same
  underlying timing rules.

## Validation expectations

- unit tests for duration calculation across warning-to-critical escalation,
  recovery to normal, and abnormal-set changes
- tests for final unresolved rows and placeholder rendering
- tests confirming reset/logout/new-patient/rollover behavior clears or
  discloses prior timing context
- GUI review showing the wording does not imply clinician response or active
  live status
- traceability updates for the new timing semantics and unresolved-state rules

## Residual risk for this pilot

Residual risk remains that a reviewer may over-trust a short duration or infer
workflow performance from it. For this pilot, that risk is acceptable only if
the duration is clearly framed as session-local alarm-state timing and not as a
human response metric.

## Human owner decision needed, if any

Product owner / architect should explicitly decide:

- whether the user-facing label is `Alarm Duration`, `Time Active`, or another
  neutral term instead of `response time`
- whether this issue depends on landing issue #93 timing metadata first
- whether duration is based on wall-clock timestamps or session-elapsed time
- how unresolved final rows should be displayed in the MVP
