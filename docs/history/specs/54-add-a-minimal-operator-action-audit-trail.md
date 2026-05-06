# Design Spec: Add a minimal operator action audit trail

Issue: #54
Branch: `feature/54-add-a-minimal-operator-action-audit-trail`
Spec path: `docs/history/specs/54-add-a-minimal-operator-action-audit-trail.md`

## Problem

The current application records patient vital-sign history and current alerts,
but it does not record a concise history of operator actions. Reviewers can see
what physiological values were entered or observed, yet they cannot directly
answer basic workflow questions such as:

- who logged in,
- whether login failed before a successful session,
- when a patient was admitted or refreshed,
- when a manual reading was added or a session was cleared,
- which settings were changed,
- whether a privileged account-management action succeeded or failed.

This is a traceability gap rather than a clinical-algorithm gap. The current
desktop application already persists user data, simulation configuration, and
alarm-limit configuration, and it exposes several mutating actions in
`src/gui_main.c`, but none of those actions leave a bounded local audit trail.

The gap is especially visible in the current UI:

- `update_dashboard()` shows patient reading history only.
- `attempt_login()` authenticates a user and opens the dashboard, but no login
  event is stored.
- `IDC_BTN_LOGOUT` destroys the dashboard and recreates the login screen, but
  no logout event is stored.
- `do_admit()`, `do_add_reading()`, and `do_clear()` mutate the active session,
  but no operator-action metadata is retained.
- settings changes persist through `app_config_save()`,
  `app_config_save_language()`, `alarm_limits_save()`, `users_add()`,
  `users_remove()`, `users_change_password()`, and
  `users_admin_set_password()`, but those mutations are not auditable today.

The current `main` branch also has no export or report workflow in production
code, so "export actions" are not a present implementation surface that this
issue can hook without broadening scope.

## Goal

Add a narrow, local-only, metadata-only audit trail for key non-clinical
operator actions that:

- uses static memory only,
- persists a fixed-size recent history locally,
- never blocks login, logout, patient admission, manual reading entry, or
  settings changes when audit persistence fails,
- exposes a read-only in-app review surface for the retained events, and
- introduces the missing requirements and traceability entries before or with
  implementation.

## Non-goals

- No changes to clinical thresholds, NEWS2 scoring, alert generation, patient
  classification, or treatment guidance.
- No logging of raw passwords, password hashes, free-text notes, full vital
  sign payloads, report contents, or arbitrary file paths.
- No cloud sync, remote callbacks, background telemetry, or network
  dependency.
- No claim of medico-legal immutability, full forensic completeness, or
  replacement of the clinical record.
- No new patient-session export feature in this issue solely to create an
  export hook.
- No editable audit log, note-taking, signatures, or workflow gating in MVP.

## Intended use impact, user population, operating environment, and foreseeable misuse

Intended use impact:

- The change adds a review aid for accountability and troubleshooting.
- It does not alter bedside monitoring logic or alert behavior.

User population:

- authenticated local operators already using the Win32 desktop application,
- local administrators who manage users and settings,
- reviewers or QA staff who need to reconstruct non-clinical operator actions.

Operating environment:

- the existing Windows desktop deployment,
- local file persistence beside the executable,
- no added network service or remote storage.

Foreseeable misuse:

- treating the audit trail as a complete clinical record,
- assuming the retained history is complete after ring-buffer wraparound,
- exposing patient names or sensitive inputs when only minimal identifiers are
  needed,
- assuming a failed action succeeded because the audit schema did not
  distinguish success from failure,
- using the MVP trail as evidence of legal chain-of-custody or timing
  precision beyond the local workstation clock.

## Current behavior

Current operator-action behavior is spread across the GUI and persistence
modules:

- `src/gui_main.c`
  - `attempt_login()` validates credentials and opens the dashboard.
  - `IDC_BTN_LOGOUT` logs the user out and returns to the login screen.
  - `do_admit()` initializes the active patient record.
  - `do_add_reading()` validates input and appends a manual reading.
  - `do_clear()` clears the current patient session.
  - settings actions save simulation mode, language, alarm limits, account
    additions/removals, and password changes.
- `src/gui_users.c`
  - persists `users.dat` with restricted file permissions and contains the
    account-management APIs that the GUI calls.
- `src/app_config.c`
  - persists `monitor.cfg` for simulation mode and language.
- `src/alarm_limits.c`
  - persists `alarm_limits.cfg`.

Current review surfaces are insufficient for operator audit:

- the dashboard history list shows only patient readings,
- the alerts list shows only active clinical alerts,
- the settings dialog has no audit tab or equivalent read-only event view,
- no audit file is loaded on startup or saved on mutation.

Current scope boundary:

- There is no production export surface in `main`, so there is nothing real to
  audit for export actions yet.

## Proposed change

### 1. Add a dedicated audit-log service module

Add a new application-service module:

- `include/audit_log.h`
- `src/audit_log.c`

The module should own all audit event storage and persistence. It should not
depend on the clinical domain modules and should remain outside the vital-sign
classification path.

Recommended MVP shape:

- static in-memory ring buffer with a compile-time capacity such as
  `AUDIT_MAX_EVENTS = 128`,
- append-only logical event sequence number,
- best-effort load on startup and best-effort save after append,
- versioned persisted file in the executable directory, for example
  `audit_log.dat`,
- restricted local file permissions comparable to `users.dat` because the file
  contains usernames and patient identifiers.

Persistence strategy:

- Keep the source of truth in memory while the process is running.
- On each append, update memory first, then attempt a bounded save of the full
  retained window.
- If saving fails, keep the in-memory event and continue the user action.
- On startup, if the file is absent, unreadable, or malformed, start with an
  empty buffer and continue without blocking application startup.

### 2. Define a minimal metadata-only event schema

Each stored event should contain only the metadata needed to reconstruct the
operator action:

- `sequence_no`
- local timestamp
- actor username or entered username when authentication failed
- actor role, when known
- action type
- target type
- target identifier, when applicable
- result code

Recommended exclusions:

- no password or password-hash material,
- no patient name,
- no raw vital-sign payload values,
- no free-text form contents,
- no serialized settings file contents,
- no arbitrary file paths.

For patient context, prefer the numeric patient ID already present in the GUI
flow. Do not add patient-name logging in MVP.

Recommended action taxonomy for MVP:

- login success
- login failure
- logout
- patient admit or refresh
- manual reading added
- session cleared
- simulation mode changed
- language changed
- alarm limits saved
- alarm limits reset
- user added
- user removed
- password changed by self
- password changed by admin

Recommended result taxonomy:

- success
- validation failed
- access denied
- persistence failed
- capacity full
- unknown failure

Short human-readable result strings should be derived by the UI from the stored
codes rather than stored as free text.

### 3. Add GUI hook points only where actions already exist

Hook the new audit API into the existing action points in `src/gui_main.c`.
Prefer GUI-layer hooks over changes to the clinical domain modules.

Hook scope for MVP:

- `attempt_login()`
  - log failed authentication attempts without storing secrets,
  - log successful login after role resolution succeeds.
- logout handler for `IDC_BTN_LOGOUT`
  - log logout before session state is cleared.
- `do_admit()`
  - log admit or refresh success with patient ID,
  - log validation failure when the action is attempted but rejected.
- `do_add_reading()`
  - log successful manual reading addition,
  - log rejection when validation fails or the patient buffer is full,
  - do not store the raw reading values in the event.
- `do_clear()`
  - log session clear success.
- settings mutations
  - simulation-mode toggle,
  - language apply,
  - alarm-limit save and reset,
  - user add, remove, and password-change operations.

Do not log passive navigation in MVP:

- opening the settings dialog,
- tab switching,
- repaint events,
- timer ticks,
- automatic simulator readings.

### 4. Add a read-only audit review surface

Expose retained events through a read-only audit tab in the existing Settings
window rather than a new top-level workflow.

Default MVP access policy:

- admin-only review access.

Rationale:

- this is the lowest-privilege expansion that still solves the review problem,
- it avoids exposing a broader cross-user action history to every clinical
  operator before policy is agreed more explicitly.

UI expectations:

- read-only list or table,
- columns or formatted rows for timestamp, actor, role, action, target, and
  result,
- most-recent-first or oldest-first ordering is acceptable as long as it is
  consistent and documented,
- no edit, clear, or export button in MVP.

If product ownership later wants broader read access, that should be a separate
policy decision layered on top of the same audit service.

### 5. Keep export hooks explicitly deferred unless an export surface already exists

Issue `#54` mentions export actions, but current production code in `main` does
not yet expose an export workflow. To keep this issue narrow:

- do not add a new export feature solely to satisfy the audit trail,
- keep the event schema extensible so a future export action can reuse it,
- if an export surface already exists on the implementation branch at coding
  time, it may add the corresponding audit hook through the same API,
- otherwise, export-action capture is deferred without blocking this issue.

### 6. Add missing requirements and traceability coverage

The current approved baseline does not include operator-action audit
requirements. Implementation should not proceed without adding them.

Use the next available IDs in the requirements set. If numbering has not moved
before implementation, the likely additions are:

- `UNS-017` for operator-action traceability and bounded local review,
- `SYS-020` for operator-action audit capture and exclusions,
- `SYS-021` for retained local review behavior and access policy,
- `SWR-AUD-001` through `SWR-AUD-006` for schema, retention, persistence,
  non-blocking behavior, GUI review surface, and hook coverage.

Expected requirement content:

- capture only defined action types and metadata fields,
- exclude sensitive inputs and clinical payloads,
- retain only a bounded recent window with overwrite behavior,
- persist locally and tolerate file absence or corruption,
- never block core monitoring or account actions when logging fails,
- expose a read-only review surface with explicit role-based access.

If the traceability tracker chooses different numeric IDs, preserve the same
scope and one-to-one coverage.

## Files expected to change

Implementation is expected to touch only the audit-adjacent service, GUI,
tests, build wiring, and requirements artifacts.

Likely source and header changes:

- `include/audit_log.h` (new)
- `src/audit_log.c` (new)
- `src/gui_main.c`
- `CMakeLists.txt`
- `tests/CMakeLists.txt`

Likely verification changes:

- `tests/unit/test_audit_log.cpp` (new)
- `tests/unit/test_auth.cpp`
- `tests/unit/test_config.cpp`

Likely requirements and traceability changes:

- `requirements/UNS.md`
- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`

Files that should not change for this issue:

- `src/vitals.c`
- `src/alerts.c`
- `src/news2.c`
- `src/patient.c`
- clinical threshold constants,
- CI or release workflows unless build wiring for the new module requires a
  small non-behavioral update.

## Requirements and traceability impact

This issue adds a new non-clinical traceability slice rather than modifying an
existing clinical algorithm.

Expected traceability impact:

- one new user need for operator-action traceability,
- one or more new system requirements for metadata capture and retained review,
- a small new software-requirements cluster under a dedicated audit module,
- new unit-test evidence for ring-buffer behavior, persistence, corruption
  handling, and exclusion rules,
- GUI/manual verification that audit review is read-only and role-appropriate.

The issue should also add explicit traceability for:

- login and logout event capture,
- patient admit or refresh,
- manual reading addition,
- settings mutations that already exist in the product,
- rejection and failure outcomes where the action is attempted but does not
  complete.

## Medical-safety, security, and privacy impact

Medical safety:

- direct clinical impact should remain low because the feature is off the
  clinical decision path,
- indirect safety value is real because post-incident reconstruction becomes
  less ambiguous,
- implementation must not delay or gate monitoring, login, logout, patient
  admission, or manual reading entry.

Security:

- this feature creates a new persisted local artifact that contains user-action
  history,
- the audit file should use restricted local file permissions and a bounded
  format,
- review access should default to admin-only in MVP,
- login-failure capture must not reveal whether the username or password was
  wrong.

Privacy:

- the main privacy risk is over-collection,
- patient names and raw vital values should remain out of scope,
- patient ID is acceptable as the minimal target identifier for MVP,
- the retained window must be bounded and clearly understood as recent history,
  not a permanent record.

## AI/ML impact assessment

This change does not add, remove, modify, or depend on an AI-enabled device
software function.

- Model purpose: none.
- Input data to any model: none.
- Model output: none.
- Human-in-the-loop limits: not applicable.
- Transparency needs: standard UI transparency only; no AI-specific behavior.
- Dataset and bias considerations: none.
- Monitoring expectations: normal software verification only.
- PCCP impact: none.

## Validation plan

Required build and regression commands:

```powershell
build.bat
run_tests.bat
ctest --test-dir build --output-on-failure -R Audit|UsersTest|ConfigTest
```

Required automated verification scope:

- ring-buffer append, overwrite, ordering, and retrieval,
- save and reload of a bounded persisted event window,
- corruption or missing-file recovery to an empty audit buffer,
- exclusion tests proving that passwords, hashes, patient names, and raw vital
  values are not stored,
- login success and failure audit coverage,
- settings-change audit coverage for simulation mode, language, alarm limits,
  and account mutations.

Required manual or GUI verification scope:

- audit tab is read-only,
- audit tab is visible only to the intended role in MVP,
- login, logout, admit or refresh, add reading, and settings changes create
  correctly ordered events,
- monitoring actions still succeed when the audit file cannot be written,
- no clinical tiles, alerts, or NEWS2 behavior changed.

## Rollback or failure handling

Runtime failure handling:

- if the audit file is missing, unreadable, or malformed, start with an empty
  retained history and continue normal operation,
- if an append-save attempt fails, keep the current action result and continue
  the user workflow,
- do not display blocking message boxes for audit persistence failures during
  core monitoring actions,
- if an operator-visible warning is needed, confine it to the audit review
  surface or another non-blocking location.

Rollback handling:

- revert the audit-module and GUI-hook commit(s) if review rejects the access
  policy or persistence shape,
- if implementation discovers that the requirement additions are not yet
  approved, stop before merging production code and return the item to
  documentation and traceability review rather than shipping untraced behavior.
