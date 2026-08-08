# KNOWLEDGE_MAP.md
> Version 1.1 — 2026-08-06

| File | What it is |
|---|---|
| `Satu_EV_Demo.ino` | setup()/loop() only — wires the headers below together |
| `config.h` | Pins, RGB565 colors, 6 placeholder menu items, UI strings (`g_lang`) |
| `display.h` | QSPI bus + AXS15231B + `Arduino_Canvas` init, raw-I2C touch read, backlight control |
| `state_machine.h` | `AppState` enum + `AppContext` + `transitionTo()` |
| `ui_screens.h` | Draw + touch-handler pair for each of the 6 screens, double-tap demo bypass gesture, top-level `tickScreen()`/`handleTouch()` dispatch |
| `qr.h` | QR wrapper — builds the placeholder payload string and draws the full-screen QR + amount overlay, using the vendored `qrcode_lib` |
| `qrcode_lib.h` / `qrcode_lib.c` | Vendored copy of ricmoo/QRCode v0.0.1 (MIT), renamed to avoid a filename collision with the ESP32 core's own bundled `qrcode.h` — see `LIBRARY_qrcode.md` |
| `.github/workflows/compile-check.yml` | arduino-cli compile in a GitHub Actions runner, uploads bootloader/partitions/app `.bin` as an artifact |
| `CLAUDE.md` | Repo compass — what this repo is/is not |
| `RULES.md` | R-1 through R-8 |
| `LIBRARY_axs15231b.md` | Arduino_GFX/AXS15231B onboarding — version pinned, why, pin sources, touch protocol |
| `LIBRARY_qrcode.md` | QRCode library onboarding — API surface, version, usage pattern, vendoring |
| `PROJECT_STATE.md` | Session-by-session build status |
| `CC_CHAT_LOG.md` | Session log for Chat to read between sessions |
| `README.md` | Setup/flash instructions for the owner |
| `selftest/Satu_EV_SelfTest.ino` | Diagnostic sketch entry point — setup()/loop() for the 6 hardware self-tests |
| `selftest/display.h` | Copy of the real `display.h` (does not auto-sync — see file header) |
| `selftest/config.h` | Copy of the real `config.h`, pins/screen dims only (does not auto-sync) |
| `selftest/tests.h` | The 6 diagnostic tests — display sanity, touch calibration, isolated Confirm-button hit-test, flush() timing, double-tap isolated, QR render isolated |
| `selftest/qrcode_lib.h` / `selftest/qrcode_lib.c` | Copy of the root `qrcode_lib.*` (needed for Test 6 to compile standalone — does not auto-sync) |
| `docs/prompts/inbox/` | New, unprocessed CC prompts land here — see RULES.md R-8 |
| `docs/prompts/archive/` | Completed CC prompts, stamped, live here |
| `docs/prompts/README.md` | Explains the inbox/archive convention |
