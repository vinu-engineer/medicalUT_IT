# Risk Note: Issue 42

Issue: `#42`  
Branch: `feature/42-historical-reading-drill-down-review`

## proposed change

Add a review-only historical drill-down for the current session so selecting an
entry from `IDC_LIST_HISTORY` repopulates the dashboard from that stored
reading's snapshot instead of always showing only the latest reading. The
intended scope is limited to review surfaces already present in the product:
vital tiles, overall status, derived alerts, and an explicit return-to-live
control. No new thresholds, diagnosis logic, storage back end, or hardware
behavior should be added.

## product hypothesis and intended user benefit

The product hypothesis is credible: clinicians, testers, and handoff reviewers
benefit from seeing what the monitor looked like at a prior abnormal or
interesting moment, not only the most recent reading. In this repository, the
data already exists in the bounded session history, but the UI currently uses
`patient_latest_reading()` for the dashboard summary and `generate_alerts()`
from the latest snapshot only. A small drill-down can improve retrospective
review without changing the underlying clinical rules.

## source evidence quality

Source evidence is sufficient for product-discovery justification but not for
clinical-effectiveness or safety validation:

- The issue-supplied Philips Horizon Trends page and GE CARESCAPE Central
  Station page support the claim that historical and current monitoring views
  are common monitoring workflows.
- The issue-supplied Mindray operator-manual link is appropriate as operator
  workflow context, but it is still vendor documentation rather than independent
  safety evidence.
- Overall evidence quality is moderate for "users expect historical review" and
  low for "this exact implementation is safe." Design safety must therefore be
  justified from repo behavior and explicit controls, not from competitor UX.

Sources reviewed on 2026-05-05:

- https://www.usa.philips.com/healthcare/product/HCNOCTN172/horizontrendstrendmeasurementdisplay
- https://www.gehealthcare.com/products/patient-monitoring/patient-monitors/carescape-central-station
- https://www.mindray.com/content/dam/xpace/en_us/service-and-support/training-and-education/resource--library/technical--documents/operators-manuals-1/legacy/H-046-006734-00-T1-ops-manual-FDA-14.0.pdf

## medical-safety impact

This is not a new clinical algorithm, but it is not zero-risk. The main hazard
is context confusion: a user may mistake a selected historical abnormal reading
for the current live patient state. That risk is real in the current codebase
because the dashboard presently derives tiles, alerts, and status from the
latest reading path, while the simulation timer continues to append new
readings and resets the session when the bounded history is full. If historical
review is made unmistakable and kept review-only, the residual medical-safety
impact is low enough for this pilot.

## security and privacy impact

No new external data flow is needed. The intended feature can remain fully
local and session-scoped. The main privacy and integrity risks come from scope
creep:

- turning a bounded session review into a longer-lived patient record;
- adding export, print, or remote-sharing behavior;
- persisting retrospective review artifacts outside the current session.

If the change stays in-memory and session-only, privacy impact remains low.

## affected requirements or "none"

Existing approved behavior that is likely affected or extended:

- `UNS-009` vital sign history
- `UNS-010` consolidated status summary
- `SYS-009` vital sign reading history
- `SYS-011` patient status summary display
- `SWR-PAT-003` latest reading access
- `SWR-PAT-006` patient summary display
- `SWR-GUI-003` colour-coded vital signs display
- `SWR-GUI-004` patient data entry via GUI
- `SWR-TRD-001` session trend/sparkline display

New or revised requirements will likely be needed for:

- explicit historical-review mode versus live mode;
- synchronized snapshot derivation across tiles, alerts, and status surfaces;
- behavior when live acquisition continues or the session reaches the
  `MAX_READINGS` reset boundary during review.

AI/ML impact: none. This candidate does not add, change, remove, or depend on
an AI/ML model.

## intended use, user population, operating environment, and foreseeable misuse

Intended use: trained clinical staff or internal testers review a prior reading
within the current monitored session on the local Windows workstation to better
understand deterioration, recovery, or handoff context.

User population: bedside clinicians, ward staff, intensivists, and internal
demo/test operators already within the product's intended user group.

Operating environment: one local workstation, one active patient session, one
bounded in-memory history (`MAX_READINGS = 10`), simulation/HAL-driven feed,
no enterprise historical archive.

Foreseeable misuse:

- assuming the selected historical reading is the current live state;
- reading historical alerts as currently active alarms;
- continuing review while new live readings arrive and silently change context;
- treating the feature as a longitudinal chart or event-audit system;
- using retrospective values to imply diagnosis, treatment advice, or alarm
  acknowledgement beyond the feature's review-only scope.

## clinical-safety boundary and claims that must not be made

This feature must stay inside a narrow review boundary:

- It is a review-only reconstruction of one stored session reading.
- It must not claim to show the patient's current state unless the UI is in the
  explicit live/latest mode.
- It must not be presented as a longitudinal medical record, central-station
  review console, alarm-management workflow, or clinical decision support
  feature.
- It must not add or imply new thresholds, scoring logic, diagnosis, triage,
  treatment recommendation, or retrospective alarm adjudication.

## severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

- Severity: Moderate. Misreading a historical abnormal snapshot as the current
  patient state could distort handoff, review, or demo decision-making even
  though the feature does not automate care.
- Probability: Occasional without controls, because the current UI is
  latest-reading oriented and the live timer continues every 2 seconds.
- Initial risk: Medium.
- Risk controls: require an unmistakable historical-review mode, keep all
  derived surfaces synchronized to the same selected snapshot, and prevent
  silent context drift when live acquisition continues or the session resets.
- Verification method: targeted GUI verification plus supporting automated
  tests for snapshot-selection state if selection logic is factored out of the
  Win32 message handlers.
- Residual risk: Low after controls.
- Residual-risk acceptability rationale: acceptable for this pilot if the
  design keeps the feature session-scoped, review-only, and visually incapable
  of being confused with live mode.

## hazards and failure modes

- A historical reading is displayed with the same visual treatment as live mode
  and is mistaken for the current state.
- The selected history row drives some surfaces while the latest reading still
  drives others, creating a mixed-context dashboard.
- New timer-driven readings arrive during review and either silently replace the
  reviewed state or invalidate the selected index.
- The `MAX_READINGS` rollover resets the session while a historical item is
  selected, causing stale or remapped review context.
- Scope expands into export, printing, or multi-session persistence and turns a
  review affordance into an uncontrolled record feature.

## existing controls

- The issue already limits scope to a review-only workflow and excludes
  thresholds, diagnosis logic, and multi-patient workflows.
- The repository already stores only a bounded, in-memory session history with
  `MAX_READINGS = 10`.
- Existing summary logic is deterministic and centralized around stored
  `VitalSigns` snapshots, `overall_alert_level()`, and `generate_alerts()`.
- The current product already has live-mode cues such as `* SIM LIVE` /
  `SIM PAUSED`, which can be extended instead of inventing a second ambiguous
  dashboard state.

## required design controls

- Entering historical review shall add a persistent, explicit mode indicator
  such as `Reviewing reading #N of M`, visible in the same glance path as the
  current live-state cues.
- All derived surfaces shall use the same selected snapshot: vital tiles,
  overall status banner, alert list, and any score or summary fields shown in
  the same view.
- Historical review shall not silently coexist with timer-driven state drift.
  For this pilot, the safest control is to auto-pause simulation while a
  historical reading is selected, or otherwise freeze the reviewed snapshot and
  block session rollover until the user returns to live mode.
- The feature shall provide a one-action return to latest/live mode.
- Historical alerts shall be labeled as alerts/status for the selected reading,
  not as current active alarms.
- The feature shall not add persistence, export, printing, or multi-session
  retrieval; it must remain one-patient, one-session, bounded-memory review.
- If the user clears the session, admits a new patient, or the reviewed record
  becomes invalid, review mode shall exit explicitly rather than mapping the
  selection to a different reading.

## MVP boundary that avoids copying proprietary UX

Keep the MVP narrow and repo-native:

- reuse the existing history list and current dashboard surfaces;
- avoid copying vendor split-screen layouts, waveform cursors, event timelines,
  multi-patient central-station concepts, strip printing, or long-window trend
  databases;
- keep navigation to simple list selection plus a clear return-to-live action,
  with optional bounded previous/next abnormal navigation only if it remains
  session-scoped and deterministic.

## validation expectations

- Verify that selecting each stored history row updates every reviewed surface
  from that exact stored reading and not from `patient_latest_reading()`.
- Verify that historical-review mode is always visually explicit and that the
  return-to-live action restores latest-reading behavior immediately.
- Verify that live acquisition cannot silently change the reviewed snapshot or
  remap the selected row during review.
- Verify behavior at session edges: first reading, last reading, empty history,
  full buffer, session clear, patient admit/refresh, and simulation pause/resume.
- Verify that no alert thresholds, NEWS2 logic, storage limits, or persistence
  behavior change as part of this feature.
- Run the issue's regression commands once implementation exists:
  `run_tests.bat`, `ctest --test-dir build --output-on-failure -R "Patient|Trend|Alert"`,
  and `python dvt/run_dvt.py`.

## residual risk for this pilot

Low if the required controls are implemented. The residual risk is mainly human
misinterpretation of retrospective information, not algorithmic or privacy
failure. That residual is acceptable for the pilot because the feature can stay
bounded to local, deterministic, session-only review.

## whether the candidate is safe to send to Architect

Yes, with controls. The feature is safe to send to Architect if the design
keeps it review-only, session-scoped, and explicit about historical versus
live context. It should not move forward as an unlabelled "show old data in the
main dashboard" change.

## human owner decision needed, if any

Human/product-owner confirmation is still needed on one interaction choice:

- whether entering historical review must auto-pause live simulation for the
  pilot, or whether background acquisition may continue with an explicit
  separate indicator that new live data is pending.

My recommendation is to require auto-pause during historical review in the
pilot, because the current 10-reading reset behavior makes background live
updates harder to keep unambiguous and safe.
