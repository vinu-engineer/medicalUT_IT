# Risk Note: Issue 27

Issue: `#27`  
Branch: `feature/27-documentation-reconcile-legacy-repo-references-and-stale-test-counts`

## proposed change

Update repository-facing documentation and generated workflow metadata so they
reference the current `vinu-dev/medvital-monitor` repository and the current
295-test verification baseline already reflected in canonical repo evidence.
The intended scope is documentation and published metadata only, including
stale repository links and stale test-count text in files such as `README.md`,
`CLAUDE.md`, `requirements/README.md`, and selected GitHub workflow templates.

## medical-safety impact

No direct runtime, clinical-threshold, alerting, NEWS2, authentication, or
patient-data behavior changes are proposed. The safety impact is indirect:
stale repository provenance and stale verification counts can mislead reviewers,
operators, or auditors about which source tree and evidence set actually back a
release. That can delay review or cause humans to consult the wrong evidence,
but it does not itself change bedside behavior. Overall medical-safety impact
is low if the implementation stays documentation-only.

## security and privacy impact

No privacy-impacting data flow changes are expected. No credential, RBAC, or
patient-record handling change is in scope. The primary security concern is
integrity and provenance: legacy repository links or stale release metadata can
send humans to outdated artifacts or incorrect evidence claims. Privacy impact
is none expected.

## affected requirements or "none"

none. The issue does not request a change to approved UNS, SYS, or SWR
behavior. It reconciles documentation and evidence references to already
approved repository state.

## hazards and failure modes

- Reviewers or operators follow a legacy GitHub link and inspect the wrong
  repository or release history.
- Release notes, README text, or workflow-generated pages claim an obsolete
  automated-test total, creating incorrect audit or change-approval evidence.
- A documentation-only task expands accidentally into requirement, test, or
  runtime changes in an attempt to force counts to match.
- Conflicting canonical sources are resolved by guesswork instead of escalation,
  producing a new documentation error under a safety/compliance label.

## existing controls

- The issue body explicitly states that runtime and clinical behavior are out of
  scope.
- Current canonical evidence already names the newer verification baseline in
  `.github/workflows/ci.yml` and `requirements/TRACEABILITY.md`.
- The repository architecture and requirements isolate clinical logic from
  repository-facing documentation and workflow metadata.
- The issue includes targeted `rg` validation commands that enumerate the known
  stale references.

## required design controls

- Restrict implementation to documentation and workflow metadata text; do not
  modify production code, thresholds, tests, requirements logic, or release
  behavior.
- Treat `origin`, `.github/workflows/ci.yml`, and
  `requirements/TRACEABILITY.md` as the canonical sources for repository
  identity and test-count evidence.
- If canonical sources disagree on the correct repository URL or test baseline,
  stop and escalate instead of choosing a count heuristically.
- Review the changed file list and diff to confirm the patch stays within the
  issue's declared documentation-only scope.
- Preserve explicit wording in implementation notes and PR text that the change
  is provenance/evidence reconciliation only.

## validation expectations

- Run the issue's targeted text searches and confirm no stale legacy repository
  references or obsolete `145`, `287`, or `121+` evidence claims remain in the
  scoped files.
- Inspect the changed file list and diff to verify documentation-only scope.
- Manually confirm that every published repository URL resolves to
  `vinu-dev/medvital-monitor` and every published test-count claim matches the
  current 295-test baseline.
- No product test rerun is required for the risk conclusion because executable
  behavior is unchanged, but published evidence text must match current
  canonical artifacts exactly.

## residual risk for this pilot

Low. Residual risk is limited to human documentation transcription error or
missed stale references, not to patient-facing behavior. With the controls
above, this issue is appropriate to move forward as a low-risk
documentation/provenance reconciliation item.
