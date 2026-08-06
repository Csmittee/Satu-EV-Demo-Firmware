# CLAUDE.md
> Version 1.0 — 2026-08-06

This repo is the **Satu EV Demo Firmware** — a standalone, offline sales
demo for the Guition JC3248W535C (ESP32-S3 + AXS15231B display/touch)
proving the Satu payment UX (select → QR → dispense) generalizes to EV
charging session sales. Display and touch only — no relays, no motors,
no real payment, no WiFi/backend calls. See RULES.md R-4, R-5.

This repo is **NOT** Satu-Vending-Firmware. Different hardware (this
board is ESP32-S3-WROOM-1-N16R8 + AXS15231B QSPI+touch; Satu 1.0 is
ESP32-8048S070C RGB panel + separate GT911 touch chip), different
customer, different theme, no relay/sensor hardware layer. Do not port
Satu 1.0's `display.h`, `ui.h`, `touch.h`, or `hardware.h` here — the bus
type, driver class, and touch protocol are all different. See:
https://github.com/Csmittee/Satu-Vending-Firmware for the unrelated
sibling repo.

Key files: `Satu_EV_Demo.ino` (setup/loop only) → `config.h` (pins,
colors, menu, strings) → `display.h` (QSPI/AXS15231B/Canvas + touch) →
`state_machine.h` (6-state enum) → `ui_screens.h` (6 screens) → `qr.h`
(placeholder QR generation). CI: `.github/workflows/compile-check.yml`.

This firmware simulates payment only. It must never be modified to make
a real payment call without a separate, explicit owner-approved session.
