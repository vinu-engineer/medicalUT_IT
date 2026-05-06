# Risk Note: Issue #85 - Trend Direction Badges to Vital Tiles

Date: 2026-05-06
Issue: #85 "Feature: add trend direction badges to vital tiles"
Branch: `feature/85-trend-direction-badges-to-vital-tiles`

## Proposed change

Add a compact trend-direction badge beside each vital tile in the dashboard so
the operator can see whether the parameter is rising, stable, or falling
without inferring direction from the sparkline alone. The proposed MVP is
strictly display-layer only: no threshold changes, no alert-logic changes, no
NEWS2 changes, no new storage, and no new device-integration behavior.

Repo observations supporting the change:

- `src/gui_main.c` already renders sparkline strips from `trend_extract_hr()`,
  `trend_extract_sbp()`, `trend_extract_temp()`, `trend_extract_spo2()`, and
  `trend_extract_rr()` for the existing dashboard tiles, but it does not expose
  the `TrendDir` result as explicit badge text.
- `include/trend.h` and `src/trend.c` already expose a deterministic
  `trend_direction()` helper with a 5% hysteresis band and no heap allocation.
- `tests/unit/test_trend.cpp` already verifies rising, falling, stable, two-point,
  and extraction edge cases for the current trend helpers.
- The current blood-pressure sparkline is systolic-only
  (`trend_extract_sbp()`), while the visible tile label shows combined systolic
  and diastolic blood pressure. A single "blood pressure trend" badge would
  therefore overstate what the current helper actually measures unless the
  requirement is made explicit.

## Product hypothesis and intended user benefit

Hypothesis: an explicit direction cue will reduce scan time and lower the
chance that a user misreads the sparkline shape when quickly reviewing the
dashboard.

Expected user benefit:

- faster glanceable understanding of recent direction of change
- better alignment between the existing sparkline and the textual status cue
- lower cognitive load during dashboard review

This is a workflow and usability hypothesis only. It is not evidence that the
feature improves clinical outcomes.

## Source evidence quality

Evidence quality is adequate for product-discovery scoping and weak for
clinical-effectiveness claims.

- Philips Clinical Surveillance marketing describes browser-based views with
  access to waveforms, trends, events, history, and device settings. This is
  good evidence that trend-oriented monitoring workflows are market-real, but
  it is vendor marketing rather than outcome evidence.
  Source: https://www.usa.philips.com/healthcare/acute-care-informatics/clinical-surveillance
- The Philips IntelliVue Information Center brochure describes Trend Review,
  Event Review, and Alarm Review for retrospective physiologic review. This
  supports the relevance of trend review as a monitoring workflow, but it is a
  legacy brochure for a much broader central-station product.
  Source: https://www.documents.philips.com/doclib/enc/fetch/2000/4504/577242/577243/577247/582646/583147/IntelliVue_Information_Center_Brochure.pdf
- GE HealthCare Portrait Mobile marketing describes continuous trending and
  actionable alarms for ward monitoring. This supports workflow relevance, not
  the safety or efficacy of a specific badge UI in this pilot.
  Source: https://www.gehealthcare.com/en-us/products/patient-monitoring/portrait-mobile

Conclusion: the sources are sufficient to justify a narrow, non-copying MVP
that exposes existing local trend output. They are not sufficient to justify
claims of earlier detection, improved outcomes, predictive deterioration
detection, or any other clinical-decision-support claim tied to the badge
itself.

## Medical-safety impact

This change does not alter vital-sign classification thresholds, NEWS2 scoring,
alarm limits, or alert generation. The medical-safety risk comes from how the
user interprets a new display cue.

Primary safety benefits:

- direction of change is clearer than a sparkline alone during a rapid scan
- a user may notice movement within the same alert band without studying the
  graph

Primary safety risks:

- users may interpret "rising" or "falling" as "worsening" or "improving"
  rather than as plain numeric direction
- a badge may look more authoritative than the underlying helper supports,
  especially with short local history
- the blood-pressure tile may imply whole-blood-pressure direction even though
  current extraction is systolic-only
- the respiration-rate tile may mislead if the UI treats `0` ("not measured")
  as real trend input

Overall medical-safety impact is low-to-moderate if the badge remains a
secondary explanatory cue. It is not acceptable if the feature is presented as
predictive guidance or as a substitute for alert severity and human judgment.

## Security and privacy impact

Security and privacy impact is limited and unchanged in kind because the badge
reuses the same local patient data already shown in the authenticated session.

- No new network path should be introduced.
- No new persistence or export path should be introduced.
- The cue must remain inside the existing authenticated dashboard session.
- The feature must not create separate trend logs or patient-history artifacts
  outside the current local session.

## Affected requirements or "none"

Likely affected existing requirements:

- `UNS-009` Vital Sign History
- `UNS-014` Graphical User Interface
- `SYS-014` Graphical Vital Signs Dashboard
- `SWR-GUI-003` Colour-Coded Vital Signs Display
- `SWR-TRD-001` Trend Sparkline and Direction Detection

New or revised derived requirements are needed for:

- badge display semantics and hidden-state behavior
- the minimum history needed before showing a trend badge
- whether the blood-pressure badge is systolic-only or requires a different
  combined trend definition
- explicit wording that the cue communicates direction only and not clinical
  desirability or treatment priority

## Intended use, user population, operating environment, and foreseeable misuse

Intended use:

- trained clinicians and internal testers glance at recent direction of change
  for already-monitored parameters within the current local session

User population:

- bedside clinical staff, ward reviewers, and internal testers using the
  Windows dashboard

Operating environment:

- local Windows desktop GUI using the current bounded in-memory history
  (`MAX_READINGS`), with simulation mode today and future device-fed data later

Foreseeable misuse:

- assuming "rising" always means worse or "falling" always means better
- treating the badge as predictive or equivalent to an alarm
- assuming the blood-pressure badge reflects both systolic and diastolic motion
- relying on a badge when there is too little history to support a defensible
  direction
- showing a respiration trend when RR is unavailable and `0` is only a sentinel
  for "not measured"

## MVP boundary that avoids copying proprietary UX

The MVP should stay narrow and implementation-led:

- simple text-first or neutral-glyph direction cues derived from the app's own
  trend helper
- no competitor-specific horizon bars, deviation bars, central-station review
  layouts, or enterprise surveillance features
- no multi-patient surveillance, no alarm prioritization workflow, and no
  retrospective review console added by this issue
- scope limited to the existing vital-sign tiles; do not expand to NEWS2,
  aggregate banners, or alert workflows without separate requirements and risk
  review

## Clinical-safety boundary and claims that must not be made

The feature may support review ergonomics. It must not claim to:

- diagnose or predict patient deterioration
- indicate "better" or "worse" without parameter-specific clinical context
- replace alert severity, active alerts, or clinician interpretation
- provide a complete longitudinal record beyond the bounded local session
- recommend treatment or escalation action

## Whether the candidate is safe to send to Architect

Yes, with constraints. The candidate is safe to send to Architect if the
direction badge is implemented as a secondary explanatory cue derived directly
from the existing validated trend path, hidden when the data does not support a
defensible direction, and kept separate from alert-severity semantics.

## Severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

| Item | Assessment |
| --- | --- |
| Severity | Moderate: a misleading direction cue could create false reassurance or unnecessary concern during a quick scan, but it does not itself change thresholds, alarms, or treatment logic. |
| Probability | Possible without controls because the current helper semantics leave real ambiguity around blood pressure, short histories, and missing RR values. |
| Initial risk | Medium |
| Key risk controls | Derive the badge only from the same trend data already feeding the sparkline; define a minimum history threshold; hide the badge for unavailable RR data; keep badge wording and styling neutral; document blood-pressure semantics explicitly; reserve green/amber/red for alert severity only. |
| Verification method | Unit tests for badge mapping and hidden states, known-scenario tests for rising/falling/stable outputs, manual GUI review that badge wording is not confused with alert severity, and traceability updates for any new or changed requirements. |
| Residual risk | Low if controls are implemented and verified. |
| Residual-risk acceptability rationale | Acceptable for this pilot because the badge remains an adjunct display cue over existing validated trend output, while the existing alert and status surfaces remain the primary safety signal. |

## Hazards and failure modes

| Hazard | Failure mode | Potential harm |
| --- | --- | --- |
| Direction badge is read as clinical recommendation | Up/down/steady is interpreted as worse/better rather than numeric direction | Operator makes an overconfident or delayed judgment during scan review |
| Blood-pressure badge overstates what is measured | Tile implies combined BP trend while only systolic trend is computed | User over-trusts a simplified cue and misses diastolic context |
| Missing respiration data appears as a real trend | `0` sentinel values are treated as actual RR points | User misreads missing data as sudden fall or recovery |
| Badge appears with too little history | One or two points are shown as a meaningful trend without clear qualification | User over-interprets noise or insufficient data |
| Badge styling conflicts with alert semantics | Direction colors or icons compete with existing NORMAL/WARNING/CRITICAL cues | User confuses trend direction with alert severity |
| Badge and sparkline drift apart | Different extraction or mapping logic is used for the badge vs. the graph | Dashboard presents contradictory cues for the same parameter |

## Existing controls

- The current trend module is deterministic, bounded, and unit-tested.
- The current sparkline already exposes the same historical data path without
  changing clinical thresholds.
- Alert severity remains controlled by existing validated classification logic.
- The session history is already bounded by static storage and the current local
  patient session model.
- The dashboard already requires authentication before patient data is shown.

These controls reduce implementation risk, but they do not by themselves define
safe badge semantics.

## Required design controls

- Derive the badge exclusively from the same extracted sample series already
  used by the tile sparkline.
- Define a minimum reading-count rule before any direction badge is shown.
- Do not show a respiration trend badge when RR is unavailable (`0` / not
  measured).
- Reserve green/amber/red for alert severity; use neutral wording or styling
  for direction.
- For blood pressure, either document the cue as systolic-only or add a new,
  explicitly specified combined-BP trend method before release.
- Keep the badge read-only and display-only. No acknowledgment, escalation, or
  workflow behavior should be attached to it.
- Ensure the new badge does not crowd out the measured value, unit, or existing
  alert-status cue.
- Do not extend this MVP to NEWS2 or the aggregate status banner without a
  separate requirement and review pass.

## Validation expectations

- unit tests for direction-to-badge mapping and hidden-state behavior
- scenario tests for known rising, falling, and stable histories
- explicit validation of the chosen blood-pressure trend semantics
- verification that alerts, NEWS2, thresholds, and persistence behavior remain
  unchanged
- manual GUI review that the cue is clearly directional and not framed as
  better/worse or alarm severity
- traceability updates for any new or revised GUI/trend requirements

## Residual risk for this pilot

Residual risk remains that users may over-interpret a simple direction cue as a
clinical recommendation, especially for parameters where "up" or "down" is not
uniformly good or bad. For this pilot, that residual risk is acceptable only if
the cue is visibly secondary, neutral, and derived from the same bounded trend
path already used for the sparkline.

## Human owner decision needed, if any

Product owner and Architect should explicitly decide:

- whether the blood-pressure badge represents systolic-only direction or needs
  a new combined-BP definition
- the minimum reading count required before a direction badge is shown
- whether the cue is text-only, neutral icon-only, or both
- whether the respiration-rate tile hides the badge entirely when RR is not
  measured
