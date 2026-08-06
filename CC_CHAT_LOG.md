# CC_CHAT_LOG.md
> Version 1.1 — 2026-08-06

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
