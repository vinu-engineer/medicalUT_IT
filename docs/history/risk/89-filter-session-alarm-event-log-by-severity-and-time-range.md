# Risk Note: Issue #89 - Filter Session Alarm Event Log by Severity and Time Range

Date: 2026-05-06
Issue: #89 "Feature: filter the session alarm event log by severity and time range"
Branch: `feature/89-filter-session-alarm-event-log-by-severity-and-time-range`

## Proposed change

Add read-only filter controls to the existing session alarm event review surface
so reviewers can narrow the visible historical event list by severity and by a
bounded session-time window without changing alert generation, alarm thresholds,
NEWS2 logic, or current active-alert behavior.

Repo observations supporting the change:

- `src/gui_main.c` already renders `IDC_LIST_EVENTS` separately from
  `IDC_LIST_ALERTS`, so the product already distinguishes historical review from
  current active alerts.
- `src/patient.c` stores session-scoped `AlertEvent` entries derived from the
  validated alert engine and already discloses automatic session resets.
- The existing event model stores reading index, aggregate severity, abnormal
  signature, and summary text, but it does not store a timestamp. Any
  "time-range" concept therefore needs an explicit, non-misleading definition
  before implementation.

## Product hypothesis and intended user benefit

Hypothesis: reviewers can complete handoff review, retrospective assessment, and
DVT inspection faster if they can narrow the session event list to warning or
critical episodes and to a recent bounded review window instead of scanning the
entire retained session.

Expected user benefit:

- quicker isolation of high-severity events during review
- less visual noise in sessions with multiple warning transitions
- faster confirmation that a recent critical episode occurred and then resolved

## Source evidence quality

Evidence quality is adequate for product-discovery scope and weak for clinical
or workflow-effectiveness claims.

- Philips Event Review Pro IFU documents viewer filtering and date-range
  filtering for retrospective records. This is relevant evidence that filtered
  review workflows are common, but the IFU is from March 2004 and reflects a
  different product/problem space.
  Source: https://www.documents.philips.com/doclib/enc/fetch/577817/577818/Event_Review_Pro_3.0_Instructions_for_Use_%28IFU%29_861276_%28ENG%29.pdf
- Draeger Alarm History Analytics markets alarm inspection by source, severity,
  quantity, duration, and chosen time frames. This supports the general product
  hypothesis that severity/time slicing is useful for alarm review, but it is
  marketing material for a broader networked analytics product.
  Source: https://www.draeger.com/en-us_us/Products/Alarm-History-Analytics

Conclusion: the external sources are sufficient to justify a narrow,
non-copying review filter MVP. They are not sufficient to justify new clinical
claims, real-time alarm-management claims, or enterprise analytics scope.

## Medical-safety impact

This candidate is presentation-only, but it still touches safety-relevant
review information because filtering can hide historical warning or critical
events from the user.

Primary safety benefit:

- makes it easier to focus on clinically relevant recent events during handoff
  or post-session review

Primary safety risks:

- a filter can hide significant recent events and make the retained session
  appear quieter than it is
- a "time-range" label can imply trustworthy chronological precision that the
  current data model does not yet provide
- sticky or uncleared filters can cause users to mistake a partial view for the
  full retained session history

Overall medical-safety impact: low-to-moderate if the feature remains a
read-only adjunct, defaults to showing all events, keeps current active alerts
unfiltered, and avoids misleading time semantics.

## Security and privacy impact

Security and privacy impact is limited because the feature reuses session-local
event data already present in the authenticated workstation session.

- No new patient-data class is introduced.
- No new network, export, or cloud path should be added in this MVP.
- Filter state should clear on logout, patient/session reset, and new-patient
  admission so a later user does not inherit a partial review state.
- Any future persistence of filter preferences should be treated separately from
  this issue because it changes review-state behavior across sessions.

## Affected requirements or "none"

There is no explicit baseline requirement yet for filtering the session alarm
event review log. The design is constrained by existing related requirements:

- `UNS-017` Session Alarm Event Review
- `SYS-020` Session Alarm Event Review Storage
- `SYS-021` Session Alarm Event Review Presentation
- `SWR-PAT-007` Session Alarm Event Capture
- `SWR-PAT-008` Session Alarm Event Access and Reset
- `SWR-GUI-013` Session Alarm Event Review List

New derived requirements are needed for:

- default filter state and visibility of the complete retained session
- exact severity-filter behavior
- exact "time-range" semantics
- filter reset behavior on logout, patient change, and automatic session reset
- explicit proof that active alerts remain unaffected by historical filters

## Intended use, user population, operating environment, and foreseeable misuse

Intended use:

- trained clinicians and internal reviewers narrow the visible historical
  session alarm-event list during local retrospective review

User population:

- bedside clinical staff, testers, and reviewers using the Windows workstation
  application

Operating environment:

- local single-patient pilot workflow with bounded in-process session storage
  and either simulator-fed or manually entered readings

Foreseeable misuse:

- assuming a filtered list is the complete retained session history
- assuming filtered-out critical events did not occur
- assuming a labeled time window represents precise wall-clock timing when the
  system only has reading-order data
- expecting the filter to affect active alerts, exports, or alarm behavior

## MVP boundary that avoids copying proprietary UX

The MVP should remain narrow and implementation-led:

- filter only the existing local `AlertEvent` review list
- keep the current event list structure and ordering
- prefer simple severity choices and a small bounded recent-window control
- do not copy central-station analytics dashboards, waveform panes, alarm
  heatmaps, or enterprise reporting layouts
- do not introduce export, cross-patient comparison, or remote analytics scope

## Clinical-safety boundary and claims that must not be made

The feature may support retrospective review. It must not claim to:

- change, suppress, acknowledge, or prioritize live alarms
- prove the absence of events outside the retained current session
- provide trustworthy elapsed clinical timing unless the data model adds and
  verifies that capability
- diagnose, recommend treatment, or replace the active-alert surface

## Whether the candidate is safe to send to Architect

Yes, with constraints. The candidate is safe to send to Architect because it is
read-only, session-scoped, and reuses already-derived event data. Architect
must treat "time range" as an open safety boundary: either add a trustworthy
session-time representation with explicit reset semantics, or narrow the MVP so
the control is clearly sequence-based rather than clock-like.

## Severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

| Item | Assessment |
| --- | --- |
| Severity | Moderate: a misleading filtered review can cause a clinician or reviewer to underappreciate recent deterioration during handoff or retrospective assessment, even though live alarm behavior is unchanged. |
| Probability | Possible without controls because partial-view state, off-by-one range boundaries, or misleading time labels are common UI failure modes. |
| Initial risk | Medium |
| Key risk controls | Default to showing all retained events; keep active alerts separate and unfiltered; define exact severity and time-window semantics; clear filter state on session changes; show when a filter is active; avoid wall-clock wording unless timestamps are truly captured and validated. |
| Verification method | Unit and GUI tests for severity inclusion/exclusion, boundary conditions on time-window filtering, filter reset behavior, and proof that event capture plus active-alert generation are unchanged by filter state; manual/DVT review of labels and reset disclosures. |
| Residual risk | Low |
| Residual-risk acceptability rationale | Acceptable for this pilot if the filter is plainly labeled as a review aid, not a clinical control, and if the user can reliably return to the full retained session with no ambiguity about whether a partial filter is active. |

## Hazards and failure modes

| Hazard | Failure mode | Potential harm |
| --- | --- | --- |
| Recent deterioration is hidden from review | Filter defaults to a partial subset or leaves a prior narrow filter active | Reviewer misses a recent warning or critical episode during handoff or test review |
| Time semantics are misleading | UI labels a range as minutes/hours without trustworthy underlying timestamps | User infers false chronology or duration from ordinal session data |
| Historical filter contaminates live review | Active-alert list or overall status is affected by historical filter state | User underreacts to the current patient condition |
| Session scope is misunderstood | Filtered view persists across logout, patient change, or session reset | Wrong-patient or incomplete-session review narrative |
| Boundary logic is wrong | Off-by-one or incorrect severity inclusion excludes expected events | Partial or incorrect retrospective assessment |

## Existing controls

- Existing alert-event generation is already centralized in `patient.c` and
  derived from validated alert logic.
- Existing GUI separates `Session Alarm Events` from `Active Alerts`.
- Existing session-reset disclosure already warns when earlier session data is
  no longer retained.
- Existing session data is bounded by static storage and local authenticated use.

These controls make a narrow filter feasible, but they do not yet make
time-range semantics safe by themselves.

## Required design controls

- The default state shall show all retained session events.
- Active-alert presentation and current overall status shall remain completely
  independent from any historical filter state.
- The UI shall show plainly when a filter is active and provide an easy clear
  path back to the full retained session.
- Severity filtering shall operate only on stored historical event severity,
  not on recalculated or alternate rules.
- If actual timestamps are not added, the design shall not label the control as
  precise elapsed time. Sequence-based wording such as recent events, recent
  readings, or session window is safer.
- If actual timestamps are added, they shall use a single trustworthy
  session-time source with explicit reset behavior and verification for manual
  and simulated entry paths.
- Filter state shall clear on logout, patient reinitialization, manual clear,
  and automatic session reset.
- Requirements and traceability shall explicitly cover no-effect-on-alert-engine
  behavior and filter-boundary verification.

## Validation expectations

- unit tests for severity-only filtering across mixed warning and critical event
  sequences
- tests for time-window boundary behavior, including first included event, last
  included event, and empty-result cases
- tests proving that event capture, event ordering, reset notices, and active
  alerts are unchanged when filters are applied or cleared
- GUI/manual review showing a user can tell when the list is filtered and can
  return to the full retained session without ambiguity
- DVT or equivalent evidence that a filtered historical list never changes the
  current active-alert list or aggregate status surface
- requirements and RTM updates for new filter behavior

## Residual risk for this pilot

Residual risk remains that users may over-trust a narrowed retrospective list,
especially if they overlook that the filter is active or assume time semantics
that the system does not actually support. For this pilot, that risk is
acceptable only if full-session visibility is the default and filter semantics
are explicit.

## Human owner decision needed, if any

Product owner and Architect should explicitly decide before implementation:

- whether "time range" means true elapsed session time or only a bounded
  sequence-based recent window
- if true elapsed time is required, whether the issue scope may expand to add a
  verified session-time field and associated requirements/tests
