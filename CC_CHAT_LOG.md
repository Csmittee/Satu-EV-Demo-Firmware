# CC_CHAT_LOG.md
> Version 1.4 — 2026-08-11

---
## [2026-08-11] — CC_PROMPT_fix_back_collision_and_instrument_v1 (from docs/prompts/inbox/)
**Did:** Bug 1 (confirmed, owner-reproduced): repositioned selftest's
`<Back>` zone from top-left to top-center, provably clear of all 5 Test
2 targets via `static_assert` — not eyeballed. Notable wrinkle: re-
reading the OLD code, the old Back rect and TR's target circle had a
~212px gap in pixel space — no actual overlap found. Flagged this
openly rather than silently implementing the requested fix as if the
prompt's framing were verified fact — see LIBRARY_axs15231b.md "Open
question." Implemented the reposition anyway since it's a safe fix
regardless of the true mechanism. Section 3 (unresolved slowness/Test 3)
NOT guess-fixed — added always-on instrumentation instead (differentiated
I2C failure logging + counter, per-touch raw x/y logging, flush()
duration via new loggedFlush() wrapper at all call sites except Test 4's
own loop, loop()-over-100ms logging) in both display.h copies,
ui_screens.h, qr.h, selftest/tests.h, both .ino files.
**Updated:** RULES.md (R-10, R-11), LIBRARY_axs15231b.md ("Open
question" section), PROJECT_STATE.md.
**New files:** none.
**Pending Chat verify:** owner must re-run Test 2 (all 5 targets
including TR) and Test 3, and if slowness recurs, send the Serial log
this time — format in RULES.md R-11. If Test 2's raw values are still
far from expected in a consistent pattern (e.g. axes swapped/inverted),
that's the coordinate-mapping question in LIBRARY_axs15231b.md, next.
**Flags:** OVERRIDE-adjacent — did not silently accept the prompt's Bug
1 framing ("straightforward layout collision") without checking; found
no pixel-space overlap in the code that produced the report, documented
the discrepancy, and implemented the requested fix anyway since it's
correct regardless of mechanism. No RULES.md rule removed. R-9 (reset
timing, PR #3) explicitly left untouched per the prompt's DO NOT TOUCH
list — no evidence yet points at it.
---
## [2026-08-08] — CC_PROMPT_fix_touch_garbage_read_v1 (from docs/prompts/inbox/)
**Did:** Root cause confirmed for the garbage `4095,4095` touch reads
(owner-captured Serial evidence: all 4 corner taps in Test 2 returned
identical max-value reads) — the touch reset sequence in `displayInit()`
was too short (20/20/50ms) vs. Espressif's own `esp_lcd_axs15231b`
reference driver (200ms/200ms). Fixed reset timing in both `display.h`
and `selftest/display.h`. Did NOT add a defensive garbage-value filter
— the prompt explicitly asked for one grounded fix, not both, and the
reference driver needs no such filter when reset timing is correct.
**Updated:** RULES.md (R-9), LIBRARY_axs15231b.md ("Touch reset timing"
section + known issues), PROJECT_STATE.md.
**New files:** none.
**Pending Chat verify:** owner must re-flash `Satu_EV_SelfTest`, re-run
Test 2/3, confirm no more `4095,4095` and Test 3 responds on first tap.
If the demo still feels "slow" after this, that may be Test 4's
flush() timing — a separate concern this fix doesn't address.
**Flags:** No unexecuted "instrumentation prompt" was sitting in
`docs/prompts/inbox/` (checked, empty) — the prompt's Section 1
contingency for that didn't apply. The prompt itself was uploaded
directly to `docs/prompts/` root again (not `inbox/`, commit `b0930ea`)
— moved to `inbox/` before processing, per R-8. No RULES.md rule was
removed or changed, only added (R-9).
---
## [2026-08-06] — CC_PROMPT_selftest_firmware_v1 (from docs/prompts/inbox/)
**Did:** Built `selftest/` — a standalone diagnostic sketch (6 tests:
display sanity, touch calibration, isolated Confirm-button hit-test,
flush() timing, double-tap isolated, QR render isolated) to isolate
whether the reported Confirm-button bug is hardware/coordinate or
application logic, before debugging the demo further. Does not touch
`Satu_EV_Demo.ino` or its headers. Extended CI to compile both sketches.
Also reorganized `docs/prompts/` into `inbox/`/`archive/` per owner
request (separate from this prompt's own scope).
**Updated:** RULES.md (R-8, prompt workflow — not test-related, per
owner's separate request), PROJECT_STATE.md, KNOWLEDGE_MAP.md, CLAUDE.md,
`.github/workflows/compile-check.yml`.
**New files:** `selftest/Satu_EV_SelfTest.ino`, `selftest/config.h`,
`selftest/display.h`, `selftest/tests.h`, `selftest/qrcode_lib.h`,
`selftest/qrcode_lib.c`, `docs/prompts/README.md`.
**Pending Chat verify:** nothing yet — this session produces diagnostic
data, not a fix. Waiting on the owner to run all 6 tests on the physical
board and report back (see PROJECT_STATE.md for exactly what to report).
**Flags:** OVERRIDE — prompt's Section 3 file list didn't itemize a
vendored qrcode_lib copy for `selftest/`, but Test 6 needs one to
compile standalone now that the repo vendors QRCode instead of
installing it via Library Manager (see LIBRARY_qrcode.md, added last
session after this prompt would have been written). Copied
`qrcode_lib.h`/`.c` into `selftest/` to match. No RULES.md entry about
the actual bug yet — root cause depends on Test 2/3 results, per the
prompt's own instruction to wait for the next session.
---
## [2026-08-06] — CI fix: qrcode.h collision (PR #1 follow-up)
**Did:** First CI run failed — `#include <qrcode.h>` was silently
resolving to the ESP32 Arduino core's own bundled qrcode component
header (used for RMakerQR provisioning) instead of ricmoo/QRCode,
because arduino-cli never added the Library-Manager-installed QRCode
library to the build (confirmed via "Used library" table omitting it).
Fixed by vendoring ricmoo/QRCode v0.0.1 into the repo as
`qrcode_lib.c`/`qrcode_lib.h` (quote-included, so it can't lose to the
core's angle-bracket header again) and dropping the `arduino-cli lib
install "QRCode"` CI step.
**Updated:** `qr.h` (include path), `.github/workflows/compile-check.yml`
(cp list + install step), `LIBRARY_qrcode.md` (KT writeup + version
note), `RULES.md` R-6 (vendoring note), `KNOWLEDGE_MAP.md`.
**New files:** `qrcode_lib.h`, `qrcode_lib.c`.
**Pending Chat verify:** none beyond original entry — this is a same-day
compile fix, not a new scope decision.
**Flags:** KT invoked (SKILL 6) — IS: `<qrcode.h>` resolves to a
different, ESP-IDF-provided header; IS NOT: a library install failure.
---
## [2026-08-06] — CC_PROMPT_ev_demo_bootstrap
**Did:** Bootstrap build. Created `Satu_EV_Demo.ino`, `config.h`,
`display.h`, `state_machine.h`, `ui_screens.h`, `qr.h`,
`.github/workflows/compile-check.yml`. 6 screens, offline demo bypass
gesture (R-7), placeholder QR payload (R-5), no WiFi/backend (R-4).
**Updated:** RULES.md (R-1 through R-7, new), CLAUDE.md, PROJECT_STATE.md,
KNOWLEDGE_MAP.md — all created fresh this session.
**New files:** All firmware/CI/doc files listed above — see
KNOWLEDGE_MAP.md for the full index. `CC_SKILL.md` and `README.md`
already existed (owner-provided); `README.md` rewritten with setup/flash
instructions.
**Pending Chat verify:** Owner needs to confirm QSPI/backlight pins
(maintainer-preset-sourced, higher confidence) and touch I2C pins
(community-sourced, lower confidence, TOUCH_INT has a conflicting
report) against the physical unit on first flash. ESP32 core version
(starting 2.0.17) is unconfirmed for this display combo until CI proves
otherwise.
**Flags:** Pinned Arduino_GFX to v1.6.0 instead of latest (v1.6.7) due to
an unresolved-in-tracker regression report (issue #803) for this exact
board — see LIBRARY_axs15231b.md. No structural-change or KT triggers
this session (brand new repo, nothing to conflict with).
---
