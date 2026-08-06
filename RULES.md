# RULES.md
> Version 1.0 — 2026-08-06
> Bootstrap session — rules R-1 through R-7 created fresh, no prior history

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
