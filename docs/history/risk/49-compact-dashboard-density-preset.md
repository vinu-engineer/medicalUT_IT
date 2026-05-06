# Risk Note: Issue #49

Date: 2026-05-06
Issue: `#49` - Feature: add a compact dashboard density preset

## proposed change

Add a user-selectable dashboard density preset with exactly two modes,
`standard` and `compact`, for the existing Win32 monitoring dashboard.

The compact preset should change presentation-only attributes such as padding,
tile spacing, and controlled label wrapping. It must not change monitored
parameters, measured values, alert thresholds, alert timing, NEWS2 behavior,
patient-record content, or the order and presence of the existing dashboard
tiles and alert surfaces.

## product hypothesis and intended user benefit

The product hypothesis is credible: some operators need the same monitoring
context on smaller or denser workstation layouts, and the current dashboard is
fixed-width with generous spacing.

Expected user benefit is operational rather than clinical. The feature may
reduce scrolling, resizing, and wasted white space while preserving the same
clinical content and alert logic.

## source evidence quality

Evidence quality is adequate for product-discovery direction, but not for
clinical or human-factors claims.

- The Philips IntelliVue X3 public product page describes multiple screens
  tailored to clinical situations and screen customization.
- The Drager Vista 300 public product page describes a configurable layout and
  broader waveform density.
- Both sources are vendor marketing materials, not independent usability
  studies, not clinical evidence, and not permission to copy proprietary UX.

That is enough to support the hypothesis that configurable monitor layouts are a
real market expectation. It is not enough to claim improved safety, faster
detection, or better clinician performance from this feature alone.

## MVP boundary that avoids copying proprietary UX

The MVP should stay generic and narrow:

- two presets only: `standard` and `compact`
- preserve the existing tile set, tile order, color mapping, and overall
  dashboard structure
- do not copy a competitor's waveform counts, screen anatomy, gestures, icons,
  labels, or proprietary navigation model
- do not add arbitrary per-user layout editors, hidden panels, or parameter
  suppression in this issue

## clinical-safety boundary and claims that must not be made

This candidate is acceptable only as a presentation-density change.

The design must not claim that compact mode improves diagnosis, triage,
response time, or patient outcomes unless separate human-factors evidence is
generated. It must not alter threshold logic, alarm prominence, measured-value
formatting semantics, or which parameters are visible.

## medical-safety impact

Direct clinical-logic risk is low because the requested change is display-only.
The main safety risk is indirect: tighter spacing can reduce legibility or hide
salient warning information.

The change becomes medically unsafe if compact mode clips numeric values,
obscures abnormal-status color or text, truncates active alerts, or makes the
shared-monitor state ambiguous for the next user.

## security and privacy impact

Security and privacy impact should remain low if the feature only stores a
non-patient display preference.

If persistence is added through `monitor.cfg`, the design must keep the stored
state limited to the density preset and must not introduce patient data,
operator identifiers, or audit-significant workflow data into the file.

## affected requirements or none

No approved requirement currently covers dashboard density as a distinct
feature.

This candidate will likely require a narrow requirement addition or update
under:

- `SYS-014` Graphical Vital Signs Dashboard
- existing GUI/persistence behavior around `SWR-GUI-003`, `SWR-GUI-010`, and
  related settings/localization flows if the preset is exposed in Settings and
  persisted in `monitor.cfg`

## intended use, user population, operating environment, and foreseeable misuse

Intended use remains unchanged: trained clinical staff or supervised demo
operators viewing the live monitoring dashboard on a Windows workstation.

User population:

- bedside clinicians
- ward or ICU staff
- demo or training operators in the pilot environment

Operating environment:

- shared Windows workstations
- desktop displays where the current fixed dashboard may feel spacious or
  crowded depending on monitor size and scaling

Foreseeable misuse:

- enabling compact mode on a small or low-resolution screen where values or
  badges become hard to read
- assuming compact mode is a different monitoring mode rather than the same
  data in a tighter layout
- leaving a shared workstation in compact mode and surprising the next user

## severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

Severity: moderate if a dense layout reduces readability enough to delay
recognition of abnormal values or active alerts.

Probability: low to medium without controls, because layout regressions are
common when spacing and wrapping change across screen widths and DPI settings.

Initial risk: medium.

Primary risk controls:

- preserve all existing values, parameters, and alert surfaces in both presets
- preserve current alert color semantics and warning/critical salience
- define minimum readable text and badge sizing for supported window sizes
- prohibit clipping or ellipsis for current-value numerics and severity badges
- make density selection explicit in Settings and deterministic on restart

Verification method:

- targeted GUI review of both presets at supported window sizes and common DPI
  settings
- confirmation that the same input readings produce the same values, alerts,
  and NEWS2 result in both presets
- persistence checks for save, reload, and fallback to `standard` on invalid
  config values

Residual risk: low if the implementation remains presentation-only and passes
explicit readability and persistence checks.

Residual-risk acceptability rationale: acceptable for this pilot because the
feature does not change clinical calculations or alarm logic, and the remaining
hazards are bounded to UI legibility that can be directly reviewed before
release.

## hazards and failure modes

- Numeric values or units may clip, wrap poorly, or become ambiguous in compact
  mode.
- Warning or critical salience may be reduced if color blocks, badges, or
  spacing become too small.
- Trend sparklines or active-alert lists may lose useful context if vertical
  compression is too aggressive.
- Persisted compact mode may surprise the next operator on a shared workstation.
- A later scope increase may quietly add or remove displayed parameters under
  the label of "density," creating an unreviewed clinical workflow change.

## existing controls

- The current dashboard already uses fixed parameter presence and stable tile
  ordering.
- Alert thresholds, NEWS2 calculation, and patient-record behavior live outside
  the requested presentation change.
- The application already has settings/config persistence patterns through
  `monitor.cfg`.
- The dashboard also exposes an active-alert list and aggregate-status banner,
  which reduce reliance on any single tile.

## required design controls

- Keep the feature strictly presentation-only for this issue.
- Preserve the existing six dashboard slots and current alert surfaces in both
  presets.
- Define and document minimum acceptable legibility for values, units, labels,
  badges, and alert list content.
- Decide explicitly whether density persistence is shared per installation or
  scoped differently; document that choice in the design because the current
  config file is workstation-local rather than user-scoped.
- Default unknown or invalid persisted values to `standard`.
- Add traceable verification evidence for layout behavior and persistence if the
  feature is implemented.

## validation expectations

- Manual GUI review of `standard` and `compact` layouts with identical readings.
- Confirm no parameter disappears and no alert text/value is clipped in the
  supported dashboard window size.
- Confirm warning and critical colors, badges, and NEWS2 tile remain equally
  salient in both presets.
- Confirm the selected preset persists and restores predictably if persistence
  is added.
- Run normal project validation commands (`build.bat`, `run_tests.bat`) to
  ensure the change does not regress non-UI behavior.

## residual risk for this pilot

Residual risk is low if the team keeps the MVP narrow and treats compact mode as
layout density only.

Residual risk becomes medium if the design allows hidden data, substantially
smaller alert affordances, or ambiguous shared-state persistence.

## whether the candidate is safe to send to Architect

Yes, with constraints.

This candidate is safe to send to Architect because the issue already sets a
display-only boundary, the product hypothesis is supported by public market
evidence, and the clinical safety boundary can be kept narrow. The Architect
should be instructed to preserve data equivalence, alert salience, and explicit
persistence behavior as first-class acceptance criteria.

## human owner decision needed, if any

One explicit product decision is still needed: whether the density preset is a
shared workstation preference in `monitor.cfg` or should remain session-scoped.

If the team cannot define minimum readable layout constraints for the supported
window size and DPI range, the feature should stop at design review rather than
move into implementation.
