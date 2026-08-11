# RULES.md
> Version 1.3 — 2026-08-11
> R-10, R-11 added — hit-rect overlap checks must be programmatic;
> always-on touch/flush/loop diagnostic logging

---

- **R-1**: GFX Library for Arduino (Arduino_GFX) is pinned to **v1.6.0** —
  do not upgrade without re-reading `LIBRARY_axs15231b.md`. v1.6.1+ has a
  reported, unresolved-in-the-tracker regression breaking AXS15231B QSPI
  init on this exact board (GitHub issue #803).

- **R-2**: Never render directly via `Arduino_AXS15231B` — always through
  `Arduino_Canvas` (the `gfx` object in `display.h`), and always call
  `gfx->flush()` after drawing a screen. Nothing appears on the physical
  panel until `flush()` is called. Direct rendering is reported unstable
  by multiple independent community sources for this display.

- **R-3**: Pin mapping (QSPI bus, backlight, touch I2C) is
  community-sourced, not vendor-confirmed — the vendor spec sheet has no
  GPIO-level pinout. Verify against the physical unit before every new
  board of this type, and before trusting any pin number in `config.h`.
  `TOUCH_INT` in particular has a conflicting community report (GPIO 3
  vs GPIO 11) — see `LIBRARY_axs15231b.md`.

- **R-4**: Fully offline by design — no WiFi/backend calls in this repo.
  A sales pitch should not depend on venue WiFi. Do not add WiFi
  connection code or HTTP calls without a separate, explicit
  owner-approved session (see PAYMENT_MODE note in README.md/CLAUDE.md).

- **R-5**: QR content generated in `qr.h` is a visual placeholder only —
  it looks like a payment-style payload but is NOT a real EMVCo-compliant
  string and NOT a real merchant ID. Never present it as a scannable real
  payment QR.

- **R-6**: Compile via GitHub Actions only — never instruct the owner to
  install these libraries in a local Arduino library folder. This repo's
  library versions (Arduino_GFX 1.6.0, QRCode 0.0.1) would collide with
  Satu-Vending-Firmware's locked versions (Arduino_GFX 1.4.9) if both
  were installed into the same local Arduino libraries directory. Note:
  QRCode itself is vendored (`qrcode_lib.c`/`.h`), not Library-Manager-
  installed at all — its header name collides with the ESP32 core's own
  bundled `qrcode.h`. See `LIBRARY_qrcode.md`. Do not "fix" this by
  reinstalling it via Library Manager.

- **R-7**: The QR and Processing screens advance via a **double-tap demo
  bypass gesture** (two sequential single-finger taps within 400ms), not
  a real trigger — this simulates external events (a customer's QR scan,
  a payment webhook) that this offline demo has no way to receive for
  real. It must be removed entirely, not just hidden, before any real
  backend integration. See `ui_screens.h` for the implementation and
  Section 4.4a of the bootstrap prompt for the full reasoning, including
  why it's single-tap-based rather than a real two-finger gesture (the
  Arduino touch path for this chip is single-touch only).

- **R-8**: New CC prompts go in `docs/prompts/inbox/` — never the repo
  root, never directly into `docs/prompts/archive/`. Once a prompt's
  session is done, CC moves it to `docs/prompts/archive/`, stamped
  `✅ COMPLETE — [date] — [summary]` at the top of the file. Never leave
  a completed prompt sitting in `inbox/`, and never let an unprocessed
  prompt sit in `archive/` — the two folders exist specifically so a
  glance tells you which is which. See `docs/prompts/README.md`.

- **R-9**: The AXS15231B touch reset sequence in `displayInit()` (both
  `display.h` and `selftest/display.h`) must hold `TOUCH_RST` low for at
  least 200ms and wait at least 200ms after releasing it before the
  first touch query — a shorter sequence (originally 20ms/20ms/50ms)
  let `readTouch()` query the chip before its post-reset boot finished,
  returning garbage (`0xFF` bytes, read as `x=y=4095`) that got treated
  as a real touch-down. Confirmed against Espressif's own
  `esp_lcd_axs15231b` reference driver, which uses 200ms on both sides
  and does no defensive garbage-value filtering — meaning correct reset
  timing, not read validation, is the actual fix. Do not "fix" a
  recurrence of `4095,4095` reads by adding a discard-garbage-values
  filter instead of checking this timing first. See
  `LIBRARY_axs15231b.md` ("Touch reset timing").

- **R-10**: Any new touch-interactive control (button, back-zone, test
  target, menu row) must have its hit-rect checked against every other
  hit-rect on the same screen for overlap **programmatically** — a
  `static_assert` (see `rectsOverlap()` in `selftest/tests.h`) or
  equivalent compile-time/boot-time check, not eyeballed coordinates.
  Origin: the self-test's `<Back>` zone was placed without checking it
  against Test 2's 5 calibration targets, and the owner reported tapping
  TR being consumed as "exit test" — re-reading the code afterward
  showed no actual pixel-space overlap existed, meaning the check would
  have passed trivially if it had existed, but its total *absence* meant
  nobody could point to that fact quickly when debugging. Always add the
  check, even when the geometry looks obviously fine.

- **R-11**: Touch I2C transactions, `flush()` calls, and `loop()`
  iterations are always-on instrumented — not a temporary debug
  session's logging, don't remove it after a fix lands:
  - Every touch I2C failure logs which step failed
    (`Wire.endTransmission()` non-zero vs `requestFrom()` byte-count
    mismatch are different failure modes, logged differently), plus a
    running failure counter logged every 50th failure
  - Every real touch-down logs raw `x,y`
  - Every `gfx->flush()` call goes through `loggedFlush()` (see
    `display.h`), which logs its duration — the one exception is Test 4
    (flush timing) in `selftest/tests.h`, which calls `gfx->flush()`
    directly since it already has its own dedicated per-call logging
  - Any `loop()` iteration over 100ms logs its duration
  See `LIBRARY_axs15231b.md` and `docs/prompts/archive/
  CC_PROMPT_fix_back_collision_and_instrument_v1.md` Section 3 for the
  exact format. If new draw functions or touch paths are added, wire
  them into this logging the same way — don't let new code bypass it.
