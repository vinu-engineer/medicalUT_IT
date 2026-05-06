# Design Spec: Render missing readings as explicit N/A

Issue: #66
Branch: `feature/66-render-missing-readings-as-explicit-n-a`
Spec path: `docs/history/specs/66-render-missing-readings-as-explicit-n-a.md`

## Problem

Issue #66 asks for intentionally missing measurements to be rendered as an
explicit `N/A` instead of a numeric-looking placeholder or a blank/omitted
field in the dashboard and summary views.

The current implementation already distinguishes one approved missing-data
state in logic: `respiration_rate == 0` means "not measured" and is excluded
from aggregate alert escalation and NEWS2 inflation. The presentation layer,
however, does not expose that meaning consistently:

- `src/gui_main.c` renders device mode as `N/A`, but renders the live RR tile
  as `--` when `respiration_rate == 0`.
- `src/gui_main.c` omits RR entirely from the history-list string when
  `respiration_rate == 0`.
- `src/patient.c` does not print a respiration-rate line in
  `patient_print_summary()`, so the latest-summary surface hides the fact that
  RR was intentionally not measured.

This creates a human-factors ambiguity in a clinician-facing path: the product
logic knows the value was not measured, but the UI/summary path does not state
that explicitly.

## Goal

Deliver a narrow, display-only implementation that shows approved missing
measurement states as explicit `N/A` in the scoped clinician-facing surfaces,
while preserving all existing clinical logic, storage semantics, and alert
behavior.

For this issue, the approved MVP target is the already-defined
`respiration_rate == 0` state in the live monitoring flow, plus preservation of
the already-existing device-mode `N/A` presentation.

## Non-goals

- No change to respiration-rate thresholds, alert generation, aggregate alert
  level, NEWS2 scoring, alarm limits, trend extraction, persistence, or data
  storage.
- No reinterpretation of `0`, blank, or invalid values for other vitals as
  "missing" unless a requirement explicitly defines that state.
- No broad redesign of dashboard layout, tile colors, badges, or summary
  format beyond the minimal wording needed to make the absence state explicit.
- No change to "no patient admitted" semantics unless implementation discovers
  an unavoidable ambiguity that requires a follow-up issue.
- No requirements-document rewrite in this issue unless implementation
  discovers that the approved scope cannot be implemented under existing
  requirement text.

## Intended use impact, user population, operating environment, and foreseeable misuse

Intended use remains bedside monitoring and review of vital signs for an active
patient on the existing Windows desktop application. The relevant users are
clinical staff and reviewers reading the live dashboard and summary output
during monitoring, handoff, or post-reading review.

Operating environment does not change. The behavior remains local to the Win32
GUI and console-style summary output already produced by the application.

Foreseeable misuse to guard against:

- interpreting `0`, `--`, or an omitted RR line as a valid measured value
- interpreting `N/A` as "normal" rather than "not measured"
- broadening `N/A` rendering to vitals whose missing-state semantics are not
  currently approved
- changing RR alerting or NEWS2 behavior under the label of a display-only fix

## Current behavior

Relevant current behavior is:

- `src/vitals.c` and `src/news2.c` already preserve the RR "not measured"
  sentinel in clinical logic.
- `src/gui_main.c` `paint_tile()` already knows not to append a unit when the
  rendered value is `N/A`.
- `src/gui_main.c` `paint_tiles()` sets all tiles to `N/A` in device mode, but
  uses `--` for RR when a patient exists and `respiration_rate == 0`.
- `src/gui_main.c` `paint_tiles()` also uses `--` placeholders when no patient
  reading exists yet.
- `src/gui_main.c` `update_dashboard()` suppresses RR from the history string
  when `respiration_rate == 0`.
- `src/patient.c` `patient_print_summary()` prints HR, BP, temperature, and
  SpO2, but no RR line at all.
- Existing automated tests cover RR sentinel handling in alert-level and NEWS2
  logic, but the summary test only verifies that `patient_print_summary()`
  does not crash. There is no automated assertion on the rendered text.

## Proposed change

Implement the issue as a narrow presentation-policy change with these design
decisions.

### 1. Scope the explicit `N/A` rule to already-approved absence states

In-scope absence states:

- device mode / no live data state that already renders `N/A`
- live patient RR where `respiration_rate == 0` means "not measured"

Out of scope for this issue:

- inventing missing-value semantics for HR, BP, temperature, or SpO2
- converting invalid values into `N/A`
- using `N/A` for "no patient admitted" unless the implementation cannot keep
  that state distinct without material UX confusion

### 2. Make the RR tile explicit when RR is not measured

When a patient exists and the latest reading has `respiration_rate == 0`,
render the RESP RATE tile value as `N/A` instead of `--`.

The tile should continue to use existing alert-level behavior:

- no RR alert generated from the missing value
- no aggregate alert escalation from the missing value
- no NEWS2 score inflation from the missing value

This preserves the existing clinical state while making the presentation
explicit.

### 3. Add RR to `patient_print_summary()` and render the missing state explicitly

`patient_print_summary()` should print a respiration-rate line in the latest
vitals block so the summary surface does not hide the missing measurement.

Expected summary behavior:

- if `respiration_rate > 0`, print the numeric RR value and RR classification
- if `respiration_rate == 0`, print `Resp Rate   : N/A` and a clear
  non-clinical annotation such as `[NOT MEASURED]` or equivalent plain text
  that does not imply a numeric reading

The exact annotation may be chosen during implementation, but it must be
obviously non-numeric and must not claim the value is clinically normal.

### 4. Keep dashboard-adjacent consistency narrow and evidence-based

The primary required surfaces are:

- live RESP RATE tile in `src/gui_main.c`
- patient summary output in `src/patient.c`

`src/gui_main.c` history-list strings should be inspected during implementation
for contradiction risk. If leaving the current RR omission would create
inconsistent review evidence against the new tile/summary behavior, align the
history string within the same issue by rendering RR as `N/A` there too.

If the history-list update would broaden scope beyond a small string-format
change, document the follow-up gap instead of expanding into a larger GUI
rewrite.

### 5. Keep the change local to presentation code

Preferred implementation shape:

- keep the absence-to-display mapping inside presentation/summary code
- use a tiny local helper or constant if it reduces duplicate `N/A` handling
- avoid adding a new cross-layer API solely for display formatting

This keeps the change in `src/gui_main.c` and `src/patient.c` rather than
pulling a UI wording rule into domain logic.

## Files expected to change

Expected implementation files:

- `src/gui_main.c`
- `src/patient.c`
- `tests/unit/test_patient.cpp`

Files to inspect and modify only if needed to avoid contradictory output on the
same scoped behavior:

- `README.md`

Files to inspect but normally not modify:

- `src/vitals.c`
- `src/alerts.c`
- `src/news2.c`
- `tests/unit/test_vitals.cpp`
- `tests/unit/test_news2.cpp`
- `docs/history/risk/66-render-missing-readings-as-explicit-n-a.md`

Files that should not change:

- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`
- alarm-limit, trend, auth, persistence, CI, and release files

## Requirements and traceability impact

No approved UNS, SYS, or SWR statement is expected to change for the MVP
implementation if the scope stays within already-approved RR absence semantics
and existing device-mode `N/A` behavior.

Affected traceability context to preserve:

- `SYS-011` patient status summary display
- `SYS-014` graphical dashboard display
- `SYS-018` RR "not measured" preservation without spurious alert escalation
- `SYS-019` RR missing state must not inflate NEWS2
- `SWR-GUI-003` color-coded tile display
- `SWR-PAT-006` patient summary display
- `SWR-VIT-008` RR missing sentinel handling
- `SWR-NEW-001` RR missing NEWS2 handling

No `@req` tags, requirement IDs, or RTM rows should change in this issue.

If the product owner wants the exact string `N/A` to become a normative
requirement for one or more surfaces, that should be handled as a follow-up
requirements change rather than folded silently into this implementation issue.

## Medical-safety, security, and privacy impact

Medical-safety impact is low and display-only if the boundary is respected. The
intended safety benefit is reduced ambiguity for clinicians and reviewers when a
measurement was intentionally not captured.

Primary safety constraint: `N/A` must represent only approved absence states,
not invalid or abnormal numeric measurements. The implementation must not hide
critical or malformed data behind an absence label.

Security impact is none expected. No authentication, authorization, network,
logging, or persistence path changes.

Privacy impact is none expected. No patient-data scope or storage/export path
changes.

## AI/ML impact assessment

This issue does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: none
- Input data: none beyond existing non-AI vital-sign display data
- Output: none
- Human-in-the-loop limits: unchanged
- Transparency needs: unchanged
- Dataset and bias considerations: not applicable
- Monitoring expectations: unchanged
- PCCP impact: none

## Validation plan

Validation should prove that presentation changed while clinical logic did not.

Targeted regression checks:

```powershell
ctest --test-dir build --output-on-failure -R "Vitals|Patient|News2"
```

Patient-summary verification should be strengthened beyond the current
no-crash test. Preferred approach:

- update `tests/unit/test_patient.cpp` to capture `stdout` and assert that:
  - RR is printed numerically when present
  - RR renders as `N/A` (and, if chosen, the not-measured annotation) when
    `respiration_rate == 0`

Manual GUI review:

- admit a patient and add a reading with `respiration_rate = 0`
- confirm the RESP RATE tile shows `N/A`
- confirm units are not appended to `N/A`
- confirm alert banner and active-alert list remain unchanged relative to the
  existing RR-missing logic
- toggle device mode and confirm the existing all-tiles `N/A` behavior still
  works
- verify no-patient state remains intentionally distinct if left unchanged

Diff-scope review:

```powershell
git diff --stat
git diff -- src/gui_main.c src/patient.c tests/unit/test_patient.cpp
```

If implementation touches RR thresholds, alert generation, NEWS2 scoring,
alarm-limit logic, or requirements documents, stop and return the issue for
design review.

## Rollback or failure handling

If implementation discovers that the summary format cannot be updated without a
larger output-contract discussion, keep the issue scoped to the explicit design
comment and return it for follow-up rather than guessing at downstream
consumers.

If GUI handling of `respiration_rate == 0` requires changes outside small
string-format logic, stop and re-evaluate scope before modifying broader
message-loop or rendering code.

If reviewers reject the exact annotation chosen for summary output, rollback is
presentation-only: revise the wording while preserving the explicit absence
state and the unchanged clinical logic.
