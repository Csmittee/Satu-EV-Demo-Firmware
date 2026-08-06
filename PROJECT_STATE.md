# PROJECT_STATE.md
> Version 1.1 — 2026-08-06

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
