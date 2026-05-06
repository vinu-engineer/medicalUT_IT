# Risk Note: Issue #75

Issue: `#75`
Branch: `feature/75-show-simulation-live-provenance`

## proposed change

Issue `#75` proposes an explicit dashboard-header provenance indicator so an
operator can tell whether the displayed values are coming from the simulator or
from live acquisition.

The current product hypothesis narrows that to a small `SIM` / `LIVE` badge
driven by the existing `sim_enabled` flag. In the current repo, that flag does
not distinguish "simulation" from "validated live acquisition." Approved and
implemented behavior today is:

- `sim_enabled = 1`: synthetic readings are produced from the simulation HAL.
- `sim_enabled = 0`: the timer stops, the tiles show `N/A`, and the UI tells
  the operator to connect real hardware for live data.

That means the candidate, as written, conflates "device mode" with "live
clinical feed."

## product hypothesis and intended user benefit

The user benefit is plausible. A visible provenance cue can reduce operator
confusion during demo, training, and future hardware-backed workflows by making
the source of displayed values explicit instead of inferred.

The unsafe assumption is the proposed wording. In this pilot, turning
simulation off does not make the data live. It makes the dashboard enter a
no-live-feed device mode. A `LIVE` badge tied only to the current flag would be
factually wrong and could overstate clinical readiness.

## source evidence quality

- The GE ApexPro CH product page supports the general need for visible
  telemetry integrity and interference awareness, but it is marketing material,
  not a design control for this product.
- The Mindray ePM 15M operator's manual supports the general concept of visible
  signal-confidence cues through the SpO2 signal quality indicator, but that is
  a parameter-level confidence display, not a monitor-wide source-provenance
  badge.
- Repo evidence is stronger than competitor evidence for this decision. The
  approved requirements and `src/gui_main.c` show only a simulation mode and a
  device mode with no live data path.

Overall evidence quality is moderate for the problem statement "operators
benefit from explicit provenance," but low for the proposed implementation
"show `SIM` / `LIVE` from the existing simulation flag."

## MVP boundary that avoids copying proprietary UX

- Keep the MVP limited to truthful source-state indication for this product.
- Do not copy competitor wording, waveform markers, signal bars, colors, or
  monitor chrome.
- Do not expand this issue into per-parameter signal-quality meters.
- Do not use `LIVE` unless a separate approved and validated live-acquisition
  state exists.

## clinical-safety boundary and claims that must not be made

- The UI must not claim `LIVE`, `real-time`, `connected`, or equivalent when
  the application is only in device mode with no current hardware sample path.
- The UI must not imply signal quality, sensor attachment quality, or clinical
  suitability from a source-state badge alone.
- The UI must remain display-only and must not alter alert thresholds, alarm
  logic, NEWS2 scoring, or patient-record behavior.

## whether the candidate is safe to send to Architect

No. The candidate is not safe to send to Architect as written because it uses
the current `sim_enabled` flag as if it were a simulation-versus-live truth
source. In the approved design, `sim_enabled = 0` means "device mode / no live
feed available," not "live acquisition active."

This should be re-scoped before architecture work begins.

## medical-safety impact

This is nominally a display-only change, but provenance labeling is
medical-safety relevant because it affects whether a clinician or tester may
trust the displayed values as representing a current patient feed.

The primary hazard is false reassurance: if the header shows `LIVE` while the
system is actually in no-data device mode, operators can misread the product
state and treat synthetic, stale, or absent acquisition context as clinically
meaningful. For a monitoring product, that integrity failure is more important
than the small visual scope suggests.

## security and privacy impact

No new patient-data flow, credential flow, or external interface is proposed.
Privacy impact is none expected.

Integrity risk is material. A false provenance label would reduce operator
trust and weaken auditability of screenshots, demonstrations, and validation
evidence by making the source state misleading.

## affected requirements or "none"

Affected approved requirements are:

- `UNS-014` graphical dashboard clarity
- `UNS-015` live monitoring feed expectations
- `SYS-014` graphical vital-signs dashboard
- `SYS-015` hardware abstraction layer
- `SWR-GUI-010` simulation mode toggle and device-mode behavior
- `SWR-GUI-011` simulation-mode banner semantics

This candidate also needs a new approved requirement set for acquisition-source
state indication before implementation. The existing requirements do not define
a truthful `LIVE` state in the current pilot.

## intended use, user population, operating environment, and foreseeable misuse

- Intended use: trained clinical staff use the Windows workstation dashboard to
  monitor and review vital-sign readings and alert status.
- User population: bedside clinicians, ward nurses, intensivists, and pilot
  operators running demonstrations or evaluation sessions.
- Operating environment: a Windows workstation in a pilot monitoring context
  where simulation-heavy workflows are common and real hardware integration is
  not yet represented by an approved production state model.
- Foreseeable misuse: an operator assumes `LIVE` means bedside hardware is
  connected and actively feeding current patient data when the system is only
  out of simulation mode; a reviewer treats screenshots labeled `LIVE` as live
  evidence; or a user overlooks a stale or absent source transition during
  startup, login, logout, or settings changes.

## severity, probability, initial risk, risk controls, verification method, residual risk, residual-risk acceptability rationale

- Severity: serious, because false source provenance can influence whether
  displayed values are trusted in a clinical-style workflow.
- Probability: occasional if the design directly maps `sim_enabled = 0` to
  `LIVE`, because that mapping would be present on every non-simulation use.
- Initial risk: high enough to block the current candidate wording.
- Required risk controls:
  1. Define an explicit approved source-state model before design. At minimum,
     distinguish `SIMULATION ACTIVE` from `DEVICE MODE - NO LIVE FEED`.
  2. Reserve any `LIVE` label for a separate validated state that proves a real
     hardware path is connected, enabled, and delivering fresh readings.
  3. Add stale-data and transition rules so the badge cannot remain `LIVE`
     through logout, startup, feed loss, or paused acquisition.
  4. Use text, not color alone, for the state indication.
  5. Update requirements and traceability before implementation.
- Verification method:
  1. Requirements review for the source-state definitions.
  2. UI verification across startup, login, logout, simulation toggle, paused
     simulation, no-hardware device mode, and future live-feed transitions.
  3. Negative tests proving `LIVE` never appears when only the simulation flag
     changed and no validated live feed exists.
- Residual risk: acceptable only after the above controls are implemented and
  verified. Residual risk is not acceptable for the current `SIM` / `LIVE`
  proposal tied to the existing flag.

## hazards and failure modes

- The header shows `LIVE` when the system is only in device mode with `N/A`
  tiles and no active hardware feed.
- The badge becomes out of sync with actual source state during startup,
  logout, or settings changes.
- A paused, stale, or disconnected future hardware feed continues to show
  `LIVE`.
- The design relies on color alone and the state is misread.
- Reviewers or testers misclassify simulation evidence as live-acquisition
  evidence because of the label text.

## existing controls

- `SWR-GUI-010` already requires device mode to show `N/A` tiles and explicit
  text instructing the operator to connect real hardware for live data.
- `src/gui_main.c` already separates simulation mode from device mode and does
  not present an approved live-data state when simulation is disabled.
- The HAL boundary in `hw_vitals.h` and `SYS-015` prevents the GUI from making
  low-level assumptions about specific simulation internals.
- The issue itself scopes out alarm and threshold changes, which limits direct
  clinical-control impact.

## required design controls

- Re-scope the candidate to truthful current-state wording such as `SIMULATION`
  versus `DEVICE MODE`, or defer `LIVE` until hardware-source semantics are
  approved.
- Add a dedicated requirement for acquisition-source indication, including the
  allowed states, text, colors, stale timeout, and transition behavior.
- Define the evidence needed to claim `LIVE`, including source freshness and
  hardware-read success criteria.
- Keep the provenance badge distinct from signal-quality, alarm, and
  physiological-status indicators.
- Update README, architecture notes, and traceability only after the new state
  model is approved.

## validation expectations

- UI review confirms that no screen state labels absent hardware data as live.
- Tests cover simulation on, simulation paused, simulation off/device mode,
  startup before first reading, logout/login transitions, and any future
  hardware disconnect or stale-data transition.
- Requirements and traceability updates are reviewed before code implementation.
- Human review confirms the chosen wording is clinically honest and does not
  overstate the maturity of hardware integration.

## residual risk for this pilot

Residual risk is not acceptable for the issue as currently phrased because it
encourages a misleading `LIVE` claim without an approved live-data state. If
the issue is narrowed to truthful source-state language and future `LIVE`
semantics are separately defined and validated, the residual risk for a pilot
display-only badge can likely be reduced to low.

## human owner decision needed, if any

Yes. A product/clinical owner must decide whether this issue should:

1. be narrowed to a truthful current-state badge such as `SIMULATION` versus
   `DEVICE MODE`, or
2. be deferred until the product has a validated live-hardware state model that
   justifies a `LIVE` claim.

Recommendation: block the current issue wording and reopen design only after
that decision and the related requirements updates are made.
