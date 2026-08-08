# PROJECT_STATE.md
> Version 1.3 — 2026-08-08

---

## [2026-08-08] — Fix: touch reset timing (garbage 4095,4095 reads)

**Root cause found and fixed — grounded in owner-captured hardware
evidence, cross-checked against Espressif's official reference driver.**
See `LIBRARY_axs15231b.md` ("Touch reset timing") and RULES.md R-9 for
the full evidence chain.

Owner's Test 2 (touch calibration) Serial capture showed all four
corner taps returning identical `(4095, 4095)` — the max a 12-bit field
holds, consistent with all-`0xFF` bytes over I2C. `displayInit()`'s
touch reset sequence (20ms/20ms/50ms) was too short — Espressif's own
`esp_lcd_axs15231b` reference driver uses 200ms on both sides of the
reset toggle. Fixed in both `display.h` and `selftest/display.h`.

This plausibly explains the reported "menu inaccessible on some
screens, feels slow" too: a garbage max-value read consumed the
touch-edge state (`_wasTouched`), so the next genuine tap didn't
register as a new edge until the phantom touch "lifted" — which screen
appeared to work was down to whether a garbage read happened to land
first, not that screen's logic being broken.

**Not independently re-verified from this session** — no physical
hardware access here. Owner must re-flash `Satu_EV_SelfTest`, re-run
Test 2 and Test 3, and confirm no more `4095,4095` readings and that
Test 3 responds reliably on first tap. If the real demo's "slow" feel
persists after this fix, check Test 4's flush() timing separately —
this fix addresses the touch-read issue, not display flush speed.

---

## [2026-08-06] — Self-test firmware (diagnostic, not a fix)

**This session produces diagnostic data, not a verified fix. Results
depend entirely on what the owner observes when running each test on
hardware.**

Built `selftest/Satu_EV_SelfTest.ino` — a standalone diagnostic sketch,
separate from the real demo, to isolate whether the reported "Confirm
button unresponsive, Menu button works" bug is a hardware/coordinate
problem or an application-logic problem, before debugging the demo's
state machine further. 6 tests: display sanity, touch coordinate
calibration (5 targets, raw x/y read directly off `readTouch()`),
isolated hit-test at the exact broken Confirm-button geometry, flush()
timing (min/max/avg over 20 runs), double-tap bypass gesture in
isolation, QR render in isolation. Does not modify `Satu_EV_Demo.ino` or
its headers — a genuinely separate sketch, own copies of `display.h`,
`config.h`, and the vendored `qrcode_lib.*` (does not auto-sync, see
each file's header comment).

CI extended to compile both sketches (`compile-demo` and
`compile-selftest` jobs), each uploading its own artifact.

**Not resolved by this session, by design:** the Confirm-button bug
itself. This session's only job is producing evidence — Test 2 and
Test 3 are built specifically to reveal whether it's a touch
coordinate-range problem (bottom ~15% of panel, where the Confirm/Cancel
buttons live) or something else. The actual fix, and any resulting RULES.md
update, come in the next session once the owner reports back what Test
2/3 showed.

Also reorganized `docs/prompts/` into `inbox/` (new, unprocessed prompts)
and `archive/` (completed, stamped) — the previous flat structure let a
fresh prompt land in the same folder as an already-completed one with
nothing distinguishing them. See RULES.md R-8, `docs/prompts/README.md`.

---

## [2026-08-06] — CI fix: QRCode vendored (same-day follow-up)

First CI run on PR #1 failed: `qrcode.h` name collision with the ESP32
Arduino core's own bundled qrcode component (see `LIBRARY_qrcode.md`).
Fixed by vendoring ricmoo/QRCode as `qrcode_lib.c`/`qrcode_lib.h`
instead of installing via Arduino Library Manager. Library version
pinned to Arduino_GFX 1.6.0 is unaffected — that step of CI passed
cleanly on the first run.

---

## [2026-08-06] — Bootstrap build (Prompt 1 of 1)

**Built:** Full offline demo firmware for Guition JC3248W535C — 6 screens
(Welcome, Menu, Confirm, QR, Processing, Finish) driven by
`state_machine.h`, QSPI+AXS15231B display via `Arduino_Canvas`
framebuffer (`display.h`), raw-I2C single-touch read, placeholder QR
generation (`qr.h`), 6 placeholder EV menu items and neutral blue/white/
dark-grey theme (`config.h`), demo bypass double-tap gesture on QR/
Processing screens (`ui_screens.h`, R-7). GitHub Actions compile-check
workflow (`.github/workflows/compile-check.yml`) mirrors
Satu-Vending-Firmware's pattern as its own independent copy.

**Fully offline by design:** no WiFi connection code, no HTTP/backend
calls anywhere in this firmware. This is a deliberate reliability choice
for a sales pitch — the demo must not depend on venue WiFi. See RULES.md
R-4.

**Verified:** GitHub Actions compile-check workflow — see the workflow
run linked from the PR for this commit for current status.

**Not verified — cannot be, from this session:** the GPIO pin numbers in
`config.h`/`display.h` are community-sourced, not vendor-confirmed (no
GPIO-level pinout in the vendor spec sheet). CI green confirms the CODE
compiles against the pinned library/core versions — it does **not**
confirm the pins are physically correct for the owner's actual unit.
That can only be confirmed by the owner flashing and observing the
screen/touch. Do not conflate the two when reporting status. Expect 2-3
physical flash cycles as pin mapping is adjusted from what the owner
observes.

**Library versions pinned:**
- GFX Library for Arduino (Arduino_GFX): **v1.6.0** — NOT latest (v1.6.7)
  because of a reported, unresolved-in-tracker AXS15231B QSPI init
  regression in v1.6.1+ on this exact board (issue #803). See
  `LIBRARY_axs15231b.md`.
- QRCode (ricmoo): **0.0.1** (library's actual version, never moved past
  this). See `LIBRARY_qrcode.md`.
- ESP32 Arduino core: starting from **2.0.17** (same as Satu 1.0, same
  WROOM-1-N16R8 family) — UNCONFIRMED for this exact display combo. If
  CI fails on core-version grounds, that failure is the confirmation
  this board needs a newer core; bump and document why, do not guess.

**Scope:** display and touch only, per owner's explicit instruction — no
relays, no motors, no MCP23017, no GPIO output control. No product
images, no Thai strings this pass (structured to add later without a
redesign — see `g_lang` in `config.h`).
