# Design Spec: Surface alarm pause countdown in the status banner

Issue: #69
Branch: `feature/69-surface-alarm-pause-countdown-in-status-banner`
Spec path: `docs/history/specs/69-surface-alarm-pause-countdown-in-status-banner.md`

## Problem

Issue #69 asks the dashboard to show how much time remains in a temporary
alarm-audio pause so an operator can tell at a glance when audible alarms are
expected to resume.

The current repository does not yet have a trustworthy alarm-pause countdown
surface, and it also does not expose an authoritative alarm-audio pause state
or pause-expiry field in the current `main` code path. In `src/gui_main.c`, the
only nearby concept is `g_app.sim_paused`, which pauses synthetic data updates
for simulation mode. Reusing that state for alarm-audio pause would be wrong:
it would conflate simulation transport control with alarm-audio behavior and
would create a misleading safety-adjacent UI.

The current status banner is also not a clean alarm-status surface. In
simulation mode `paint_status_banner()` renders a rolling simulation message,
and in device mode it renders a device-mode string. That means the countdown
feature cannot be implemented safely as "just append text" unless the design
first defines:

- the authoritative source of alarm-audio pause-active state
- the authoritative source of pause-expiry time
- banner priority when pause state overlaps simulation/device/aggregate status
- fail-safe behavior when pause state is unavailable or stale

## Goal

Add a narrow, display-only countdown in the existing dashboard status banner
that appears only while local alarm audio is actively paused and shows the
remaining time until automatic expiry or manual resume.

The design goal is to make the feature:

- driven by authoritative pause state and expiry data, not by a UI-only counter
- explicitly scoped to alarm audio pause, not general alarm suppression
- visually distinct from a normal/green aggregate status banner
- independent from simulation pause/resume behavior
- easy to clear immediately when the pause ends or becomes indeterminate

## Non-goals

- No changes to alarm thresholds, alert generation, aggregate severity, NEWS2,
  alarm-limit behavior, authentication, persistence, or release tooling.
- No new pause, resume, acknowledge, silence, or alarm-routing control under
  this issue.
- No deriving the countdown from `g_app.sim_paused`, `reading_count`, or the
  simulator sample cadence.
- No claim that the countdown reflects physiological onset time, clinician
  acknowledgement age, or any clinical effectiveness metric.
- No copying competitor wording, iconography, countdown placement, or color
  composition.
- No CLI summary, export, audit, or historical review changes.

## Intended use impact, user population, operating environment, and foreseeable misuse

- Intended use impact: show trained operators that local alarm audio is
  temporarily paused and when that pause is expected to end.
- User population: bedside clinicians, ward nurses, and other trained operators
  using the Windows dashboard.
- Operating environment: the current single-process Win32 dashboard, initially
  in the existing pilot environment and later in the same UI path used for live
  monitoring.
- Foreseeable misuse: a user may read the banner as meaning all alarms are off
  when only audible alarm output is paused.
- Foreseeable misuse: a user may trust a stale or frozen countdown after pause
  expiry, logout, or session reset.
- Foreseeable misuse: a user may confuse alarm-audio pause with simulation
  pause if the implementation reuses the existing `SIM PAUSED` state.
- Foreseeable misuse: a user may interpret the countdown as an acknowledgement
  timer or alarm-age timer rather than a pause-expiry timer.

## Current behavior

- `AppState` in `src/gui_main.c` stores `sim_paused`, `sim_enabled`, and
  `sim_msg_scroll_offset`, but no alarm-audio pause-active flag or pause-expiry
  timestamp.
- `paint_header()` already shows `SIM PAUSED` or `* SIM LIVE` based on
  `sim_paused`, reinforcing that the existing pause concept is simulation-only.
- `paint_status_banner()` currently renders:
  - a device-mode string when simulation is disabled
  - a rolling simulation message when simulation is enabled
- `WM_TIMER` currently advances dashboard state only through `TIMER_SIM`, and
  that timer path is skipped when `sim_paused` is true. A countdown tied to
  this timer would freeze incorrectly if simulation were paused.
- `update_dashboard()` rebuilds listbox content and invalidates the window, but
  it does not maintain pause-expiry state or countdown formatting.
- Localization contains strings for simulation pause/resume and status text, but
  none for alarm-audio pause wording or countdown text.
- Issue #64 (`add an alarm-audio status badge`) is adjacent repo context and
  already recognizes that a real alarm-audio source-of-truth is needed. This
  issue should reuse that same state model if it lands first; it must not
  invent a second independent interpretation.

## Proposed change

Implement the feature as a banner-only presentation of an authoritative
alarm-audio pause state plus expiry time.

### 1. Define a single source of truth for pause-active state and expiry

The countdown must be driven by an authoritative application-layer pause model
with, at minimum, the following semantics:

- whether local alarm audio pause is currently active
- whether the pause state is known/trustworthy
- when the current pause is expected to expire

This can be represented either by extending the local alarm-audio status model
planned for issue #64 or by adding equivalent fields in the same application
layer. The important constraint is that `gui_main.c` banner paint code reads a
single trusted state instead of decrementing its own independent counter.

Preferred shape:

- `alarm_audio_pause_active`
- `alarm_audio_pause_known`
- `alarm_audio_pause_expires_at_tick`

The exact storage type is up to implementation, but the data must support
clamping and monotonic countdown updates.

### 2. Treat simulation pause and alarm pause as separate concepts

The design must explicitly preserve this separation:

- `sim_paused` controls simulation data flow only
- alarm-audio pause controls countdown visibility only

The countdown must not start, stop, reset, or clear solely because simulation
is paused or resumed, unless the underlying alarm-pause source itself changes.
This avoids a misleading banner that appears to change alarm behavior when the
operator only paused synthetic data playback.

### 3. Add banner-priority rules

The status banner needs deterministic priority so overlapping states remain
comprehensible.

Recommended priority order:

1. Device-mode / no-monitoring hard-stop messaging when the application is not
   presenting live monitoring status.
2. Alarm-audio paused countdown when pause-active state is known and active.
3. Existing non-paused banner behavior for the current branch baseline.

While the pause countdown is visible, it should temporarily replace the rolling
simulation banner text rather than attempt to share the same line with the
scrolling message.

### 4. Use explicit audio-only wording

The issue body and risk note both point toward an audio-only pause, not a
general "all alarms disabled" state. The visible wording should therefore stay
precise.

Recommended banner text pattern:

- `Alarm audio paused - resumes in MM:SS`

If implementation later proves the underlying product behavior is broader than
audio-only pause, stop and escalate rather than silently shipping overstated
wording.

### 5. Use a real-time countdown, not sample-based decay

The countdown should be computed from current monotonic time against the stored
expiry tick, then clamped to zero:

- `remaining_ms = max(0, expires_at - now)`
- display as `MM:SS`

The display must update on a cadence that reflects real elapsed time. A
dedicated 1 Hz UI timer is preferred. The countdown must not depend on the
2-second simulation timer because that timer:

- is too coarse for a precise countdown
- stops while `sim_paused` is true
- would drift from the real pause-expiry source

### 6. Define clear and fail-safe reset conditions

The countdown must clear immediately when any of the following occurs:

- manual resume of alarm audio
- automatic pause expiry
- pause state becomes unknown or invalid
- logout or dashboard teardown
- session clear or state transition that invalidates the current monitoring
  context

The UI must never show:

- negative remaining time
- a paused banner after expiry
- a paused banner after manual resume
- a paused banner sourced only from stale persisted UI state

This issue should not add independent countdown persistence to `monitor.cfg`.

### 7. Keep paused-state styling visibly non-normal

The paused banner must not look like `ALL NORMAL` or any other reassuring
steady-state condition.

Recommended visual behavior:

- use a dedicated caution presentation for the paused banner
- keep the countdown text legible and stable
- preserve patient-severity information through the existing tiles and active
  alerts list rather than pretending the pause state is a clinical severity

Do not reuse normal-state wording or styling that could be mistaken for
resolved patient risk.

### 8. Localize all new banner strings

Implementation should route all new text through the existing localization
layer. At minimum, add strings for:

- `Alarm audio paused`
- `Resumes in`
- any compact fallback wording needed for narrow layouts

Do not hard-code English-only countdown text in `gui_main.c`.

### 9. Make the dependency on issue #64 explicit

Issue #64 is the nearest design dependency because it establishes the general
alarm-audio status model. This issue should reuse that model rather than create
parallel state semantics.

Implementation rule:

- if issue #64 has merged, extend its authoritative alarm-audio state with
  pause-active and expiry metadata
- if issue #64 has not merged, implement the minimum equivalent state plumbing
  needed for this issue in the same layer, without adding alarm-control UI

If implementation discovers there is still no trustworthy pause-active /
pause-expiry source, stop and split a prerequisite issue instead of deriving
the countdown from simulation controls or dummy UI state.

## Files expected to change

Expected implementation files:

- `src/gui_main.c`
- `include/localization.h`
- `src/localization.c`

Expected requirements and traceability files:

- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`

Expected verification-support files:

- `tests/unit/test_localization.cpp`

Optional helper-unit files if implementation extracts countdown formatting or
pause-state derivation into a pure helper for testability:

- `include/alarm_pause_status.h`
- `src/alarm_pause_status.c`
- `tests/unit/test_alarm_pause_status.cpp`

Files that should not change for this issue:

- `src/vitals.c`
- `src/alerts.c`
- `src/patient.c`
- `src/news2.c`
- `src/alarm_limits.c`
- `src/gui_users.c`
- `src/gui_auth.c`
- `src/app_config.c`
- `include/app_config.h`
- `.github/workflows/**`

## Requirements and traceability impact

This is a GUI-display enhancement with safety-adjacent wording and reset
semantics. It should remain traceable as presentation behavior, not as a
change to alarm-generation logic.

Recommended requirements approach:

- keep `SYS-006`, `SYS-011`, and all alarm-threshold / NEWS2 requirements
  unchanged
- use `SYS-014` as the current approved system-level anchor if issue #64 has
  not yet merged
- add a dedicated GUI-level requirement for alarm-audio pause countdown
  behavior, visibility conditions, refresh cadence, and clear-on-expiry /
  clear-on-resume behavior

If issue #64 merges first and introduces a new system/software requirement pair
for alarm-audio state, this issue should extend that pair instead of creating a
competing independent requirement chain.

If issue #64 has not merged by implementation time, the implementer should:

- update `SYS-014` to mention display of temporary alarm-audio pause countdown
  within the dashboard status area
- add the next available `SWR-GUI-xxx` requirement for:
  - audio-only wording
  - visibility only while pause-active state is known and active
  - monotonic countdown formatting from authoritative expiry time
  - immediate clear on resume, expiry, logout, and invalidation

Traceability updates should map the chosen GUI requirement to the banner path
in `src/gui_main.c` and to the relevant verification evidence without touching
alert-classification or alarm-limit requirement rows.

## Medical-safety, security, and privacy impact

Medical-safety impact is low for direct function and medium for presentation
integrity. The change can improve operator awareness of a temporary mute window,
but a stale or overstated countdown could create false reassurance.

Safety constraints:

- countdown state must be derived from authoritative pause data
- wording must stay audio-specific unless the product behavior is explicitly
  broader
- paused presentation must be visually non-normal
- countdown must clear immediately on expiry or resume
- alarm generation, thresholds, and patient-severity logic must remain
  unchanged

Security impact is low if the feature stays display-only. No new permission
boundary, network path, or credential flow is needed.

Privacy impact is none expected. No new patient-data class, export surface, or
storage path is introduced.

## AI/ML impact assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: none
- Input data: none
- Output: none
- Human-in-the-loop limits: unchanged
- Transparency needs: unchanged beyond ordinary UI wording clarity
- Dataset and bias considerations: none
- Monitoring expectations: none beyond ordinary regression verification
- PCCP impact: none

## Validation plan

Primary verification expectations:

- verify the countdown appears only while authoritative pause-active state is
  known and active
- verify the displayed value decreases monotonically and is derived from the
  real expiry time rather than from sample count
- verify manual resume clears the banner immediately
- verify automatic expiry clears the banner exactly at zero without negative
  display
- verify logout, clear session, and monitoring invalidation remove stale paused
  state
- verify simulation pause/resume does not by itself alter alarm-pause countdown
  unless the authoritative pause source changes
- verify the paused banner stays visually distinct from normal-status
  presentation
- verify active alert severity presentation remains correct while audio is
  paused

Recommended verification commands during implementation:

```powershell
run_tests.bat
ctest --test-dir build --output-on-failure -R GUI|Config|Alarm|Patient
git diff --stat
rg -n "alarm audio paused|resumes in|SWR-GUI|SYS-014" src include requirements
```

Manual GUI scenarios should include at minimum:

- pause-start at a nonzero duration
- mid-countdown repaint behavior
- manual resume before expiry
- automatic expiry at zero
- pause-state invalidation on logout or session reset
- simulation pause toggled while alarm pause remains active

If implementation extracts a pure helper for formatting or state derivation,
add targeted unit tests for clamping, `MM:SS` formatting, and clear-on-invalid
behavior.

## Rollback or failure handling

If implementation cannot identify a trustworthy pause-active and pause-expiry
source, stop and split a prerequisite issue instead of fabricating the feature
from simulation state or a decrement-only UI timer.

If the final layout cannot present the countdown without conflicting with the
status banner's higher-priority mode messaging, prefer temporarily replacing
the rolling simulation message during pause-active state rather than widening
scope into a broader UI refactor.

If requirements work would collide with issue #64 numbering or merge order,
reuse the behavioral intent and assign the next available requirement IDs at
implementation time rather than forcing conflicting IDs into parallel branches.
