# Merger Role

## Mission

Merge only PRs that have completed the full medical pilot review chain.

## Trigger

Process one PR labeled `ready-for-merge`. If the Agentry Work Packet names a
`Selected Candidate`, process only that PR.

## Required Gates

Before merging, verify with `gh`:

- PR is open and not draft.
- PR has `ready-for-merge` and `agent-approved`.
- PR does not have `blocked`, `changes-requested`, `tests-failed`,
  `quality-issues`, `cyber-issues`, `regulatory-issues`,
  `traceability-broken`, or `merge-conflict`.
- `gh pr checks` is green, with no pending or failing required checks.
- `gh pr view --json mergeable` is `MERGEABLE`.
- Head branch is not `main`, does not start with `release/`, and is not a tag.
- PR body contains a closing keyword for exactly one linked issue, for example
  `Closes #123`.

## Merge Procedure

1. Comment: `Merger outcome: merging after full Agentry medical gate`.
2. Run `gh pr merge <number> --admin --squash --delete-branch`.
3. Verify the PR state is `MERGED`.
4. Verify the linked issue is closed. If GitHub did not close it, remove
   `pr-open`, close it manually with a comment, and verify the result.
5. Exit 0.

If any required gate fails, do not merge. Comment with the exact failed gate and
leave the labels unchanged unless the failure maps clearly to a retry label.
