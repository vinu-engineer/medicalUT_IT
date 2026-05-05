# Design Spec: Documentation reconcile legacy repo references and stale test counts

Issue: #27
Branch: `feature/27-documentation-reconcile-legacy-repo-references-and-stale-test-counts`
Spec path: `docs/history/specs/27-documentation-reconcile-legacy-repo-references-and-stale-test-counts.md`

## Problem

Issue #27 identifies documentation and workflow-metadata drift between the
current repository identity and the text published in current-facing project
artifacts. Several files still point to the legacy
`vinu-engineer/medicalUT_IT` repository or publish obsolete test-count claims
such as `145 test cases`, `121+ tests`, or `287 total`, even though current
canonical evidence already identifies the active repository as
`vinu-dev/medvital-monitor` and the current verification baseline as 295 tests.

The defect is not in runtime behavior. It is in provenance and evidence text
that operators, reviewers, and auditors may use to locate the repository,
release artifacts, or current verification baseline.

## Goal

Produce a narrow documentation-only change set that makes current-facing
repository links, generated workflow metadata, and published test-count text
match the current canonical repository and test evidence.

The implementation should leave a small, reviewable diff and make it easy for a
reviewer to verify that:

- current-facing GitHub URLs resolve to `vinu-dev/medvital-monitor`
- current-facing test-count claims match the 295-test baseline
- no runtime, clinical, CI-behavior, or requirements-logic changes were made

## Non-goals

- No production code, tests, thresholds, NEWS2 logic, alarm limits,
  authentication, persistence, or release mechanics changes.
- No changes to approved UNS, SYS, or SWR statements unless a current-facing
  documentation summary is merely being aligned to already approved evidence.
- No edits to historical records under `docs/history/specs/` or
  `docs/history/risk/` solely to remove old repository names or historical test
  totals. Those files are audit artifacts and may legitimately describe older
  states.
- No attempt to make every historical revision-history row mention 295 tests.
  Historical revision rows should remain historically accurate.
- No broad documentation refresh outside repository identity and published test
  evidence.

## Current behavior

Current canonical evidence already exists:

- `.github/workflows/ci.yml` labels the Windows job `295 tests`
- `requirements/TRACEABILITY.md` revision I reports `37/37 SWRs` and `295`
  total tests
- issue #27 defines the intended scope as documentation/evidence maintenance
  only

Current-facing drift still exists in multiple places:

- `README.md` still uses legacy GitHub badge and release links, the old
  `medicalUT_IT/` repository-root label, stale SWR/test totals in the repository
  tree, and obsolete `287 total` test claims.
- `CLAUDE.md` still describes the pipeline test stage as `145 test cases`.
- `requirements/README.md` still uses `121+ tests` as a release-readiness
  checklist threshold.
- `docs/ARCHITECTURE.md` still describes the test tree as `136 unit tests` and
  `12 integration tests`.
- `docs/DEVOPS_WORKFLOWS.md` still tells developers to clone the legacy
  repository URL.
- `.github/workflows/release.yml` still writes the legacy repository URL into
  generated release-note metadata.
- `.github/workflows/pipeline.yml` still writes the legacy repository URL into
  the generated GitHub Pages footer.

One validation complication is already visible: the issue's raw text-search
pattern also matches historical design and risk documents, which should not be
rewritten just to force zero grep hits. The implementation therefore needs a
current-facing validation scope, not an all-history rewrite.

## Proposed change

Implement the issue as a provenance-and-evidence reconciliation only, with the
following decisions:

1. Treat `origin` remote naming, `.github/workflows/ci.yml`, and
   `requirements/TRACEABILITY.md` revision I as the canonical sources for
   repository identity and current test-count evidence.
2. Update current-facing documentation files that publish stale repository
   links or stale test totals, specifically:
   - `README.md`
   - `CLAUDE.md`
   - `requirements/README.md`
   - `docs/ARCHITECTURE.md`
   - `docs/DEVOPS_WORKFLOWS.md`
3. Update workflow templates that generate stale repository provenance text:
   - `.github/workflows/release.yml`
   - `.github/workflows/pipeline.yml`
4. Keep `.github/workflows/ci.yml` unchanged unless implementation discovers
   that the stated 295-test baseline itself is wrong. Under the current issue
   evidence, it is an authority, not a target for change.
5. Keep `requirements/TRACEABILITY.md` unchanged unless implementation finds a
   separate current-facing inconsistency unrelated to historical revision rows.
   Under current evidence, it is the test-count source of truth.
6. Update `README.md` repository-structure examples and testing summaries so
   they reflect the current repository name, the presence of the localization
   test file, the current `37/37 SWR` traceability state, and the 295-test
   total already documented in the RTM.
7. Exclude `docs/history/specs/**` and `docs/history/risk/**` from stale-link
   and stale-count cleanup unless a historical file is itself the broken
   deliverable. That is not the case for issue #27.
8. Review the final diff to ensure the change stays documentation and generated
   metadata only, with no functional CI logic or release-behavior edits beyond
   published text.

## Files expected to change

Expected files:

- `README.md`
- `CLAUDE.md`
- `requirements/README.md`
- `docs/ARCHITECTURE.md`
- `docs/DEVOPS_WORKFLOWS.md`
- `.github/workflows/release.yml`
- `.github/workflows/pipeline.yml`

Expected files to inspect but likely not modify:

- `.github/workflows/ci.yml`
- `requirements/TRACEABILITY.md`

Files that should not change:

- `src/**`
- `include/**`
- `tests/**`
- `requirements/SYS.md`
- `requirements/SWR.md`
- `docs/history/specs/**` except this new issue-27 spec
- `docs/history/risk/**`

## Requirements and traceability impact

No approved requirements behavior changes are expected. The issue body and risk
note both state that runtime and clinical behavior are out of scope.

Traceability impact is indirect:

- repository-facing evidence should point to the correct repository
- published test-count summaries should match the current RTM baseline
- reviewers should be able to correlate README and workflow-generated metadata
  back to `requirements/TRACEABILITY.md` without manual reconciliation

No `@req` tags, no test identifiers, and no SWR/SYS logic should change.

## Medical-safety, security, and privacy impact

Medical-safety impact is low and indirect. This change does not alter vital
classification, NEWS2 scoring, alarm limits, authentication, persistence,
simulation, or patient-record handling. The safety value is provenance clarity:
operators and auditors should not be routed to the wrong repository or an
obsolete verification baseline when assessing a release.

Security impact is limited to integrity and provenance. Correct repository URLs
reduce the risk of humans consulting the wrong release history or artifacts.
There is no credential, RBAC, or cryptographic behavior change.

Privacy impact is none expected. No patient data, logs, exports, or storage
paths change.

## Validation plan

Use targeted text validation against current-facing files, not all history
artifacts.

Primary validation:

```powershell
rg -n "vinu-engineer/medicalUT_IT|medicalUT_IT/|145 test cases|287 total|Run all 287 tests|121\+ tests|136 unit tests|36/36 SWR|287 tests" README.md CLAUDE.md docs requirements .github -g '!docs/history/specs/*' -g '!docs/history/risk/*'
```

Confirm that any remaining matches are either:

- expected canonical references such as `295 tests` in `.github/workflows/ci.yml`
- historically accurate revision-history rows
- intentionally untouched audit-history files excluded from the implementation

Evidence cross-check:

```powershell
rg -n "295 tests|37/37 SWR" README.md docs/ARCHITECTURE.md CLAUDE.md requirements/README.md requirements/TRACEABILITY.md .github/workflows/ci.yml
```

Repository-identity cross-check:

```powershell
rg -n "vinu-dev/medvital-monitor" README.md docs .github/workflows
```

Diff-scope review:

```powershell
git diff --stat
git diff -- README.md CLAUDE.md requirements/README.md docs/ARCHITECTURE.md docs/DEVOPS_WORKFLOWS.md .github/workflows/release.yml .github/workflows/pipeline.yml
```

No product test rerun is required for the design intent because executable
behavior is unchanged. If the implementer chooses to run documentation-adjacent
checks anyway, they should treat those as optional confidence checks, not as a
requirement to broaden scope.

## Rollback or failure handling

If implementation discovers conflicting authorities for repository identity or
test totals, stop and escalate rather than choosing a count heuristically.

If the diff expands beyond documentation and generated metadata text, revert the
extra scope before review and keep only the provenance/evidence reconciliation.

If a file appears historically important rather than current-facing, prefer
leaving it untouched and documenting why in the implementation handoff rather
than rewriting audit history.
