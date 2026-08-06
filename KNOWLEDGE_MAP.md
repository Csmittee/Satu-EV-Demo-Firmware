# KNOWLEDGE_MAP.md
> Version 1.0 — 2026-08-06

| File | What it is |
|---|---|
| `Satu_EV_Demo.ino` | setup()/loop() only — wires the headers below together |
| `config.h` | Pins, RGB565 colors, 6 placeholder menu items, UI strings (`g_lang`) |
| `display.h` | QSPI bus + AXS15231B + `Arduino_Canvas` init, raw-I2C touch read, backlight control |
| `state_machine.h` | `AppState` enum + `AppContext` + `transitionTo()` |
| `ui_screens.h` | Draw + touch-handler pair for each of the 6 screens, double-tap demo bypass gesture, top-level `tickScreen()`/`handleTouch()` dispatch |
| `qr.h` | QRCode library wrapper — builds the placeholder payload string and draws the full-screen QR + amount overlay |
| `.github/workflows/compile-check.yml` | arduino-cli compile in a GitHub Actions runner, uploads bootloader/partitions/app `.bin` as an artifact |
| `CLAUDE.md` | Repo compass — what this repo is/is not |
| `RULES.md` | R-1 through R-7 |
| `LIBRARY_axs15231b.md` | Arduino_GFX/AXS15231B onboarding — version pinned, why, pin sources, touch protocol |
| `LIBRARY_qrcode.md` | QRCode library onboarding — API surface, version, usage pattern |
| `PROJECT_STATE.md` | Session-by-session build status |
| `CC_CHAT_LOG.md` | Session log for Chat to read between sessions |
| `README.md` | Setup/flash instructions for the owner |
