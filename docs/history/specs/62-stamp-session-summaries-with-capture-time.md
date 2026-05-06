# Design Spec: Stamp session summaries with capture time

Issue: #62
Branch: `feature/62-stamp-session-summaries-with-capture-time`
Spec path: `docs/history/specs/62-stamp-session-summaries-with-capture-time.md`

## Problem

Issue #62 identifies a provenance gap in the repo's existing patient-summary
output. `patient_print_summary()` prints demographics, BMI, latest vitals,
overall status, and active alerts, but the rendered artifact does not say when
that summary text was generated. A copied, pasted, screenshot, or printed
summary can therefore be mistaken for a live view or for a newer artifact than
it really is.

The gap is operational rather than algorithmic. Current code already keeps the
summary surface narrow and text-based:

- `src/patient.c` renders the formatted patient summary
- `src/main.c` reuses `patient_print_summary()` for each CLI end-of-session
  summary
- no existing summary path stores or displays provenance time metadata

The risk note on this branch already limits scope to metadata-only provenance.
The design must preserve that boundary and avoid introducing any new meaning
about physiologic measurement time, audit logging, or persistence.

## Goal

Add one explicit summary-generation timestamp line to the existing patient
summary output so operators can tell when a summary artifact was generated.

The intended outcome is:

- copied or printed summaries show an unambiguous render time
- the timestamp meaning is clearly about summary generation, not measurement
  capture
- the change stays inside the existing summary display path and does not alter
  clinical classification, alerting, NEWS2, storage, or authentication logic

## Non-goals

- No timestamping of individual vital-sign measurements.
- No new field in `PatientRecord`, `PatientInfo`, or `VitalSigns`.
- No persistence of summary times to disk, config, export files, audit logs, or
  EMR-facing artifacts.
- No change to `vitals.c`, `alerts.c`, `news2.c`, alarm limits, authentication,
  localization, or GUI dashboard behavior.
- No vendor-style report redesign, alarm annotation system, or broader summary
  export workflow.
- No locale-specific month/day names, timezone guessing, or ambiguous
  human-formatted date strings.

## Intended use impact, user population, operating environment, and foreseeable misuse

- Intended use impact: improve provenance clarity for session summary artifacts
  used during review, handoff, or retrospective discussion.
- User population: trained clinical users, testers, reviewers, and developers
  reading console or printed summary output.
- Operating environment: local workstation console output, copied terminal
  text, screenshots, or printed summaries generated from the workstation.
- Foreseeable misuse: a user may mistake the summary timestamp for the time the
  displayed vitals were measured, or may treat it as proof of current live
  state.
- Primary design control: label the field explicitly as summary-generation
  time and use a fixed UTC format so the artifact cannot imply hidden local or
  bedside timing semantics.

## Current behavior

Current summary behavior is centralized and bounded:

- `src/patient.c` prints the entire patient summary from
  `patient_print_summary(const PatientRecord *rec)`.
- The function calculates BMI, reads the latest vital signs, computes overall
  alert status, and prints active alerts inline.
- If no readings exist, the vitals and alerts sections are omitted but the
  summary shell still prints.
- `src/main.c` does not have a second end-of-session summary renderer; both CLI
  patient sessions already call `patient_print_summary()`.
- `tests/unit/test_patient.cpp` currently verifies only that
  `patient_print_summary()` does not crash for no-reading, normal, and critical
  cases. It does not assert on rendered content, timestamp format, or wording.

## Proposed change

Implement the feature as one additive provenance line inside the existing
summary renderer, with these decisions:

1. Keep the summary timestamp render-time only. Do not store it in any patient
   data structure or attach it to individual `VitalSigns` readings.
2. Add one private helper in `src/patient.c` that obtains the current time once
   per summary render, converts it to UTC, and formats it as ASCII
   `YYYY-MM-DD HH:MM:SS UTC`.
3. Print the new line as:

```text
  Summary generated at : 2026-05-06 05:12:36 UTC
```

4. Place the line after the existing `Readings:` line and before `Latest
   Vitals:` so it is visible as artifact metadata, not mixed into clinical
   values.
5. Render the line even when `reading_count == 0`. A summary with no readings
   is still a generated artifact and should still carry provenance metadata.
6. If clock acquisition or UTC conversion fails, print an explicit unavailable
   value such as:

```text
  Summary generated at : unavailable
```

   Do not fabricate a timestamp and do not omit the field silently.
7. Keep the public patient-summary API unchanged unless implementation proves a
   test seam is impossible without an interface extension. The preferred design
   is a private helper plus output-capture tests, not a new public timestamp
   parameter.
8. Treat `src/main.c` as an inspection target rather than a likely behavior
   change. Because the CLI summary already delegates to `patient_print_summary()`,
   the shared formatter can live entirely inside `src/patient.c`.
9. Upgrade the patient-summary tests from crash-only verification to content
   assertions that confirm:
   - the timestamp line exists
   - the label is exactly about summary generation
   - the rendered value matches the UTC format or the explicit unavailable
     fallback
   - existing vitals, overall-status, and active-alert text remain present
     and semantically unchanged

## Files expected to change

Expected files:

- `src/patient.c`
- `tests/unit/test_patient.cpp`

Expected files to inspect and modify only if the implementation needs comment
or contract clarification:

- `include/patient.h`
- `src/main.c`

Possible documentation files to update only if the team decides the additive
metadata should be made explicit in approved requirement wording:

- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`

Files that should not change:

- `src/vitals.c`
- `src/alerts.c`
- `src/news2.c`
- `src/gui_main.c`
- `src/gui_users.c`
- `src/app_config.c`
- `include/vitals.h`
- `tests/integration/**`
- `docs/history/risk/62-stamp-session-summaries-with-capture-time.md`

## Requirements and traceability impact

Primary traceability remains under the existing summary requirements:

- `SYS-011` Patient Status Summary Display
- `SWR-PAT-006` Patient Summary Display

No new UNS, SYS, or SWR identifier is required for the MVP. The feature is an
additive provenance line on an existing summary surface, not a new clinical or
storage behavior.

Implementation guidance:

- keep all existing required summary fields intact
- do not change alert ordering, BMI logic, overall status logic, or omitted
  sections for no-readings behavior
- if requirement wording is updated, keep the same traceability IDs and clarify
  only that the summary may include one explicitly labeled non-clinical
  provenance timestamp

No new `@req` tags, new RTM rows, or new verification categories should be
introduced solely for this feature.

## Medical-safety, security, and privacy impact

Medical-safety impact is low and indirect. The feature does not alter bedside
logic, threshold evaluation, aggregate alert status, NEWS2 scoring, or alarm
generation. The safety value is better stale-artifact recognition during review
or handoff.

The main residual safety risk is semantic confusion if the timestamp is read as
the time the vitals were measured. The design controls for that are:

- explicit label text: `Summary generated at`
- fixed UTC suffix in the rendered value
- no storage in vital-sign or patient-session structures
- no change to the clinical text around the latest vitals

Security impact is low. No new input surface, credential path, privilege rule,
network path, or persistence behavior is introduced.

Privacy impact is low. The summary already contains patient-identifying and
clinical information; the timestamp adds provenance detail but no new patient
data category.

## AI/ML impact assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: none
- Input data: none beyond existing non-AI summary content and system clock time
- Output: one deterministic provenance string in the summary
- Human-in-the-loop limits: unchanged
- Transparency needs: unchanged, aside from explicit labeling of the summary
  timestamp
- Dataset and bias considerations: none
- Monitoring expectations: no AI monitoring required
- PCCP impact: none

## Validation plan

Use targeted unit validation plus manual console confirmation.

Output-content unit tests:

- Replace the current crash-only `PatientPrintSummary.*` checks with stdout
  capture assertions.
- Verify the no-readings case still prints the summary shell, overall status,
  and the timestamp line.
- Verify normal and critical cases print the timestamp line plus the existing
  vitals/status/alerts content.
- Match the timestamp value against a fixed pattern such as
  `\\d{4}-\\d{2}-\\d{2} \\d{2}:\\d{2}:\\d{2} UTC`, with a separate test path or
  assertion allowance for the explicit `unavailable` fallback if the formatter
  exposes that branch.

Suggested verification commands:

```powershell
cmake -S . -B build -G Ninja -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure -R "Patient|GUI"
build/tests/test_unit.exe --gtest_filter=PatientPrintSummary.*
```

Manual console validation:

- Run the CLI demo and confirm both end-of-session summaries show the same
  `Summary generated at` label and UTC formatting.
- Confirm the timestamp appears exactly once per summary block and is visually
  separate from the latest-vitals section.

Diff-scope validation:

```powershell
git diff --stat
git diff -- src/patient.c tests/unit/test_patient.cpp include/patient.h src/main.c requirements/SYS.md requirements/SWR.md requirements/TRACEABILITY.md
```

The final implementation diff should remain summary-surface and test scoped. If
the change starts touching alert logic, NEWS2, persistence, or authentication,
stop and split the work.

## Rollback or failure handling

If implementation cannot acquire or format UTC time reliably within the current
static-memory and portability constraints, fail closed:

- keep clinical summary behavior unchanged
- print an explicit `unavailable` provenance value only if the failure path is
  well understood and testable
- otherwise revert the timestamp addition rather than shipping ambiguous or
  fabricated metadata

If requirement clarification proves necessary and expands beyond the bounded
summary scope, split that documentation update into the same issue only if it
stays under `SYS-011` and `SWR-PAT-006`; otherwise open a follow-up traceability
task.

If reviewers object to UTC specifically, the only acceptable fallback is a
still-unambiguous format that names the time basis explicitly. Do not switch to
bare locale-formatted local time.
