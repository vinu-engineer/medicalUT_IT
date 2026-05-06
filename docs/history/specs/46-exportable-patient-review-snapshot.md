# Design Spec: Add exportable patient review snapshot

Issue: #46
Branch: `feature/46-exportable-patient-review-snapshot`
Spec path: `docs/history/specs/46-exportable-patient-review-snapshot.md`

## Problem

The product currently supports two ways to review patient state:

- the live Win32 dashboard in `src/gui_main.c`
- the demo console summary emitted by `patient_print_summary()` in `src/patient.c`

Neither path produces a user-initiated, durable review artifact for handoff or
retrospective review. Operators can see the current patient summary, sparkline
trends, and active alerts, but they cannot export a compact local snapshot
without manual transcription.

The current codebase already contains almost all of the read-only data needed
for a narrow MVP:

- `PatientRecord` stores demographics and up to 10 sequential readings
- `patient_latest_reading()` returns the active snapshot source
- `patient_current_status()` and `generate_alerts()` already derive aggregate
  status and active alerts deterministically
- `trend_extract_*()` and `trend_direction()` already derive bounded direction
  labels used by the GUI sparkline tiles

The gap is an export workflow, not clinical logic.

This gap has two important constraints:

- The export must stay inside the current authentication boundary. The existing
  `patient_monitor` CLI demo in `src/main.c` is not login-gated, so adding a
  patient-data export flag there would widen privacy risk and bypass the
  intended Windows GUI workflow.
- The repository does not currently store authoritative timestamps or audit-log
  semantics for readings. A snapshot export must therefore avoid implying that
  it is a complete, current, or official medical record.

## Goal

Add a narrow, user-initiated, plain-text export from the authenticated Win32
dashboard that writes a deterministic patient review snapshot for the active
local session.

The MVP snapshot shall:

- include patient identity and session context
- include the latest vital values with current classifications
- include aggregate alert status and active alert text
- include bounded trend direction derived from the existing trend helpers
- save only to an explicit user-chosen local path
- provide explicit success or failure feedback

## Non-goals

- No change to acquisition, thresholding, alert generation, NEWS2 scoring, or
  any other clinical logic.
- No CLI export path in this issue. Export remains GUI-only because the current
  CLI path is not authenticated.
- No PDF generation, print workflow, HL7 export, EMR integration, network sync,
  cloud upload, background auto-save, or scheduled export.
- No authoritative timestamps, event chronology, or claims that the artifact is
  a complete chart, audit log, or official medical record.
- No new trend algorithm. The design reuses existing `trend_direction()`
  behavior and adds only gating to avoid misleading output when data are absent.
- No localization of the exported file body in this MVP. The artifact content
  stays fixed and deterministic; only the GUI control and result messages follow
  the current UI localization mechanism.

## Intended use impact, user population, operating environment, and foreseeable misuse

Intended use impact:

- Extends the product from live on-screen review to a constrained local handoff
  or retrospective-review artifact.
- Does not add diagnosis, prediction, or treatment recommendation behavior.

User population:

- authenticated clinical users and administrators already permitted to use the
  monitoring dashboard

Operating environment:

- local Windows workstation
- active authenticated GUI session
- active patient record in memory
- user-selected local filesystem destination

Foreseeable misuse:

- treating the exported snapshot as a complete or current clinical record
- exporting the wrong patient after a refresh or before confirming the active
  patient identity
- over-interpreting textual trend direction as a prediction or clinical advice
- exporting when only one reading exists and assuming the trend is meaningful
- saving the file into an insecure shared location

## Current behavior

Current review behavior is split across existing modules:

- `src/gui_main.c` paints the current dashboard from `g_app.patient`, updates
  trend sparklines from `trend_extract_*()`, and clears session data on
  `IDC_BTN_LOGOUT` and `do_clear()`.
- `src/patient.c` provides `patient_print_summary()`, which writes a console
  summary to `stdout`, but it does not create a file or operate behind GUI
  authentication.
- `src/alerts.c` emits deterministic active-alert records in a fixed parameter
  order.
- `src/trend.c` exposes direction helpers for heart rate, systolic blood
  pressure, temperature, SpO2, and respiration rate.

There is no current implementation for:

- an export button or menu action in the dashboard
- a save-file dialog or explicit export destination workflow
- a deterministic plain-text review artifact
- a formal requirement chain for exportable patient review snapshots

There is also a domain nuance that matters for design:

- `respiration_rate == 0` means "not measured"
- `trend_extract_rr()` currently copies those `0` values through for sparkline
  display
- exporting a textual RR trend from raw zero-filled history would risk
  overstating meaning when RR was not measured

## Proposed change

Implement the feature as a GUI-only export flow backed by a shared,
non-allocating formatting helper.

### 1. Add a shared snapshot formatter module

Add a new formatter module in the shared library:

- `include/review_snapshot.h`
- `src/review_snapshot.c`

This module should stay OS-agnostic and should not open files directly. It
should expose a small C API such as:

```c
int review_snapshot_format(const PatientRecord *rec, char *out, size_t out_sz);
```

Behavior:

- use only caller-provided buffers; no heap allocation
- return failure when `rec == NULL`, `out == NULL`, `out_sz` is too small, or
  `rec->reading_count == 0`
- build a deterministic plain-text artifact in a fixed section order
- reuse existing read-only helpers instead of duplicating clinical logic:
  `calculate_bmi()`, `bmi_category()`, `patient_latest_reading()`,
  `patient_current_status()`, `generate_alerts()`, `trend_extract_*()`,
  `trend_direction()`, and `alert_level_str()`

This keeps the data derivation testable in unit tests without pulling Win32
file-dialog or message-box behavior into the domain-adjacent formatter.

### 2. Export only from the authenticated dashboard

Add a new dashboard action in `src/gui_main.c`, for example `IDC_BTN_EXPORT`,
visible after login alongside other monitoring actions.

Expected workflow:

1. User logs in and reaches the dashboard.
2. Export control is disabled unless:
   - `g_app.has_patient != 0`
   - `g_app.patient.reading_count > 0`
3. User activates export.
4. GUI opens a standard Save As dialog restricted to plain-text output
   (for example `*.txt`) with a deterministic default filename such as
   `patient-<id>-review-snapshot.txt`.
5. On confirmation, GUI calls `review_snapshot_format()` into a fixed local
   buffer, then writes that buffer to disk using a restricted local file-open
   path.
6. GUI shows explicit success or failure feedback.
7. Canceling the save dialog is a no-op, not an error.

The control shall be available to both `ROLE_ADMIN` and `ROLE_CLINICAL`
sessions because both roles already have monitoring access under `SYS-017`.

Export availability shall be tied to patient-session state, not simulation
state. A manually entered patient with at least one reading remains exportable
even when simulation is paused or disabled.

### 3. Keep file-write enforcement in the GUI layer

The GUI layer should own path selection and file creation. It should not use a
plain `fopen()` write that weakens local privacy controls.

Instead, the GUI export path should follow the same restricted-write posture
already used by `src/gui_users.c`:

- owner read/write only where supported
- deny concurrent access while the file is open
- verify full write completion
- if a short write or flush failure occurs, report failure and best-effort
  remove the partial file rather than leaving a silent truncated artifact

This keeps the shared formatter cross-platform and deterministic while making
the actual patient-data file write explicit and reviewable in the Windows GUI.

### 4. Snapshot content contract

The artifact should be a compact plain-text document with fixed English labels
and a clear scope disclaimer near the top. Proposed content:

```text
PATIENT REVIEW SNAPSHOT
Local session only. Not a complete medical record.

Patient Name   : Sarah Johnson
Patient ID     : 1001
Age            : 52 years
BMI            : 26.3 (Overweight)
Readings       : 3 / 10
Overall Status : WARNING

Latest Vitals
- Heart Rate       : 108 bpm [WARNING]      Trend: RISING
- Blood Pressure   : 148/94 mmHg [WARNING]  Trend: RISING (systolic)
- Temperature      : 37.9 C [WARNING]       Trend: RISING
- SpO2             : 93 % [WARNING]         Trend: FALLING
- Resp Rate        : 23 br/min [WARNING]    Trend: RISING

Active Alerts
- Heart rate 108 bpm [normal 60-100]
- BP 148/94 mmHg [normal 90-140 / 60-90]
- Temp 37.9 C [normal 36.1-37.2]
- SpO2 93% [normal 95-100%]
- RR 23 br/min [normal 12-20]
```

Required content rules:

- Include a top-of-file statement that the artifact is a local session snapshot
  and not a complete medical record.
- Include patient identity, BMI, reading count, and aggregate alert status.
- Include latest vital values and their current classifications.
- Include active alerts in the deterministic order already produced by
  `generate_alerts()`.
- Report blood-pressure trend explicitly as systolic-based so the export does
  not imply diastolic trend analysis that the current code does not perform.
- Do not include timestamps, duration claims, event chronology, or external
  system identifiers.

### 5. Trend-direction rules for export text

Trend wording must stay narrower than the graphical sparkline presentation.

For heart rate, systolic BP, temperature, and SpO2:

- use `trend_extract_*()` plus `trend_direction()`
- if fewer than 2 readings exist, export `Trend: INSUFFICIENT DATA`

For respiration rate:

- first extract the RR history
- treat `respiration_rate == 0` as "not measured", not as a real value for
  textual trend interpretation
- export `Trend: INSUFFICIENT DATA` unless at least two measured RR values are
  available for trend analysis
- if the latest RR itself is `0`, print `Resp Rate : not measured` and do not
  emit a misleading rising/falling/stable label

This preserves the existing trend algorithm while preventing a local
"not measured" sentinel from becoming a misleading textual narrative.

### 6. Requirements and traceability update plan

This feature needs a new requirements chain rather than overloading the current
summary-display requirement.

Proposed new user need:

- `UNS-017` - Local patient review snapshot export

Proposed new system requirements:

- `SYS-020` - Authenticated local snapshot export
- `SYS-021` - Snapshot content and gating
- `SYS-022` - Snapshot write feedback and local-only boundary

Proposed new software requirements:

- `SWR-EXP-001` - Deterministic snapshot formatting
- `SWR-EXP-002` - Trend wording and missing-data handling
- `SWR-EXP-003` - Buffer-bounded, no-heap formatter failure behavior
- `SWR-GUI-013` - Dashboard export action, Save As flow, and explicit write
  outcome feedback

Expected RTM totals after implementation:

- User needs: 17
- System requirements: 22
- Software requirements: 41

`requirements/README.md` should also be updated to document the new `SWR-EXP`
module prefix and maintenance expectations for this export path.

## Files expected to change

Requirements and traceability:

- `requirements/UNS.md`
- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`
- `requirements/README.md`

Production code:

- `include/review_snapshot.h`
- `src/review_snapshot.c`
- `src/gui_main.c`
- `include/localization.h`
- `src/localization.c`
- `CMakeLists.txt`

Verification and validation:

- `tests/unit/test_review_snapshot.cpp`
- `tests/unit/test_localization.cpp`
- `dvt/DVT_Protocol.md`
- any DVT script or checklist file used to record the GUI export scenario

Files that should not change:

- `src/vitals.c`
- `src/alerts.c`
- `src/patient.c`
- `src/trend.c`
- `src/news2.c`
- authentication, RBAC, CI, release, or hardware-acquisition code

If implementation discovers that any of those clinical modules must change to
support this MVP, stop and return the issue to design review first.

## Requirements and traceability impact

This feature affects:

- patient identification (`UNS-008`)
- within-session history review (`UNS-009`)
- consolidated review workflow (`UNS-010`)
- data integrity and failure handling (`UNS-011`)
- authentication boundary (`UNS-013`)
- GUI monitoring workflow (`UNS-014`)

But the core export need is distinct enough to justify a new `UNS-017`.

The existing `SYS-011` / `SWR-PAT-006` summary chain should remain in place for
console and display summary behavior. The export artifact should not be merged
into `SWR-PAT-006`, because that would blur the boundary between:

- an internal stdout summary helper
- a user-facing persisted artifact with privacy and failure-handling controls

## Medical-safety, security, and privacy impact

Medical safety:

- No clinical threshold or scoring behavior changes.
- The main safety risk is misinterpretation of the artifact as complete, live,
  or authoritative.
- Exported trend text must remain descriptive only and must not imply
  prediction, recommendation, or audit-grade chronology.

Security:

- Export remains within the existing authenticated dashboard path.
- No new remote surface, service, API, or background export channel is added.
- Both user roles may export because both are already permitted to monitor
  patient data; this issue does not add a new privileged action class.

Privacy:

- The artifact contains patient identifiers and physiological data, so local
  file creation is itself the primary privacy risk.
- The implementation must require explicit user path selection and must use a
  restricted-write posture consistent with local patient-data handling.
- No hidden auto-save or remembered export directory should be introduced in
  this MVP.

## AI/ML impact assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- No model is introduced.
- No training or inference data path changes.
- No human-in-the-loop AI decision support is added.
- No dataset, bias, monitoring, or PCCP impact applies.

The export is derived only from existing deterministic rule-based summary,
alert, and trend helpers.

## Validation plan

### Automated

Add targeted unit coverage for `review_snapshot_format()`:

- no-readings input returns failure
- normal patient snapshot contains demographics, reading count, and overall
  status in fixed section order
- abnormal patient snapshot contains active alerts in deterministic order
- rising, falling, and stable trend wording for HR, systolic BP, temperature,
  and SpO2
- fewer than two readings produces `INSUFFICIENT DATA`
- RR not measured (`0`) does not produce a misleading RR trend
- output remains deterministic across repeated calls with the same input
- output fails cleanly when the destination buffer is too small

Suggested automated command set after implementation:

```powershell
cmake -S . -B build -G Ninja -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure -R Snapshot|Patient|Trend|Localization
```

### Manual / DVT

Add a GUI verification scenario that confirms:

- export button is unavailable until a patient exists with at least one reading
- authenticated admin and clinical users can both export
- Save As dialog opens with text-file filter and deterministic default name
- successful export creates a readable local text file with the expected
  disclaimer and fields
- canceling the dialog does not mutate session state
- invalid or inaccessible paths produce an explicit failure message
- logout and clear-session paths disable export again

If the project keeps GUI verification in DVT rather than unit automation, add a
new DVT step and record it as the approved evidence for `SWR-GUI-013`.

## Rollback or failure handling

Runtime failure handling:

- If formatting fails, do not create a file and show an explicit export-failed
  message.
- If file creation or write fails, report the failure and best-effort delete any
  partial output.
- Session state must remain unchanged after any export failure or dialog cancel.

Scope failure handling:

- If implementation pressure expands toward timestamps, chronology, formal
  record semantics, print/PDF workflow, CLI export, or network transfer, stop
  and split that work into a new design issue instead of broadening this MVP.
- If the human owner decides the artifact must behave as an official medical
  record, return this issue to `ready-for-design`; that requirement set is
  materially broader than the constrained snapshot approved by the current risk
  note.
