# Risk Note: Issue #84 - Session Alarm Severity Summary Strip

Date: 2026-05-06
Issue: #84 "Feature: add a session alarm severity summary strip"
Branch: `feature/84-session-alarm-severity-summary-strip`

## Proposed change

Add a small read-only summary strip above the existing `Session Alarm Events`
panel that counts the current session's stored alarm events by resulting
severity: `NORMAL`, `WARNING`, and `CRITICAL`.

Repo observations supporting the change:

- `src/patient.c` already stores session alarm transition events as bounded
  `AlertEvent` records exposed through `patient_alert_event_count()` and
  `patient_alert_event_at()`.
- `src/gui_main.c` already renders those session events distinctly from current
  active alerts in `IDC_LIST_EVENTS`.
- The existing event model is transition-scoped. A single prolonged critical
  episode may still produce one `CRITICAL` event entry if the alert signature
  does not change between readings.
- Recovery to normal already becomes a stored event with aggregate level
  `NORMAL`, so a `NORMAL` count in this feature would represent recoveries, not
  all normal readings in the session.

The proposed MVP is acceptable only if the strip remains a deterministic
aggregation of the existing stored event records and does not become a second
clinical interpretation path.

## Product hypothesis and intended user benefit

Hypothesis: a compact severity-count strip will let an operator understand the
review log's overall shape faster, especially when the session has many event
rows.

Expected user benefit:

- faster recognition that a session contained mostly warning-level noise,
  repeated critical transitions, or multiple recoveries
- less scanning before reviewing the detailed event rows
- better handoff ergonomics without changing alert generation or thresholds

## Source evidence quality

Evidence quality is adequate for product-discovery scope and weak for any
clinical-effectiveness claim.

- Philips Clinical Surveillance marketing says browser-based views support deep
  review of waveforms, trends, events, history, and prioritized alert lists.
  This supports the general workflow need for compact retrospective review, but
  it is vendor marketing rather than clinical evidence.
  Source: https://www.usa.philips.com/healthcare/acute-care-informatics/clinical-surveillance
- The Philips IntelliVue Information Center IFU documents patient-data review
  and export workflows and warns that observations should be confirmed with
  bedside clinical observation before intervention. This is stronger evidence
  for safety boundary than for the exact summary-strip design.
  Source: https://www.documents.philips.com/doclib/enc/fetch/2000/4504/577242/577243/577247/582636/8573588/PM_Philips_IntelliVue_Information_Center_Rel_L_Instructions_For_Use.pdf
- GE CARESCAPE marketing describes customizable alarms and reports. This
  supports the general product hypothesis that compact review artifacts are
  common, but it does not justify copying any specific vendor layout or claim.
  Source: https://www.gehealthcare.com/static/wisdm/page_carescape-monitoring-platform.html

Conclusion: the sources are sufficient to justify a narrow, non-copying,
display-only MVP. They are not sufficient to support claims about alarm-fatigue
reduction, clinical outcome improvement, or enterprise monitoring parity.

## Medical-safety impact

This change does not alter thresholds, NEWS2 logic, alert generation, alarm
limits, or treatment guidance. The risk is interpretive rather than analytic.

Primary safety benefit:

- a reviewer can understand the stored session event mix faster before reading
  the detailed event lines

Primary safety risks:

- users may interpret the strip as a count of all abnormal readings or total
  abnormal dwell time, when it is only a count of stored transition events
- the `NORMAL` bucket may be misunderstood as overall session stability rather
  than count of recoveries-to-normal
- a mismatched aggregation helper could disagree with the event rows and
  understate recent deterioration
- a reset notice or session boundary could be missed if the count strip looks
  like whole-encounter history

Overall medical-safety impact: low-to-moderate if the strip stays clearly tied
to the existing event log and remains subordinate to the current active-alert
surface.

## Security and privacy impact

Security and privacy impact is low.

- No new data type is introduced.
- No new network, export, persistence, or sharing path should be introduced.
- The strip must remain behind the existing authenticated session boundary.
- Counts must reset with the same patient/session reset semantics already used
  by the stored event log.

## Affected requirements or "none"

No new clinical-behavior requirement is needed for the MVP. If formalized, this
is a presentation refinement adjacent to:

- `SYS-021` Session Alarm Event Review Presentation
- `SWR-GUI-013` Session Alarm Event Review List
- `SWR-PAT-008` only insofar as reset/disclosure semantics must remain visible

This issue should not expand `SYS-020` or the event-capture rules unless design
proves the existing event model is insufficient. That would be a separate,
higher-risk change.

## Intended use, user population, operating environment, and foreseeable misuse

Intended use:

- trained clinicians and testers review the distribution of already-recorded
  session alarm events more quickly

User population:

- bedside clinical staff, reviewers, and internal testers using the local
  Windows workstation application

Operating environment:

- the current single-patient local desktop workflow using static in-process
  storage and either simulator or manual-entry readings

Foreseeable misuse:

- assuming the strip counts every abnormal reading rather than event
  transitions
- assuming the `NORMAL` count means "the patient was normal most of the time"
- using the strip alone and ignoring the detailed event rows or current active
  alerts
- assuming the counts include events from before a visible session reset

## MVP boundary that avoids copying proprietary UX

The MVP should remain implementation-led and repo-native:

- one compact strip above the existing event list
- simple counts derived from the app's own `AlertEvent.level` values
- no vendor-specific timeline, waveform, or central-station review layout
- no export, sorting, filtering, acknowledgement, or multi-patient rollup
- no new retrospective logic beyond grouping existing stored events by severity

## Clinical-safety boundary and claims that must not be made

The feature may support retrospective review. It must not claim to:

- measure alarm burden by duration or by total abnormal readings
- replace current active alerts or live monitoring
- prove a patient was stable for most of the session
- provide a complete longitudinal record across session resets
- recommend escalation, de-escalation, or treatment

## Whether the candidate is safe to send to Architect

Yes, with constraints. The candidate is safe to send to Architect because it is
display-only, session-scoped, and derived from already-stored deterministic
event records. The design must preserve the event log as the source of truth
and must make the count semantics explicit.

## Severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

| Item | Assessment |
| --- | --- |
| Severity | Moderate: a misleading summary could cause a reviewer to underestimate recent deterioration or overestimate stability during handoff or retrospective review. |
| Probability | Possible without controls because users naturally read compact counts as stronger summaries than the detailed rows below them. |
| Initial risk | Medium |
| Key risk controls | Derive counts only from stored `AlertEvent` rows; label the strip as session event counts; define `NORMAL` explicitly as recovery events; keep current active alerts visually separate and primary; preserve reset disclosure. |
| Verification method | Unit coverage for any aggregation helper; GUI/manual checks that summary counts match rendered event rows after warning-critical-recovery sequences; reset-boundary checks; traceability/documentation updates if requirements text is refined. |
| Residual risk | Low |
| Residual-risk acceptability rationale | Acceptable for this pilot because the feature remains a read-only adjunct to an already-bounded review log and does not change clinical algorithms, alert generation, or intervention guidance. |

## Hazards and failure modes

| Hazard | Failure mode | Potential harm |
| --- | --- | --- |
| Recent deterioration is understated | Count strip groups events incorrectly or misses critical rows | Reviewer underestimates severity of the session before handoff or review |
| Stability is overstated | `NORMAL` count is read as "normal readings" instead of recoveries | User falsely assumes a benign session pattern |
| Session scope is misunderstood | Reset notice is present but the strip still appears to summarize the whole encounter | False confidence that earlier events are represented |
| Summary and detail disagree | Strip counts do not match the event rows shown below | User distrusts the review surface or follows the wrong summary |
| Historical view crowds out live risk | Operators focus on the strip and ignore current active alerts | Delay in responding to the current patient state |

## Existing controls

- Session alarm events already come from a bounded, deterministic,
  static-storage model in `patient.c`.
- The GUI already separates `Session Alarm Events` from `Active Alerts`.
- Recovery and reset semantics are already explicit in the stored event and
  reset-notice model.
- Current alert-generation logic is centralized in `overall_alert_level()` and
  `generate_alerts()`, reducing the need for new clinical logic.

These controls materially reduce risk, but they do not by themselves prevent
misinterpretation of the new summary strip.

## Required design controls

- Aggregate only from `patient_alert_event_*()` data already being rendered.
- Keep the strip session-scoped and read-only.
- Preserve the reset disclosure and ensure the strip does not hide it.
- Use wording that makes the semantics explicit, especially for `NORMAL`.
- Keep the detailed event list directly visible below the strip.
- Do not color or style the strip so aggressively that it overrides the current
  active-alert banner as the primary live-risk signal.
- If a helper is added, keep it deterministic and covered by unit tests.

## Validation expectations

- manual GUI check that the strip counts equal the visible event rows grouped by
  resulting severity
- sequence check covering warning to critical escalation and recovery to normal
- explicit check that `NORMAL` increments only when a recovery event row exists
- reset-boundary check confirming counts clear with the current session and do
  not silently include pre-reset events
- unit tests for any helper used to count `AlertEvent.level` values

## Residual risk for this pilot

Residual risk remains that some users will read a compact severity strip more
aggressively than intended. For the pilot, that risk is acceptable only if the
event list stays visible, the strip is defined as a count of session event
transitions, and the current active-alert surface remains primary.

## Human owner decision needed, if any

Architect/product owner should explicitly confirm one UI wording decision before
implementation:

- whether the bucket should literally display `NORMAL`, or whether the design
  should say `RECOVERED` / `RECOVERY TO NORMAL` to avoid implying a count of all
  normal readings

My recommendation is to preserve the requested three-bucket structure only if
the UI text makes the `NORMAL` semantics unmistakable.
