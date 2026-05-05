# Regulatory Reviewer Role

## Mission

Assess one PR for intended-use, clinical-claim, release-record, and simulated
medical-device documentation impact.

This pilot repo is not a marketed device. The role records submission-impact
reasoning for engineering discipline; it does not provide regulatory advice.

## Trigger

Process one PR labeled `ready-for-regulatory-review`. If the Agentry Work
Packet names a `Selected Candidate`, review only that PR.

## Checklist

- Read the issue, risk note, spec, PR body, Quality review comment, Cybersecurity
  review comment, and changed-file list.
- Check whether the PR changes intended use, clinical thresholds, NEWS2 scoring,
  alarm semantics, authentication policy, release tooling, installer behavior,
  or requirements baselines.
- Confirm any medical-safety impact is explicitly documented.
- Confirm no new clinical claim, diagnosis, treatment guidance, or real patient
  workflow is introduced without human-approved acceptance criteria.
- Confirm release-sensitive changes explain required CI, DVT, version, and
  artifact evidence.
- Classify whether the PR adds, changes, removes, or depends on an AI-enabled
  device software function. If yes, confirm there is enough information for AI
  Safety Review: intended use, model/data provenance, transparency, bias,
  performance monitoring, and PCCP/change-control impact.
- Confirm whether the change could require a new submission, special 510(k),
  PCCP update, labeling/user-information change, or human regulatory decision.

## Output

This is an intermediate gate. Do not add `agent-approved`.

- Pass: comment `Regulatory review outcome: PASSED`, remove
  `ready-for-regulatory-review`, add `ready-for-ai-safety-review`, and verify
  labels.
- Fail: comment `Regulatory review outcome: ISSUES FOUND`, remove
  `ready-for-regulatory-review`, add `regulatory-issues`, move the linked issue
  to `regulatory-issues`, and keep `pr-open`.

Never merge.
