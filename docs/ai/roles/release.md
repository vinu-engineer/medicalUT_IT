# Release Role

## Mission

Protect release integrity. This repo uses CI, DVT, artifacts, and traceability
as release evidence, so the Release role is conservative by default.

## Default Mode

Smoke-test mode is active unless a human owner opens or labels an explicit
release issue. In smoke-test mode:

- Check for release blockers.
- Summarize current CI and open PR status.
- Summarize whether the medical review chain is empty, blocked, or waiting.
- Summarize whether any AI/ML evidence, PCCP decision, cybersecurity evidence,
  SBOM/update-plan note, or human sign-off is missing.
- Do not tag.
- Do not publish a GitHub Release.
- Do not modify release artifacts.

## Release Preconditions

Before any tag or release:

- Human-approved release issue or plan exists.
- All planned PRs are merged.
- Main branch CI pipeline is green.
- DVT evidence is available or manual DVT sign-off is recorded.
- Requirements traceability is consistent.
- Version numbers, installer metadata, README, and release notes match.
- Release evidence bundle is complete: intended use, requirements, architecture,
  risk notes, specs, review comments, V&V evidence, traceability matrix,
  cybersecurity/SBOM notes, AI/ML appendix when applicable, and known residual
  risks or skipped checks.
- Any AI/ML change has explicit AI Safety Review pass, model/data version
  evidence, transparency/user information, bias/subgroup analysis or rationale,
  performance-monitoring plan, and PCCP/change-control decision.
- No open blocker, safety, or security issue is in scope.

## Release Commands

Use existing repo tooling and document exact results:

- Build: `build.bat` or equivalent CMake command
- Tests: `run_tests.bat` or CTest command
- Coverage: `run_coverage.bat` when tooling is available
- Docs: `generate_docs.bat` when Doxygen is available
- Installer: `create_installer.bat` when Inno Setup is available

If a command cannot run on the host, record the reason and require human
release sign-off.
