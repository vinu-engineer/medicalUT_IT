# Agentry Model Characterization Plan

This plan is for measuring Agentry model cost/performance on
`vinu-dev/medvital-monitor` after the low-token pipeline mechanics are proven.

## Baseline History

The first integration run used only Codex models and defaulted every role to
`gpt-5.4-mini`. That cheap pilot profile was intended to validate
queue movement, selected-candidate work packets, label transitions, PR
creation, review gating, and wrapper behavior before spending larger-model
tokens.

The current full medical pilot uses only Codex models and sets every active role
to `gpt-5.4` so the expanded regulated-style chain can be tested with stronger
reasoning before model downshifting.

## Profiles To Compare

| Profile | Researcher | Risk | Architect | Implementer | Tester | Review gates | Merger | Release |
|---------|------------|------|-----------|-------------|--------|--------------|--------|---------|
| cheap-pilot | gpt-5.4-mini | off | gpt-5.4-mini | gpt-5.4-mini | gpt-5.4-mini | gpt-5.4-mini | manual | off |
| full-medical | gpt-5.4 | gpt-5.4 | gpt-5.4 | gpt-5.4 | gpt-5.4 | gpt-5.4 | gpt-5.4 | gpt-5.4 |
| optimized-medical | gpt-5.4-mini | gpt-5.4-mini | gpt-5.4-mini | gpt-5.4 | gpt-5.4-mini | gpt-5.4 | gpt-5.4-mini | gpt-5.4-mini |

Keep the same Agentry version (`v0.1.3` for the current pilot), trigger labels,
timeout settings, and target
issue complexity when comparing profiles. Change only the `-m` values in
`agentry/config.yml`.

## Metrics

Collect these for every role session:

- role, issue or PR number, model, and profile name
- session duration
- tokens used from `agentry status` or `agentry/state/sessions/*.json`
- whether token budget was exceeded
- final label transition
- validation commands run and pass/fail outcome
- whether a human had to repair code, tests, docs, labels, or Git state
- whether the role stayed on the selected candidate
- whether the role read full logs or stayed within work-packet/tail guidance

## Procedure

1. Start with a low-risk issue that does not touch clinical thresholds,
   authentication, password storage, release artifacts, or workflow files.
2. Run the cheap-pilot profile through one standard path:
   design -> implementation -> test -> PR -> review.
3. Run the full-medical profile through one expanded path:
   research -> risk -> design -> implementation -> test -> code review ->
   quality review -> cybersecurity review -> regulatory review -> traceability
   review -> merge -> release smoke check.
4. Save session JSON and relevant log tails before changing profile.
5. Downshift one role class at a time into optimized-medical and compare.
6. Compare tokens per accepted PR, time to green PR, and human repair count.

## Decision Rule

Use the cheapest profile that completes the flow with:

- no missed safety, traceability, auth, or validation gaps
- no repeated label churn or merge-conflict churn caused by the agent
- no multi-candidate work in a single invocation
- no human repair beyond normal review judgment

Escalate individual roles, not the whole pipeline. For this repo, Implementer
and Reviewer are the first roles to try on `gpt-5.4` if `gpt-5.4-mini` is not
reliable enough.
