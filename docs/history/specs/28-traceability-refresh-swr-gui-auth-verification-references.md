# Design Spec: Issue 28

Issue: `#28`  
Branch: `feature/28-traceability-refresh-swr-gui-auth-verification-references`  
Spec path: `docs/history/specs/28-traceability-refresh-swr-gui-auth-verification-references.md`

## Problem

`requirements/SWR.md` still cites stale or unsupported verification evidence for
three GUI/authentication requirements:

- `SWR-GUI-001` still names `AuthValidation.*`, but the approved auth unit tests
  in `tests/unit/test_auth.cpp` use the `UsersTest.REQ_GUI_001_*` pattern.
- `SWR-GUI-002` still names `AuthDisplayName.*`, but the approved auth unit
  tests use `UsersTest.REQ_GUI_002_*`.
- `SWR-GUI-003` claims structural verification by `AuthValidation.*`, even
  though those auth tests do not exercise GUI tile painting or banner repaint
  behavior.

`requirements/TRACEABILITY.md` and `tests/unit/test_auth.cpp` already reflect
the current evidence names, so the mismatch is in the SWR document rather than
in product code or test implementation. That weakens audit confidence and
change-impact review for authentication and GUI evidence.

## Goal

Reconcile the SWR verification text for `SWR-GUI-001`, `SWR-GUI-002`, and
`SWR-GUI-003` so it matches the approved evidence already present in
`requirements/TRACEABILITY.md` and `tests/unit/test_auth.cpp`, without changing
runtime behavior.

## Non-goals

- Changing authentication logic, credential handling, session management, or
  logout behavior.
- Changing GUI painting, color mapping, repaint triggers, or dashboard layout.
- Adding new automated GUI tests, new requirement IDs, or new traceability
  claims beyond the existing approved evidence.
- Editing `src/**`, `include/**`, `tests/**`, CI workflows, or release
  artifacts for this issue.
- Reworking SYS-level intent for `SYS-013` or `SYS-014`.

## Current Behavior

- `requirements/SWR.md` currently documents stale `Verified by:` text for
  `SWR-GUI-001` and `SWR-GUI-002`, and an overstated structural-verification
  claim for `SWR-GUI-003`.
- `tests/unit/test_auth.cpp` defines the current auth evidence as:
  - `UsersTest.REQ_GUI_001_*` for `auth_validate()` behavior.
  - `UsersTest.REQ_GUI_002_*` for `auth_display_name()`-related behavior.
- `requirements/TRACEABILITY.md` already maps:
  - `SWR-GUI-001` to `UsersTest.REQ_GUI_001_*`.
  - `SWR-GUI-002` to `UsersTest.REQ_GUI_002_*`.
  - `SWR-GUI-003` to GUI demonstration evidence rather than auth unit tests.
- `src/gui_auth.c` and `src/gui_main.c` already implement the approved auth and
  GUI behaviors; this issue is about evidence wording only.

## Proposed Change

1. Update the `Verified by:` line for `SWR-GUI-001` in `requirements/SWR.md`
   from the stale `AuthValidation.*` label to
   `UsersTest.REQ_GUI_001_*`.
2. Update the `Verified by:` line for `SWR-GUI-002` from the stale
   `AuthDisplayName.*` label to `UsersTest.REQ_GUI_002_*`.
3. Update the `Verified by:` text for `SWR-GUI-003` so it no longer claims
   structural verification by auth unit tests and instead aligns to GUI
   demonstration/manual evidence already described in the RTM.
4. Keep the requirement statements, `Traces to:` mappings, and `Implemented in:`
   references unchanged unless a strictly local wording fix inside
   `requirements/SWR.md` is needed to avoid ambiguity.
5. If the repository process requires document metadata or revision-history
   maintenance for an SWR edit, keep that change confined to
   `requirements/SWR.md` and do not expand scope into unrelated traceability
   artifacts.

## Files Expected To Change

Expected implementation file:

- `requirements/SWR.md`

Files expected not to change for this issue:

- `requirements/TRACEABILITY.md`
- `tests/unit/test_auth.cpp`
- `src/gui_auth.c`
- `src/gui_main.c`
- Any other production, test, CI, or release file

## Requirements And Traceability Impact

- Affected SWR entries:
  - `SWR-GUI-001`
  - `SWR-GUI-002`
  - `SWR-GUI-003`
- Related higher-level requirements remain unchanged:
  - `SYS-013`
  - `SYS-014`
- This is a traceability and verification-evidence correction for existing
  approved behavior, not a behavior change.
- No new SWR IDs, SYS IDs, test cases, or acceptance criteria should be
  introduced.
- Because the issue touches authentication and traceability wording, the final
  implementation review should explicitly confirm that documentation claims do
  not overstate automated evidence for GUI presentation behavior.

## Medical-Safety, Security, And Privacy Impact

- Medical safety: indirect positive impact only. Correct evidence mapping makes
  review of approved authentication and GUI requirements more trustworthy, but
  does not alter clinical thresholds, alerting, NEWS2, device I/O, or patient
  workflow behavior.
- Security: no intended control change. Authentication, password storage, role
  enforcement, and logout flows remain untouched; only their verification
  references are corrected.
- Privacy: no intended impact. No patient-data collection, persistence, access,
  or export path changes.

## Validation Plan

Use targeted text validation rather than full product test reruns:

```powershell
rg -n "SWR-GUI-001|SWR-GUI-002|SWR-GUI-003|Verified by:" requirements/SWR.md
rg -n "REQ_GUI_001|REQ_GUI_002|AuthValidation|AuthDisplayName" tests/unit/test_auth.cpp requirements/TRACEABILITY.md
git diff --name-only
git diff -- requirements/SWR.md
```

Expected validation outcome:

- `requirements/SWR.md` cites `UsersTest.REQ_GUI_001_*` for `SWR-GUI-001`.
- `requirements/SWR.md` cites `UsersTest.REQ_GUI_002_*` for `SWR-GUI-002`.
- `requirements/SWR.md` no longer claims `AuthValidation.*` structurally
  verifies `SWR-GUI-003`.
- The changed file list remains documentation-only.

No executable behavior change is expected, so routine unit/integration reruns
are not required unless the implementer discovers an unrelated discrepancy.

## Rollback Or Failure Handling

- If implementation-time inspection shows that `requirements/TRACEABILITY.md` or
  `tests/unit/test_auth.cpp` no longer represent the approved canonical
  evidence, stop and re-evaluate scope before editing multiple traceability
  artifacts.
- If reconciling `SWR-GUI-003` requires inventing new automated evidence, do
  not proceed under this issue; leave GUI-demo evidence in place and escalate a
  follow-on test-design issue instead.
- Rollback is straightforward because the intended implementation is limited to
  SWR documentation: revert the `requirements/SWR.md` change and restore the
  prior evidence wording if necessary.
