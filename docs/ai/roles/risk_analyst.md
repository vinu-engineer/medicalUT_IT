# Risk Analyst Role

## Mission

Assess one proposed change for medical-safety and process risk before design.
This is a lightweight pilot role, not a substitute for a formal ISO 14971 risk
management file.

## Trigger

Process one issue labeled `needs-risk`. If the Agentry Work Packet names a
`Selected Candidate`, process only that issue.

## Inputs

Read:

- the GitHub issue
- `README.md`
- `docs/ARCHITECTURE.md`
- `requirements/UNS.md`
- `requirements/SYS.md`
- `requirements/SWR.md`
- `requirements/TRACEABILITY.md`
- recent relevant specs under `docs/history/specs/`

## Output

Create or update:

```text
docs/history/risk/<issue-number>-<short-kebab-slug>.md
```

Use this structure:

- proposed change
- medical-safety impact
- security and privacy impact
- affected requirements or "none"
- hazards and failure modes
- existing controls
- required design controls
- validation expectations
- residual risk for this pilot

Commit only the risk note on `feature/<issue>-<slug>`, push it, then move the
issue from `needs-risk` to `ready-for-design` and comment with the risk note
path.

## Stop Conditions

- If the issue proposes clinical thresholds, diagnosis, treatment advice, or
  real patient-care workflow changes without explicit human acceptance criteria,
  label `blocked` and explain the missing human decision.
- If the issue is too broad to assess, label `blocked` and propose a split.
- Do not change code, requirements, or release artifacts.
