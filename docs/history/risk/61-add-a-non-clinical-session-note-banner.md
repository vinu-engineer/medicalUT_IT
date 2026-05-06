# Risk Note: Issue 61

Issue: `#61`
Branch: `feature/61-add-a-non-clinical-session-note-banner`

## proposed change

Add one short operator-entered session note for the active monitoring session.
The intended scope is display-only operational context such as transfer context,
demo scenario labeling, or a handoff cue. The note would be session-scoped,
not a clinical measurement, and would be shown in a review-facing location such
as the dashboard header or patient summary.

## product hypothesis and intended user benefit

Operators and reviewers need a fast way to explain why a session exists or why
a snapshot was captured without changing vital-sign logic or opening a broader
charting workflow. A short note can reduce later ambiguity during internal
review, training, demo use, or operational handoff when the vital signs alone
do not explain the session context.

## source evidence quality

Source evidence is moderate and sufficient for a narrow product hypothesis, not
for copying a competitor workflow.

- The Philips IntelliVue Information Center IFU is the strongest source in the
  issue. It is a manufacturer IFU for a patient-monitoring review product and
  supports the claim that contextual recording annotations can accompany review
  and print workflows.
- The Philips "notes in the status line" source appears to support the general
  concept of operator-entered contextual notes, but it is not a direct spec for
  this product and should not be treated as a workflow template.
- The Mindray BeneVision CMS Viewer source is product-discovery evidence only.
  It supports the general review/printing use case, but it is marketing-grade
  evidence rather than a detailed safety or usability specification.

Conclusion: the public evidence supports a defensible need for non-clinical
review context, but it does not justify mirroring any specific competitor UX,
terminology, or workflow.

## MVP boundary that avoids copying proprietary UX

- One free-text field only, capped at about 120 characters.
- One obvious entry path only; no threaded comments, multi-user collaboration,
  per-reading annotations, tagging, templates, or auto-suggestions.
- Session-scoped only; no cross-session history browser, search, or dashboard of
  prior notes.
- Neutral presentation only; do not replicate competitor status-line, print
  layout, or review-screen composition verbatim.
- If the note is included in any summary or export, it must appear as a plainly
  labeled optional context field rather than as a proprietary-style annotation
  workflow.

## clinical-safety boundary and claims that must not be made

- The note must be labeled as non-clinical session context.
- It must not affect vital-sign classification, NEWS2 scoring, alarm limits,
  alarm escalation, or any recommendation or response-band logic.
- It must not be presented as charting, diagnosis, treatment advice, validated
  clinical observation, or a legal medical record entry.
- It must not imply clinician review, approval, or system-generated meaning.
- It must not become a workaround for free-form clinical documentation.

## whether the candidate is safe to send to Architect

Yes. The issue has enough information to proceed to architecture if the design
stays within the non-clinical boundary above and creates a new requirements
slice for note entry, storage scope, reset behavior, display labeling, and any
summary/export behavior.

## medical-safety impact

Direct medical-safety impact is low because the proposed note is not part of
measurement, classification, or alerting. The real hazard is indirect: a note
can be mistaken for clinically meaningful documentation or can persist onto the
wrong patient/session if the design does not isolate and clear it correctly.
That would create review confusion and could contribute to a wrong-context
handoff or a mistaken belief that the monitor recorded or endorsed the text.

## security and privacy impact

The feature introduces a small but real privacy and integrity surface because it
adds a new free-text field associated with a patient session. The note could
contain PHI, inappropriate language, or quasi-clinical instructions entered by
an operator. Existing authentication and local-workstation controls reduce the
surface, but the design must still constrain length, scope, reset behavior, and
display/export paths so the note does not leak across patients or become an
unlabeled data-provenance problem.

## affected requirements or "none"

Current approved UNS/SYS/SWR: none.

If this feature is accepted, design should add a bounded requirement slice for:
session-note capture, session-note storage scope, reset/clear semantics, display
labeling, any print/export presentation, and explicit exclusion from alerting
and scoring logic.

## intended use, user population, operating environment, and foreseeable misuse

- Intended use: allow an authenticated operator to attach one short non-clinical
  context note to the active monitoring session.
- User population: trained clinical staff, demo operators, and internal
  reviewers using the Windows workstation application.
- Operating environment: local desktop monitoring session within the existing
  authenticated GUI workflow.
- Foreseeable misuse: entering treatment advice, clinical conclusions, or chart
  narrative into the note.
- Foreseeable misuse: assuming the note is system-generated, clinically
  validated, or part of the alert status.
- Foreseeable misuse: stale note carryover after patient refresh, Clear Session,
  logout, or a new admission.
- Foreseeable misuse: including PHI or unnecessary identifying detail in a field
  intended only for brief operational context.

## severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

- Severity: minor to serious, depending on failure mode. A neutral display-only
  note is low concern, but wrong-patient persistence or clinical misreading of a
  free-text note can contribute to a harmful context error.
- Probability: low to occasional without controls because the operator is
  manually entering free text and the feature would sit close to clinically
  meaningful UI elements.
- Initial risk: medium for this pilot because the hazard is informational rather
  than algorithmic, but it can still mislead humans if poorly scoped.
- Risk controls: separate the note from vitals and alerts in the data model;
  label it as non-clinical; cap length and reject multiline/control-character
  input; clear it on Admit/Refresh, Clear Session, and Logout; ensure it does
  not feed classification, NEWS2, alarm, summary-status, or recommendation
  logic; and keep one narrow edit path only.
- Verification method: review the changed file list to confirm no scoring or
  alert logic changed; add unit coverage for storage bounds and reset behavior;
  add integration/manual GUI checks for entry, display, clear/reset, and logout
  behavior; and verify any summary/export output labels the field explicitly as
  non-clinical session context.
- Residual risk: low after the controls above.
- Residual-risk acceptability rationale: acceptable for the pilot only if the
  design remains clearly non-clinical, session-scoped, and isolated from
  decision-support and alerting behavior. If the feature expands into charting,
  clinical instructions, or durable cross-session documentation, this risk
  conclusion no longer applies.

## hazards and failure modes

- The note is mistaken for a clinically validated observation, plan, or order.
- The note remains attached to the wrong patient after Admit/Refresh, Clear
  Session, logout, or a later session start.
- The note is displayed too close to the alert banner or in the same visual
  language, causing users to infer clinical significance.
- The note is silently included in a patient summary, printout, or export
  without a clear non-clinical label.
- Long or unbounded text truncates unpredictably and hides important context or
  overlaps safety-relevant UI.
- Free-text content introduces PHI, inappropriate language, or treatment advice
  into a field that was not designed as chart documentation.

## existing controls

- The issue already states that the feature must not affect thresholds, alerting,
  diagnosis logic, or recommendations.
- The current product architecture separates patient/vital-sign logic from GUI
  presentation, which makes non-clinical isolation feasible if preserved.
- The current product does not already expose a session-note path, so there is
  no legacy behavior that must be preserved for backward compatibility.
- The issue proposes a short note and a single simple path, which already
  constrains complexity compared with broader annotation systems.

## required design controls

- Introduce a dedicated session-context field rather than reusing alert, status,
  or patient-demographic fields.
- Apply an explicit label such as "Session note (non-clinical)" anywhere the
  value is shown.
- Keep the note visually distinct from the aggregate alert banner and severity
  colours.
- Enforce the length cap and reject line breaks or control characters.
- Default the note to empty on every new session, patient refresh, clear, and
  logout path.
- Ensure the value is never consumed by alert generation, NEWS2, triage cues,
  trend logic, or summary-status computation.
- Require any future print/export inclusion to use the same non-clinical label
  and to remain optional and bounded.
- Add traceability for the new feature instead of piggybacking on existing
  clinical display requirements.

## validation expectations

- Add or update requirements before implementation so the note behavior is
  explicitly specified and traceable.
- Add unit verification for character-cap enforcement and reset semantics.
- Verify in GUI review that the note can be entered through one path only and is
  cleared on Admit/Refresh, Clear Session, and Logout.
- Verify that the note does not alter alert level, NEWS2 score, status banner
  text, or alarm-limit behavior.
- Verify any printed or exported summary either omits the note or labels it as
  non-clinical session context.
- Review the final diff to confirm the implementation does not broaden into
  charting, collaboration, or cross-session history.

## residual risk for this pilot

Low, if implemented as a narrow session-context aid. The residual risk is
primarily human misuse of free text, not algorithmic or clinical-processing
failure. That residual risk is acceptable for the pilot only when the feature
remains short, clearly labeled, session-bound, and excluded from clinical logic
and formal documentation claims.

## human owner decision needed, if any

Yes. The product/clinical owner should make two explicit decisions before
implementation completes:

- Whether the note is write-once after save or whether one controlled edit path
  is allowed.
- Whether the note is omitted from printed/exported summaries by default, or
  shown only with an explicit non-clinical label.
