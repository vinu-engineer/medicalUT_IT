# Risk Note: Issue 70

Issue: `#70`
Branch: `feature/70-low-glare-night-mode`

## proposed change

Add a single manual low-glare night mode to the Win32 dashboard so bedside
review is less visually harsh in low-light rooms. The approved scope for risk
assessment is presentation-only: lower-luminance colors and backgrounds for the
header, patient bar, vital tiles, NEWS2 tile, status banner, and related
settings surfaces, while leaving vital values, alert thresholds, NEWS2 logic,
alarm generation, patient data handling, and authentication unchanged.

Repo evidence supports a bounded implementation path. Current display colors are
centralized in `src/gui_main.c`, especially the palette constants and tile or
banner painting flow, while persisted UI preferences already use the simple
`monitor.cfg` path in `src/app_config.c`.

## product hypothesis and intended user benefit

The product hypothesis is that clinicians reviewing a monitor in a dim room may
prefer a lower-glare presentation that preserves the same information and alert
meaning while reducing visual harshness. The intended user benefit is comfort
and readability during low-light review, not a change in clinical decision
logic, alarm behavior, or patient management.

## source evidence quality

Source evidence quality is moderate for feature existence and low for clinical
benefit. The issue cites a public Philips Efficia CM12 technical data sheet
accessed on 2026-05-06. That document states that the monitor supports manual
and automatic night mode and presents night mode as a bedside-monitor feature,
which is enough to justify a narrow product-discovery hypothesis that this
affordance exists in the market. It is not clinical evidence that night mode
improves outcomes, and it does not justify copying another vendor's workflow,
layout, timing logic, or styling.

## MVP boundary that avoids copying proprietary UX

The MVP should stay limited to:

- one manual toggle only
- a lower-glare palette that preserves existing tile layout and alert semantics
- a visible indication that night mode is active
- local preference persistence only if its scope is explicitly defined

The MVP should not include:

- automatic scheduling by time of day
- ambient-light sensing
- Philips naming, layout, iconography, or interaction patterns
- any change to alarm thresholds, alarm audio rules, NEWS2, or patient session logic

## medical-safety impact

Medical-safety impact is low to moderate before controls and low after controls.
The feature does not change clinical algorithms, thresholds, or alerts if it
remains presentation-only. The safety concern comes from human factors: the
dashboard currently uses strong color semantics for `NORMAL`, `WARNING`, and
`CRITICAL` states, so an overly dim or low-contrast palette could reduce the
salience of abnormal conditions and delay recognition.

## security and privacy impact

No new patient-data flow, network path, credential flow, or access-control
behavior is needed. Privacy impact is none expected if the feature stores only a
display preference. The main non-clinical risk is integrity and usability on a
shared workstation: if the preference is persisted globally in `monitor.cfg`,
the next user may inherit an unexpected display mode.

## affected requirements or "none"

No currently approved UNS, SYS, or SWR entry explicitly defines night mode.
If the feature proceeds, design should propose new or updated GUI-level
requirements covering:

- manual activation and deactivation behavior
- default state and persistence scope
- preserved alert-state color semantics and readability expectations
- verification evidence for visual review in both normal and low-light use

## intended use, user population, operating environment, and foreseeable misuse

Intended use remains unchanged: trained clinical staff review patient-monitoring
information on a Windows workstation. The likely users are bedside clinicians
and ward staff reviewing the display in dim rooms, including overnight care
areas. The operating environment includes shared workstations where multiple
users may log in during the same shift.

Foreseeable misuse includes:

- assuming night mode lowers clinical urgency rather than only changing display brightness
- reducing contrast so far that warning or critical states are harder to distinguish
- leaving the mode enabled for the next user on a shared workstation without an obvious indicator
- treating night mode as a privacy or alarm-silence feature when it is neither

## clinical-safety boundary and claims that must not be made

This candidate may support display ergonomics only. It must not be described as:

- reducing alarm fatigue
- improving diagnostic accuracy
- improving patient outcomes
- providing privacy protection
- replacing alarm acknowledgement or alarm-volume controls

Clinical claims remain unchanged. Alert meaning, severity logic, thresholds,
NEWS2 scoring, and any patient-care workflow must stay exactly as they are.

## severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

- Severity: potentially serious if poor contrast or altered semantics causes a
  delayed recognition of `WARNING` or `CRITICAL` status.
- Probability before controls: medium, because palette changes touch most of the
  visual surfaces in the active dashboard.
- Initial risk: medium.
- Risk controls:
  preserve distinct normal, warning, and critical visual states;
  keep warning and critical contrast at least as readable as the current mode;
  keep alert text, badge labels, and alarm logic unchanged;
  show an explicit "night mode active" indicator;
  fail safe to standard mode if the preference is missing or invalid;
  define whether persistence is per user, per workstation, or session-only
  before implementation.
- Verification method:
  manual GUI review in normal and night modes using normal, warning, and
  critical simulations;
  explicit check that NEWS2 and tile values are unchanged between modes;
  persistence and restart checks for the stored preference;
  logout/login handoff check on a shared workstation scenario;
  low-ambient-light readability review with documented screenshots or test notes.
- Residual risk after controls: low.
- Residual-risk acceptability rationale:
  acceptable for this pilot if the design remains presentation-only and all
  controls above are satisfied, because the feature does not alter clinical
  computation or alarm generation and the remaining risk is a bounded
  human-factors visibility risk that can be directly reviewed.

## hazards and failure modes

- `WARNING` and `CRITICAL` colors become too similar to `NORMAL`.
- Numeric text loses contrast against darker tile backgrounds.
- The status banner and tile colors diverge and present inconsistent urgency.
- The feature persists across users on a shared workstation without clear notice.
- A corrupt or partial config value leaves the UI in an unreadable state.
- Designers expand the change into automatic schedule logic or alarm-behavior
  changes without a separate risk review.

## existing controls

- Domain logic for vital classification, alert generation, and NEWS2 is
  separated from presentation by the documented architecture.
- The issue explicitly keeps thresholds, alerting, NEWS2, and patient data
  handling out of scope.
- Current visual palette and painting behavior are centralized enough in
  `src/gui_main.c` to support a bounded review.
- Existing config persistence is already isolated in `src/app_config.c`, which
  reduces the chance that the feature needs invasive cross-module changes.

## required design controls

- Keep the first implementation manual only; do not add automatic day or night behavior.
- Preserve the existing `NORMAL` / `WARNING` / `CRITICAL` wording and clinical meaning.
- Define a contrast target or explicit visual acceptance criteria before coding.
- Add a clear visible indicator when night mode is enabled.
- Decide and document persistence scope before implementation. If the team
  cannot support user-scoped persistence safely, default to standard mode on
  login rather than silently inheriting a prior user's preference.
- Keep device-mode, simulation-mode, and localization messaging readable under
  the alternate palette.
- Treat invalid or missing persisted values as standard mode.

## validation expectations

- Manual comparison of standard mode and night mode for normal, warning, and
  critical monitor states.
- Confirm that alarm severity semantics, NEWS2 score, and all displayed values
  are identical across modes.
- Confirm that the header, patient bar, tiles, and status banner all switch
  together and do not mix palettes.
- Verify preference persistence or reset behavior exactly matches the approved
  scope decision.
- Verify that the mode remains readable in device mode, simulation mode, and
  after localization changes.

## residual risk for this pilot

Residual risk is low if the change stays within the approved MVP boundary and
the design controls above are implemented. The main remaining concern is human
factors readability, not a change in clinical logic.

## whether the candidate is safe to send to Architect

Yes. This candidate is safe to send to Architect as a bounded human-factors and
presentation feature, provided the design keeps the scope manual and
presentation-only and explicitly addresses alert visibility and persistence
scope.

## human owner decision needed, if any

One product decision should be made before implementation starts: should night
mode be session-only, user-specific, or workstation-wide when the device is used
by multiple clinicians? The current config pattern is workstation-local, so this
decision affects handoff risk and must be explicit.
