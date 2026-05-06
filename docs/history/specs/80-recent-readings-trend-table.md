# Design Spec: Issue 80

Issue: `#80`
Branch: `feature/80-recent-readings-trend-table`
Spec path: `docs/history/specs/80-recent-readings-trend-table.md`

## Problem

The current dashboard already stores and renders the data needed for short-run
trend review, but it does not present that data in the compact format requested
by the issue:

- `src/gui_main.c` paints sparkline strips inside the vital tiles from
  `g_app.patient.readings`, which gives directional context but not easy
  adjacent-value comparison.
- `update_dashboard()` rebuilds a free-form `Reading History` list, but each row
  is a long text sentence rather than a compact table, and the surface is not
  scoped to the most recent 5 readings requested by the issue.
- Operators who want to compare recent values must either parse the long raw
  history lines or infer values from the sparkline alone.

This is a review ergonomics gap, not a clinical-logic gap.

## Goal

Add a bounded, read-only recent-readings trend table on the dashboard that:

- shows only the latest 5 stored readings from the active session
- makes row order explicit without implying unsupported timestamps
- reuses only existing `PatientRecord.readings` data and existing alert/status
  semantics
- stays visually secondary to the existing tiles, banner, active alerts, and
  session alarm event review

## Non-goals

- Changing vital-sign thresholds, NEWS2 scoring, alarm limits, alert generation,
  or treatment guidance.
- Adding timestamps, elapsed-time calculations, or claims about equal sampling
  intervals.
- Adding persistence, export, printing, logging, network transmission, or
  cross-session history retention.
- Expanding the reading buffer beyond `MAX_READINGS`, adding dynamic allocation,
  or introducing a second history store.
- Broad dashboard redesign, new clinical workflow controls, filtering, or
  competitor-style timeline review.
- Reworking `patient_print_summary()` or other console-review surfaces for this
  issue.

## Intended Use Impact, User Population, Operating Environment, And Foreseeable Misuse

Intended use impact:

- The feature adds a secondary dashboard review surface for recent in-session
  values only.
- It does not change the primary monitoring workflow or the meaning of existing
  clinical outputs.

User population:

- Trained clinical users, internal testers, and reviewers using the local Win32
  workstation application.

Operating environment:

- The existing single-patient desktop GUI, using in-process static storage and
  either simulator-fed or manually entered readings.

Foreseeable misuse:

- Assuming the rows represent true timestamps or exact elapsed time.
- Reading the table as a clinical interpretation aid rather than a raw value
  review aid.
- Assuming the table contains more than the bounded session buffer or more than
  the displayed 5-row recent window.
- Treating stale rows after clear, logout, patient refresh, or automatic buffer
  reset as current-patient information.
- Trusting the table if it ever disagrees with the existing banner or tile data.

## Current Behavior

- `PatientRecord` stores up to `MAX_READINGS` raw `VitalSigns` entries in
  session order and rejects additions when full.
- `paint_tiles()` already extracts sparkline data directly from
  `g_app.patient.readings` for the five vital tiles.
- `update_dashboard()` currently rebuilds three review controls:
  - `IDC_LIST_ALERTS` for active alerts from the latest reading only
  - `IDC_LIST_EVENTS` for session alarm events
  - `IDC_LIST_HISTORY` for free-form raw reading rows
- The `Reading History` list is full-width text, not a columnar trend table,
  and currently shows up to all retained readings.
- The current data model does not store capture timestamps, only session order.
- `do_clear()`, `do_admit()`, logout/session transitions, and automatic
  full-buffer reset paths already clear or reinitialize patient session state.

## Proposed Change

1. Reuse the existing bottom `Reading History` panel footprint rather than add a
   fourth scrolling review surface. Rename that surface to a recent-trend label
   such as `Recent Trend Readings`.
2. Replace the free-form history listbox in that panel with a read-only
   report-style table control, preferably a Win32 `SysListView32` in report
   mode, so the GUI shows true columns instead of packed text strings.
3. Use fixed columns for the bounded MVP:
   - `Reading`
   - `HR`
   - `BP`
   - `Temp`
   - `SpO2`
   - `RR`
   - `Status`
4. Populate the table only from existing `g_app.patient.readings` entries.
   No new patient-domain fields, caches, persistence layers, or shadow history
   stores are needed.
5. On each `update_dashboard()` call, compute the recent window as:
   - `start = max(0, reading_count - 5)`
   - `end = reading_count - 1`
   and repopulate rows from that slice in stored order, oldest to newest within
   the displayed window.
6. Make order explicit by showing absolute reading sequence numbers such as
   `#6`, `#7`, `#8`, `#9`, `#10`. Do not add timestamp or duration columns.
7. Reuse existing display semantics per row:
   - heart rate, blood pressure, temperature, SpO2, and respiration values come
     directly from the stored `VitalSigns`
   - respiration shows `--` when the stored value is `0`, matching current UI
   - `Status` derives from `overall_alert_level()` plus `alert_level_str()`
8. Keep the existing tiles, NEWS2 tile, status banner, active-alert list, and
   session alarm-event list unchanged in purpose. The recent-trend table is a
   secondary review aid, not a replacement for the current alert surfaces.
9. Use the same patient/session snapshot already consumed by the sparkline and
   list rebuild path so the table cannot lag behind the rest of the dashboard
   after a repaint or refresh.
10. Empty-state behavior must be explicit:
    - no patient admitted: placeholder row or equivalent message
    - patient admitted but no readings: placeholder row or equivalent message
    - after clear/logout/session reset: no stale rows remain visible
11. Keep the existing minimum window size if possible by reusing the current
    history panel area and fitting exactly one header row plus up to 5 data
    rows. If column widths need tuning, confine that change to the existing
    bottom panel instead of broadening the dashboard layout.

## Files Expected To Change

Expected requirements and traceability files:

- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`
- `README.md`

Expected implementation files:

- `include/localization.h`
- `src/localization.c`
- `src/gui_main.c`

Expected verification or evidence files:

- `dvt/DVT_Protocol.md`

Files expected not to change:

- `include/patient.h`
- `src/patient.c`
- `src/trend.c`
- `include/trend.h`
- `src/alerts.c`
- `src/news2.c`
- `src/alarm_limits.c`
- session persistence or auth modules

## Requirements And Traceability Impact

- Existing higher-level requirements that already cover the bounded scope:
  - `UNS-009` vital sign history
  - `UNS-014` graphical dashboard
  - `SYS-009` vital sign reading history
  - `SYS-014` graphical vital signs dashboard
- Existing software requirements adjacent to this change:
  - `SWR-GUI-003` dashboard presentation and repaint behavior
  - `SWR-GUI-004` dashboard behavior tied to patient/session data
  - `SWR-TRD-001` sparkline trend surface that the new table complements
- The current approved SWR set does not explicitly describe a compact,
  last-5, columnar trend-review table. A new GUI requirement is the cleanest
  traceable path.
- Preferred requirement addition:
  - `SWR-GUI-014 - Recent Readings Trend Table`
  - traces to `SYS-009` and `SYS-014`
  - implemented in `src/gui_main.c` via control creation, layout, and
    `update_dashboard()` repopulation
  - verified primarily by manual GUI review / DVT evidence
- No new UNS or SYS statement is required for this bounded dashboard-only MVP.
  If stakeholders later want true timestamps, a second retained history
  surface, console-summary parity, or broader review claims, that should be a
  separate requirement change.

## Medical-Safety, Security, And Privacy Impact

Medical safety:

- Direct clinical-logic impact is low because no thresholds, alerts, NEWS2
  scoring, or treatment guidance change.
- The primary hazard is display integrity: misordered, stale, truncated, or
  mislabeled rows could cause a reviewer to misunderstand recent patient
  direction.
- Required controls:
  - explicit `Reading #` order indicator
  - no timestamp or elapsed-time claims
  - row values sourced only from the existing patient buffer
  - per-row status reused from existing alert semantics
  - same refresh boundary as the sparkline and other dashboard lists
  - explicit empty-state handling after patient/session transitions

Security:

- No new authentication, authorization, or privilege paths are introduced.
- The feature remains inside the same authenticated in-process dashboard
  boundary as the data it duplicates.

Privacy:

- No new persistence, export, or transmission path is introduced.
- The table duplicates patient observations already visible in the dashboard, so
  privacy impact remains bounded to local display.

## AI/ML Impact Assessment

This change does not add, change, remove, or depend on an AI-enabled device
software function.

- Model purpose: not applicable
- Input data: existing deterministic vital-sign readings only
- Output: deterministic row rendering of already stored values
- Human-in-the-loop limits: unchanged
- Transparency needs: the UI must remain explicit that row order is sequence
  order, not clock time
- Dataset and bias considerations: not applicable
- Monitoring expectations: standard software verification and manual GUI review
- PCCP impact: none

## Validation Plan

Regression commands:

```powershell
cmake -S . -B build -G Ninja -DBUILD_TESTS=ON
cmake --build build
ctest --test-dir build --output-on-failure -R "Patient|Trend|GUI"
run_tests.bat
```

Manual GUI verification should cover:

- no-patient and no-reading placeholders
- manual admission plus 1 through 5 readings
- more than 5 readings, proving the window slides to the latest 5 only
- reading-order correctness within the displayed slice
- value fidelity against known manual-entry and demo-scenario inputs
- status-column consistency with `overall_alert_level()` for each displayed row
- correct `RR` placeholder behavior when respiration is not measured
- clear session, logout, patient refresh, simulation disable, and automatic
  full-buffer reset removing stale rows
- continued consistency between table, sparkline, tiles, active alerts, and
  session alarm events after each refresh

Verification evidence approach:

- Primary verification should be manual GUI review / DVT because the change is
  confined to Win32 presentation behavior.
- If implementation extracts any pure row-formatting helper, add targeted unit
  coverage for row-window selection and placeholder behavior; otherwise do not
  widen the issue just to force artificial automation.

## Rollback Or Failure Handling

- If the report-style table control cannot be created or fit cleanly in the
  existing bottom panel, fall back to the pre-change raw history surface rather
  than widening scope into a larger dashboard redesign.
- A table-rendering defect must not block patient admission, reading ingestion,
  sparkline painting, alert generation, or session reset behavior.
- If stakeholders later insist on preserving the current full raw history list
  and also adding a separate recent-trend table, split that into a follow-up
  issue instead of broadening this MVP beyond the existing panel footprint.
