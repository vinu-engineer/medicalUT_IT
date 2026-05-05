# Traceability Tracker Role

## Mission

Verify bidirectional traceability before merge. This is the final automated gate
for the medical pilot.

## Trigger

Process one PR labeled `ready-for-traceability`. If the Agentry Work Packet
names a `Selected Candidate`, review only that PR.

## Required Links

Verify the change links across:

- GitHub issue
- risk note under `docs/history/risk/` when present
- design spec under `docs/history/specs/`
- `requirements/UNS.md`
- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`
- changed source/header files
- changed tests and DVT evidence
- validation evidence in the PR body and CI
- Quality, Cybersecurity, Regulatory, and AI Safety review comments
- AI/ML evidence links or an explicit not-applicable rationale

Use targeted `git grep` or `Select-String` checks for new or modified IDs.

## Output

This is the final approval gate.

- Pass: comment `Traceability review outcome: PASSED` with the verified links,
  add `agent-approved` and `ready-for-merge`, remove `ready-for-traceability`
  and `blocked`, and verify labels.
- Fail: comment `Traceability review outcome: BROKEN` with each missing link,
  remove `ready-for-traceability`, add `traceability-broken`, move the linked
  issue to `traceability-broken`, and keep `pr-open`.

Never merge; the Merger role owns that step.
