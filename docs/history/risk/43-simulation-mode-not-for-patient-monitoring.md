# Risk Note: Issue #43

Date: 2026-05-06  
Issue: `#43`  
Title: `Safety: make simulation mode unmistakably non-clinical`  
Decision: Safe to send to Architect if the change remains presentation-only and replaces the current `SWR-GUI-011` wording requirement.

## Proposed change

Replace the current subtle simulation indicators with an explicit non-clinical warning whenever `sim_enabled = 1`. The warning should appear on the primary simulation-facing surfaces called out in the issue: header, status banner, and patient or session review surfaces derived from synthetic data.

This is a human-factors containment change. It must not alter vital-sign generation, alert thresholds, NEWS2 scoring, alarm-limit behavior, patient-record semantics, or HAL behavior.

## Product hypothesis and intended user benefit

The issue hypothesis is credible: the current dashboard uses `* SIM LIVE`, `SIM PAUSED`, and a decorative rolling message, but the rest of the display still resembles an operational monitor. A screenshot, handoff, or quick glance could therefore be misread as live monitoring evidence.

The intended benefit is narrower than clinical performance. The change should reduce confusion during demos, DVT runs, UI work, and review of synthetic session artifacts, while preserving the current simulation workflow for development and training.

## Source evidence quality

Evidence quality is sufficient for product-discovery and safety-boundary work, but not as clinical validation evidence.

- Official manufacturer documentation supports unmistakable demo-mode labeling on monitor displays.
- A Philips Instructions for Use document on `documents.philips.com` describes Demonstration Mode as visibly marked as not for patient monitoring.
- A Mindray DPM 5 operator manual on `mindray.com` describes demo mode as training-only and warns that clinical use can mislead staff and delay or distort treatment decisions.
- One Philips CMS service-guide URL cited in the issue was not retrievable on 2026-05-06, so it should be treated as unstable supporting context rather than the sole precedent.

The evidence is strong enough to justify a non-copying MVP boundary: generic explicit warning language is common industry practice, but exact competitor layouts or branded UX should not be copied.

## Medical-safety impact

Primary hazard: synthetic values are mistaken for real patient-monitoring data.

Potential harms:

- delayed response because staff assume a synthetic stable display reflects a real patient
- inappropriate escalation because staff react to simulated deterioration as if it were live
- incorrect reuse of screenshots, summaries, or session outputs as clinical evidence

This issue is appropriately safety-scoped even though it does not change clinical logic. It addresses a foreseeable interpretation error at the presentation layer.

## Security and privacy impact

Security impact is low. The change does not require new permissions, storage, networking, authentication, or cryptography. The main integrity benefit is clearer provenance of synthetic data.

Privacy impact is low to none if the feature remains presentation-only. No new identifiers, exports, logs, or patient-data flows should be introduced.

## Affected requirements or "none"

Primary affected requirements:

- `UNS-014`
- `UNS-015`
- `SYS-014`
- `SWR-GUI-010`
- `SWR-GUI-011`

Conditionally affected if synthetic labels are extended into patient or session review surfaces:

- `SWR-GUI-004`

Important requirement concern:

- `SWR-GUI-011` currently requires a decorative rolling message and even hardcodes non-safety wording. That requirement must be revised or replaced before implementation, otherwise the safer design would still be noncompliant with the approved requirement set.

`SYS-015` is contextual only. The candidate should not change HAL behavior.

## Intended use, user population, operating environment, and foreseeable misuse

Intended use for this candidate:

- demo, DVT, training, and UI-development presentation of synthetic monitoring data

Primary users:

- developers
- testers
- reviewers
- demo operators

Operating environment:

- local Win32 desktop application
- simulation-backed sessions and screenshots
- review surfaces that may be shared outside the running app

Foreseeable misuse:

- simulation mode is displayed in a clinical-looking context and assumed to be live monitoring
- a paused simulation is mistaken for a frozen real-patient display
- synthetic session summaries or screenshots are reused without an obvious non-clinical marker
- the wording hardens one screen but leaves another synthetic-data screen ambiguous

## Severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

Severity: Serious. If synthetic values were treated as actual patient status in a real-care context, misuse could contribute to delayed or improper treatment.

Probability: Low in the current pilot. The repository is still demo-oriented, uses a simulation back-end by default, and does not introduce a new live-care workflow in this issue.

Initial risk: Medium. Severity is meaningful even though current deployment probability is limited.

Risk controls required:

1. Use an explicit non-clinical warning on every in-scope simulation surface.
2. Keep the warning visible when simulation is live, paused, resumed, or reviewed after capture.
3. Make the warning legible against all alert-level backgrounds and not dependent on color alone.
4. Keep the change presentation-only; no new calculation, alarm, or acquisition behavior.
5. Revise requirements and traceability evidence so the safer behavior is the approved behavior.

Verification method expected downstream:

- requirement review for revised `SWR-GUI-010` and `SWR-GUI-011` intent
- GUI demonstration for sim on or off, pause or resume, patient present or absent, and synthetic session-review surfaces
- targeted regression of configuration, localization, and patient/session behavior

Residual risk: Low if all controls above are implemented.

Residual-risk acceptability rationale: Acceptable for this pilot because the change reduces a meaningful interpretation hazard without widening clinical claims, provided the product continues to present simulation as non-clinical and the project does not represent the UI as bedside-ready.

## Hazards and failure modes

- warning appears in the banner but not in header or session-review surfaces
- warning disappears when simulation is paused
- warning contrast becomes unreadable on warning or critical backgrounds
- session summaries derived from synthetic data lack the same marker as the live dashboard
- implementation changes text on screen but leaves requirements and verification evidence stale
- wording implies the system is safe for patient monitoring once simulation is turned off

## Existing controls

- `sim_enabled` already gates simulation behavior and is persisted in configuration
- the dashboard already distinguishes simulation and device modes
- header badges currently show `* SIM LIVE` or `SIM PAUSED`
- status messaging already changes by operating mode
- the current architecture keeps simulation behind the HAL and separate from the future hardware path

These controls help, but they are not sufficient because the current wording is subtle and partially decorative rather than unmistakably non-clinical.

## Required design controls

- replace the decorative simulation message with explicit non-clinical wording
- ensure the warning is present on all synthetic-data surfaces named in the issue, not only the status banner
- keep the marker visible for paused simulation and for any session-summary or historical-review surface generated from synthetic data
- use generic warning text and repo-native styling rather than copying competitor layout details
- update requirements and `requirements/TRACEABILITY.md` alongside the UI change
- preserve current clinical logic, alert logic, timer behavior, and HAL boundaries

## Validation expectations

- verify simulation-on surfaces all show the non-clinical marker
- verify simulation-off surfaces return to device-mode messaging without synthetic-data remnants
- verify pause or resume does not suppress the warning
- verify synthetic session-review surfaces remain marked after navigation and refresh
- verify localization and text rendering still fit available UI regions
- verify no tests or demos that cover configuration persistence and patient/session flow regress because of the wording change

## MVP boundary that avoids copying proprietary UX

- Use generic warning text such as a clear all-caps non-clinical label.
- Reuse existing repository surfaces and visual components.
- Do not copy competitor screen layout, iconography, wave placement, or product-specific demo conventions.
- Limit the MVP to text and presentation hardening; do not add training workflows, hardware detection, or workflow orchestration.

## Clinical-safety boundary and claims that must not be made

- Do not claim this change makes the product safe for bedside use.
- Do not claim real-patient monitoring readiness, validated hardware integration, or regulatory clearance.
- Do not imply that clearer simulation labeling mitigates risks from incorrect thresholds, alarms, or treatment logic.
- Do not describe synthetic session outputs as clinical records.

## Residual risk for this pilot

After implementation, the main remaining risk is deliberate or careless misuse by a human who ignores clear warnings or reuses synthetic outputs outside context. That residual risk is acceptable for the pilot if the UI stays explicit, the change remains local to presentation, and future work does not widen the claim set.

## Human owner decision needed, if any

Human owner confirmation is still needed on two points:

- approve the exact standard warning phrase to be used across all synthetic-data surfaces
- approve retirement or replacement of the current `SWR-GUI-011` decorative-message requirement

These are not blockers to architectural design. They should be resolved in the design and requirements update before implementation starts.

## Whether the candidate is safe to send to Architect

Yes. The candidate is safe to send to Architect because it is narrowly scoped, addresses a real human-factors hazard, has enough public source precedent to define a non-copying MVP, and does not require changes to clinical algorithms or hardware behavior.
