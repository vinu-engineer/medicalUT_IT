# Design Spec: Issue 82

Issue: `#82`  
Branch: `feature/82-copy-current-patient-review-summary-to-clipboard`  
Spec path: `docs/history/specs/82-copy-current-patient-review-summary-to-clipboard.md`

## Problem

The current repository already has a deterministic patient review summary, but
it is only available as console output:

- `src/patient.c` `patient_print_summary()` prints the current patient summary
  to `stdout`.
- `src/gui_main.c` shows the same patient state across `IDC_LIST_ALERTS`,
  `IDC_LIST_EVENTS`, and `IDC_LIST_HISTORY`, but it does not expose a single
  copyable plain-text handoff artifact.
- The dashboard has no `Copy Summary` action, no shared summary-text builder,
  and no clipboard handling path.

As a result, clinicians, testers, and reviewers who want to paste the current
session summary into a local note or ticket must retype data that the product
already formats deterministically. The risk note for `#82` also makes clear
that clipboard export is a privacy boundary and must stay manual, narrow, and
explicitly separated from durable export behavior.

## Goal

Add a bounded, manual, plain-text clipboard action that:

- copies the current patient session review summary without changing any
  clinical calculations or alert semantics
- reuses one validated summary-text source rather than creating a second,
  diverging summary definition
- keeps summary generation fixed-buffer and deterministic
- gives visible success/failure feedback
- stays local to the Win32 presentation layer with no file export, network
  transmission, or background copy behavior

## Non-goals

- Adding CSV, PDF, printer, file-export, email, chat, EMR, HL7, or network
  delivery behavior.
- Adding automatic copy on timer tick, alert change, session reset, logout, or
  patient clear.
- Adding keyboard accelerators or hotkeys in this MVP. A button-only workflow
  is sufficient for the first implementation.
- Adding redacted-copy, role-based copy variants, or user-configurable summary
  templates.
- Changing vital-sign thresholds, NEWS2 logic, alarm-limit logic,
  authentication, or session-retention rules.
- Treating the copied text as an official record, durable report, or complete
  longitudinal history.

## Intended Use Impact, User Population, Operating Environment, And Foreseeable Misuse

Intended use impact:

- The feature adds a manual note-assist and handoff adjunct for the active
  patient session.
- It does not replace the live dashboard, active-alert list, or clinician
  judgment.

User population:

- Authenticated clinicians, internal testers, and reviewers using the local
  Windows desktop application.

Operating environment:

- The existing single-patient Win32 workflow, with simulator-fed or manually
  entered readings, static in-process patient/session state, and no existing
  app-managed export pipeline.

Foreseeable misuse:

- Pasting one patient's copied summary into the wrong note, ticket, or patient
  context.
- Treating the copied text as live state after additional readings or a session
  reset.
- Misreading `Session Alarm Events` as currently active alerts if the copied
  sections are not clearly labeled.
- Moving identifiable patient text into unmanaged destinations through OS
  clipboard history, remote-desktop redirection, or clipboard sync features.

## Current Behavior

- `PatientRecord` already stores the demographic data, bounded vital-sign
  history, and bounded session alarm event review data needed for the summary.
- `patient_print_summary()` prints a deterministic English summary containing:
  patient demographics, BMI, reading count, latest vitals, overall status,
  active alerts, and session alarm events.
- `patient_print_summary()` has no caller-supplied buffer API; it writes only
  to `stdout`.
- `update_dashboard()` rebuilds the live GUI surfaces from the current patient
  record, but it does not provide a combined plain-text artifact for copying.
- `src/gui_main.c` currently defines header actions for logout, pause, and
  settings, plus the existing patient/session controls. There is no clipboard
  command path or copy-specific feedback path.
- `src/localization.c` localizes dashboard labels and button captions, but the
  console summary output is not localized today.

## Proposed Change

1. Add one shared patient-summary text formatter in the patient module that
   writes into a caller-supplied buffer and returns explicit success/failure
   status.
2. Refactor `patient_print_summary()` to delegate to that shared formatter so
   the clipboard payload and console output cannot drift apart.
3. Keep the formatter fixed-buffer and deterministic:
   - buffer ownership remains with the caller
   - size should be derived from existing maxima (`MAX_ALERTS`,
     `ALERT_MSG_LEN`, `MAX_ALERT_EVENTS`, `ALERT_EVENT_SUMMARY_LEN`) rather
     than an unbounded string-building path
   - overflow must return failure rather than silently truncating copied text
4. Expose the formatter through `include/patient.h` so the Win32 GUI can reuse
   it directly.
5. Add a dashboard `Copy Summary` button in the header control row and update
   all related layout/recreation paths:
   - initial `WM_CREATE`
   - right-edge `reposition_dash_controls()`
   - language-refresh reconstruction in `refresh_dash_language()`
6. Keep the MVP to a button-only workflow:
   - no accelerator table
   - no hidden auto-copy side effects
   - no copy action outside the authenticated dashboard
7. Enable the copy action only when a patient session exists. If a patient is
   admitted but has zero readings, copying is still allowed and should produce
   the same no-readings summary already emitted by `patient_print_summary()`.
8. Preserve exact content parity with the existing summary sections:
   - patient demographics
   - BMI and category
   - reading count
   - latest vitals when present
   - overall status
   - active alerts
   - session alarm events, including any reset disclosure
9. Preserve the explicit distinction between current `Active Alerts` and
   historical `Session Alarm Events`. The copied text must not collapse those
   sections into an ambiguous alarm block.
10. Keep the clipboard action plain text only and session-scoped only. No
    attachment, rich text, file artifact, or background persistence is allowed.
11. Provide visible success/failure feedback. A lightweight inline status is
    preferred if it fits the existing dashboard cleanly; a concise modal
    confirmation is acceptable for the MVP if a non-modal surface would expand
    scope materially.
12. If the copy attempt fails, the application must not report success. When
    possible, failure handling should leave the existing clipboard contents
    untouched.
13. For this MVP, copied content should match the current summary content,
    including identifiers already present in `patient_print_summary()`. If the
    product owner wants redaction, role-based suppression, or a second
    clipboard variant, that is follow-on scope and should not be folded into
    this issue.
14. Keep localization scope narrow:
    - the new dashboard button label and any user-facing copy feedback strings
      should be localizable through the existing localization layer
    - the copied summary payload should remain aligned with the current
      `patient_print_summary()` text for MVP parity rather than introducing a
      new localized summary format
15. Clipboard transport must stay clearly inside the Win32 presentation layer.
    The preferred implementation is a Win32-owned copy surface (for example, a
    hidden/off-screen multiline edit control using `WM_COPY`) so summary
    generation remains fixed-buffer and the app does not introduce a second
    manual heap-management path.
16. If implementation instead requires direct clipboard APIs such as
    `OpenClipboard()` and `SetClipboardData()`, document that raw text formats
    normally require Win32-owned transfer memory and treat that as an explicit
    presentation-layer interoperability exception. Do not let that exception
    leak into the patient/domain summary builder.

## Files Expected To Change

Expected requirements and traceability files:

- `requirements/UNS.md`
- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`

Expected implementation files:

- `include/patient.h`
- `src/patient.c`
- `include/localization.h`
- `src/localization.c`
- `src/gui_main.c`

Expected verification files:

- `tests/unit/test_patient.cpp`
- `tests/unit/test_localization.cpp`
- `dvt/DVT_Protocol.md`

Files expected not to change:

- `src/vitals.c`
- `src/alerts.c`
- `src/news2.c`
- `src/alarm_limits.c`
- authentication and account-management behavior outside existing session
  boundaries

## Requirements And Traceability Impact

- The current approved requirements define summary display and session review,
  but they do not explicitly authorize clipboard export or copy feedback.
- A defensible requirement update is expected, not just a code-only change.

Adjacent existing requirements:

- `UNS-010` Consolidated Status Summary
- `UNS-017` Session Alarm Event Review
- `SYS-011` Patient Status Summary Display
- `SYS-021` Session Alarm Event Review Presentation
- `SYS-013` User Authentication Enforcement
- `SYS-012` Static Memory Allocation
- `SWR-PAT-006` Patient Summary Display
- `SWR-GUI-003` Colour-Coded Vital Signs Display
- `SWR-GUI-013` Session Alarm Event Review List

Likely new or revised requirement content:

- a user-visible need for manually reusing the current patient summary in a
  local handoff/note workflow, if the team wants this behavior elevated at UNS
  level rather than treated only as a summary refinement
- a SYS-level requirement for explicit user-invoked copy of the current
  patient-session summary to the local clipboard
- SYS/SWR language for deterministic payload parity with the approved summary
  content
- SYS/SWR language for visible success/failure feedback
- SYS/SWR language prohibiting automatic copy, hidden persistence, and silent
  truncation
- SWR-level definition of the shared summary formatter and button enable/disable
  behavior

Traceability expectations:

- trace the copied payload back to the same summary content definition used by
  `patient_print_summary()`
- distinguish new clipboard-export behavior from existing display-only summary
  requirements
- do not claim new clinical inference, alerting, or record-retention behavior

## Medical-Safety, Security, And Privacy Impact

Medical safety:

- No intended change to thresholds, NEWS2 scoring, alert generation, alarm
  limits, or treatment guidance.
- Primary safety benefit is lower transcription burden for manual handoff or
  note capture.
- Primary safety risks are wrong-patient paste, stale copied snapshots, and
  confusion between current active alerts and historical event rows.
- The design remains acceptable only if the live dashboard stays primary and
  the copied text remains clearly sectioned and obviously session-scoped.

Security:

- No new authentication or authorization model is introduced.
- The feature must remain inside the authenticated dashboard and must not copy
  automatically after logout, clear, or timer events.

Privacy:

- Clipboard handling is the dominant risk because copied text leaves the
  application boundary and may persist in OS clipboard history or sync features.
- The issue must not add app-managed file persistence, background caching, or
  network delivery.
- User feedback should avoid implying that the app can revoke or control copied
  data after it leaves the process boundary.

## AI/ML Impact Assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: not applicable
- Input data: existing deterministic patient/session data only
- Output: deterministic plain-text summary only
- Human-in-the-loop limits: unchanged from the current system
- Transparency needs: clear section labeling for active alerts versus session
  history, but no AI explainability concerns
- Dataset and bias considerations: not applicable
- Monitoring expectations: standard software verification and GUI validation
  only
- PCCP impact: none

## Validation Plan

Automated validation should focus on the shared summary formatter rather than
trying to unit-test the Win32 clipboard stack directly.

Recommended automated coverage:

- `tests/unit/test_patient.cpp`
  - exact or substring validation that the shared formatter emits the same
    sections currently expected from `patient_print_summary()`
  - no-readings summary path
  - normal and critical latest-reading paths
  - session alarm events and session-reset notice inclusion
  - buffer-too-small or overflow failure path with no silent truncation
- `tests/unit/test_localization.cpp`
  - availability of the new button label and any new copy-feedback strings

Recommended manual validation:

- admit a patient with no readings and confirm the copy action is enabled only
  once a patient exists
- copy a normal summary and paste into Notepad or another plain-text target
- copy a summary after a warning/critical transition and confirm that `Active
  Alerts` and `Session Alarm Events` remain distinct in the pasted text
- confirm the UI reports failure accurately if the clipboard path cannot
  complete
- confirm no new file, network, or background export behavior exists

Recommended validation commands:

```powershell
run_tests.bat
ctest --test-dir build --output-on-failure -R "Patient|Localization"
```

## Rollback Or Failure Handling

- If implementation cannot keep summary generation fixed-buffer and
  deterministic, stop and re-scope before merging.
- If the only reliable clipboard path requires a presentation-layer memory
  exception that the team is unwilling to document, block the feature rather
  than hiding the exception.
- If requirements owners reject identifier-inclusive copy behavior or require
  redaction/role-based variants, split that policy work into a follow-on issue
  rather than overloading this MVP.
- If the UI cannot provide unambiguous success/failure feedback without a
  broader dashboard redesign, prefer the simplest explicit confirmation path
  over a silent or ambiguous copy action.
- Rollback is straightforward because the intended runtime change is additive:
  remove the shared formatter API and dashboard copy command, and keep the
  existing display-only summary behavior.
