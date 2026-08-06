# docs/prompts/ — CC prompt workflow

> Version 1.0 — 2026-08-06

Two folders, one direction of travel:

- **`inbox/`** — drop new CC prompts here. This is the only place new,
  unprocessed prompts should land. Nothing in here has been acted on yet.
- **`archive/`** — where CC moves a prompt once its session's work is
  done, stamped `✅ COMPLETE — [date] — [summary]` at the top of the
  file. Nothing in here is pending action.

**Owner:** send new prompts to `docs/prompts/inbox/CC_PROMPT_<name>.md`
(e.g. via GitHub's "Add file" upload, same as before) — not to the repo
root and not directly into `archive/`. That's what caused the mix-up
this convention replaces: a fresh, unprocessed prompt (`selftest_firmware_v1`)
landed in the same flat `docs/prompts/` folder as an already-completed,
stamped one (`ev_demo_bootstrap`), with nothing distinguishing them.

**CC:** on session start, look in `inbox/` for the prompt named in the
instruction. At session close, move it to `archive/` with the completion
stamp — never leave a processed prompt sitting in `inbox/`, and never
place a new, unprocessed prompt directly into `archive/`. See RULES.md
R-8.
