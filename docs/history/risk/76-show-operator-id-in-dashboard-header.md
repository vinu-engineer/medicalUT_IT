# Risk Note: Issue 76

Issue: `#76`  
Branch: `feature/76-show-operator-id-in-dashboard-header`

## proposed change

Add a read-only operator-identity label in the dashboard header next to the
existing role badge so the active authenticated session is easier to recognize
on a shared workstation.

Repo evidence shows the session already carries both a display name and an
account identifier:

- `src/gui_main.c` stores `g_app.logged_username` at login and already clears it
  on logout.
- `src/gui_main.c` currently paints `g_app.logged_user` (display name) and the
  role badge in the header.
- `src/gui_users.c` resolves `display_name` from the authenticated username.

This means the candidate is a bounded presentation-layer change that should
reuse existing authenticated session state rather than introduce a new
authentication, storage, or audit subsystem.

## product hypothesis and intended user benefit

- Shared clinical workstations and handoffs create a real need to confirm which
  authenticated account is active before acting on the monitor.
- Showing a session identity label in the header can reduce ambiguity during
  handoff, review, and account-switch checks.
- The intended user benefit is operational traceability and reduced user
  confusion, not new clinical decision support.

## source evidence quality

Moderate but limited.

- Internal evidence is strong that the product already has the technical hooks
  needed for a low-risk display-only change: the login path records
  `logged_username`, the header already renders user-facing session context, and
  role-dependent UI behavior already exists.
- External product-discovery evidence is limited to a single competitor IFU:
  Philips EarlyVue VS30 Instructions for Use, which includes a User ID pane
  showing logged-on status and username behavior. This is useful as proof that
  session identity on a monitor UI is a plausible pattern, but it is not
  clinical evidence and should not be copied as a workflow or proprietary UX.
- The issue text slightly understates current behavior because this repository
  already paints a display name in the header. The remaining design question is
  not whether any session identity is shown, but whether the header should add a
  distinct operator ID and how that differs from display name.

## medical-safety impact

Direct clinical risk is low because the candidate does not alter vital-sign
acquisition, threshold logic, NEWS2 scoring, alarm routing, patient identity,
or treatment guidance.

There is still indirect safety relevance:

- On a shared workstation, clearer session identity can reduce the chance that a
  clinician assumes the wrong account is active during handoff or review.
- If implemented poorly, a wrong, stale, truncated, or ambiguous operator label
  could create misplaced confidence about who is currently logged in.
- If the header crowds out existing simulation, alarm, or patient context, it
  could degrade recognition of more safety-relevant status elements.

## security and privacy impact

- No new credential entry, privilege boundary, or network surface is needed if
  the UI reads only the existing authenticated session fields.
- Privacy risk is low to moderate and mostly local: exposing an operator ID in a
  visible header increases shoulder-surfing exposure if future account IDs are
  personally identifying, such as full names or email-style usernames.
- The feature must not display passwords, password hashes, patient identifiers,
  or any hidden account metadata.
- The feature must not imply that a durable operator activity log exists unless
  a separate audited logging feature is implemented.

## affected requirements or "none"

Existing requirements likely touched by design intent:

- `SYS-013` User Authentication Enforcement
- `SYS-017` Role-Based Access Control
- `SWR-GUI-002` Session Management (Login / Logout)
- `SWR-GUI-008` Role-Based UI Differentiation

New or revised requirements will likely be needed to define:

- the exact operator ID source field
- the label text and visual distinction from display name and role badge
- truncation/layout behavior
- logout clearing behavior
- privacy constraints on what identifier may be displayed

## intended use, user population, operating environment, and foreseeable misuse

- Intended use: help trained clinical staff recognize the currently active
  authenticated account on the dashboard.
- User population: ward clinicians and administrators already authorized to use
  the monitor.
- Operating environment: shared Windows workstation in a monitored clinical
  setting with logins, handoffs, and nearby observers.
- Foreseeable misuse:
  - treating the visible operator ID as proof of recent action ownership
  - assuming a displayed name and operator ID are interchangeable when they are
    different account fields
  - relying on the header instead of logging out before handoff
  - exposing an overly identifying account string to nearby observers

## severity, probability, initial risk, risk controls, verification method, residual risk, and residual-risk acceptability rationale

- Severity: Minor. The main harm is user confusion, privacy exposure, or loss of
  trust in session attribution, not direct clinical misclassification.
- Probability: Occasional if identity semantics, clearing behavior, or layout
  are underspecified.
- Initial risk: Low to Medium for the pilot, driven by ambiguity rather than
  clinical logic.
- Risk controls:
  - derive the displayed operator ID only from the authenticated session already
    stored in memory
  - define one authoritative field for "operator ID" and label it explicitly
  - keep the feature read-only with no new logon/logoff workflow in this MVP
  - clear the identifier immediately on logout and refresh it on every login
  - preserve visibility of existing header elements such as role badge,
    simulation status, logout/settings controls, and patient context
  - truncate or clip safely for long identifiers without overlapping safety
    indicators
  - prohibit use of patient identifiers, credentials, or implied audit claims
- Verification method:
  - GUI smoke on at least admin and clinical accounts confirming the header
    changes on login, clears on logout, and updates after account switching
  - layout checks at supported window sizes with long usernames/display names
  - confirmation that simulation/live status remains visible and unchanged
  - targeted tests or manual evidence that no new persistence, logging, or role
    escalation behavior was introduced
- Residual risk: Low if the controls above are implemented.
- Residual-risk acceptability rationale: acceptable for this pilot because the
  feature remains display-only, reuses existing session state, and can improve
  session-awareness without changing any patient-facing clinical behavior.

## hazards and failure modes

- Header shows the wrong operator ID after logout, failed login, or account
  switch.
- Header shows a display name when the requirement intends a stable account ID,
  leading to ambiguous attribution.
- Long identifiers overlap or hide simulation/live status, buttons, or other
  header signals.
- Designers copy a competitor interaction model that adds logon workflow,
  validation prompts, or action gating beyond the issue scope.
- Future usernames include email addresses or other high-exposure identifiers,
  causing avoidable privacy leakage on shared screens.
- Users infer that the visible header constitutes a complete audit trail of
  operator actions.

## existing controls

- The authentication subsystem already differentiates username, display name,
  and role.
- The login path already records `g_app.logged_username` and the logout path
  clears session identity state.
- The header already renders authenticated session context, which reduces the
  amount of new UI state needed.
- Role-based controls already exist, so the feature does not need to change
  authorization rules to show the identity label.
- The issue explicitly keeps authentication-model changes, activity-history
  persistence, and permissions changes out of scope.

## required design controls

- Define whether "operator ID" means username, employee-style account ID, or an
  already-approved display token. Do not leave this implicit.
- Keep any operator ID visually distinct from patient ID and from the role
  badge, for example through explicit labeling.
- Do not add or imply operator action history, nonrepudiation, or audit-log
  claims in the UI text or requirements.
- Reuse only existing authenticated in-memory session fields in this MVP. No
  new persistence or identity synchronization should be added.
- Preserve the current safety-critical header signals, especially simulation
  status and access to logout.
- If future product direction requires personally identifying account labels,
  obtain a human product/privacy decision on acceptable exposure and masking
  rules before implementation.

## MVP boundary that avoids copying proprietary UX

- Safe MVP: show one read-only operator identity string from the current session
  beside the existing role badge, using this product's own styling and layout.
- Not in MVP: interactive user-ID panes, new logon dialogs from the header,
  forced action gating, timeout workflows, dropdown account menus, or any
  replication of competitor toolbar structure.
- The competitor IFU should inform only the general product hypothesis that
  session identity on a monitor can be useful, not the exact UI mechanics.

## clinical-safety boundary and claims that must not be made

- The feature must be described as session-awareness and traceability support
  only.
- It must not claim to verify who performed each patient-care action unless a
  separate auditable action log exists.
- It must not claim to improve diagnosis, alert accuracy, alarm response time,
  or treatment efficacy.
- It must not alter the intended use of the monitor or introduce new clinical
  workflow dependencies.

## validation expectations

- Verify the chosen operator ID matches the authenticated account for admin and
  clinical logins.
- Verify the header clears identity state on logout and after failed login.
- Verify account switching updates the identity immediately without restart.
- Verify long operator IDs do not obscure role badge, simulation status, or
  header buttons.
- Verify no patient identifiers, passwords, or hidden account attributes are
  rendered.
- Verify requirements and test evidence distinguish display name from operator
  ID if both can appear.

## residual risk for this pilot

Low. The remaining risk is mainly local confusion or privacy overexposure if
identity semantics are defined poorly. Because the implementation can reuse
existing authenticated session state and stay strictly display-only, the pilot
can manage the risk through clear requirements and layout verification.

## whether the candidate is safe to send to Architect

Yes, with constraints.

The candidate is safe to send to Architect if the design stays within a
read-only session-identity display, explicitly defines what "operator ID"
means, preserves existing header safety cues, and avoids any claim that the new
label is an audit trail or action-attribution mechanism.

## human owner decision needed, if any

One human product/privacy decision is still needed:

- choose the approved identity token for display in the header
  (`username`, display name, or another sanctioned account label), and confirm
  whether that token is acceptable for shoulder-surfing exposure in the target
  clinical environment
