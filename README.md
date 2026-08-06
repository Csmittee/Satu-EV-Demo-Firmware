# Satu-EV-Demo-Firmware

A standalone, fully offline sales-demo firmware proving the Satu payment
UX (product select → QR pay → dispense) generalizes to EV charging
session sales. Runs on a Guition JC3248W535C (ESP32-S3-WROOM-1-N16R8,
320×480 capacitive touch, AXS15231B combined display+touch controller).

**This firmware simulates payment only.** It never calls the Satu
backend or any payment processor, and never connects to WiFi. It must
never be modified to make a real payment call without a separate,
explicit owner-approved session — see Satu-Vending-Firmware's
PAYMENT_MODE pattern if/when this demo becomes a real integration.

This is a sales demo, not the real thing: 6 placeholder EV charging
items with placeholder prices (see `config.h`) and a QR code that LOOKS
like a payment payload but is not a real, scannable one (see
`LIBRARY_qrcode.md`, RULES.md R-5). Display and touch only in this
phase — no relays, no dispensing hardware.

---

## What's in this repo

See `KNOWLEDGE_MAP.md` for a one-line-per-file index, and `CLAUDE.md`
for how this repo relates to the unrelated sibling repo,
Satu-Vending-Firmware.

## No local Arduino IDE needed

You do not need to install Arduino IDE or manage a local Arduino library
folder for this project. GitHub Actions compiles the firmware in an
isolated cloud runner on every push and uploads the finished `.bin`
files as a downloadable artifact. You only need `esptool` installed
locally (you already have this for Satu 1.0 flashing) — nothing else.

Do **not** install this repo's libraries into your local Arduino
libraries folder — their pinned versions (Arduino_GFX 1.6.0, QRCode
0.0.1) would collide with Satu-Vending-Firmware's locked versions
(Arduino_GFX 1.4.9) if both ended up in the same folder. See RULES.md R-6.

## Build versions (current)

- **ESP32 Arduino core:** UNCONFIRMED for this exact display combo —
  starting from **2.0.17** (same as Satu 1.0, same WROOM-1-N16R8 chip
  family). Check the latest GitHub Actions run for whichever version it
  actually needed to go green; if it was bumped, that will be recorded
  in `LIBRARY_axs15231b.md`.
- **GFX Library for Arduino (Arduino_GFX):** pinned to **v1.6.0**, not
  the latest release. Reason: a reported, unresolved-in-tracker
  regression (GitHub issue #803) breaks AXS15231B QSPI init on this
  exact board in v1.6.1 and later. See `LIBRARY_axs15231b.md`.
- **QRCode (ricmoo):** **0.0.1** — this library's actual version, it has
  never moved past this number. See `LIBRARY_qrcode.md`.

## Getting the firmware

1. Push (or merge) triggers `.github/workflows/compile-check.yml`.
2. Once the run is green, open it in the Actions tab and download the
   `satu-ev-demo-firmware-<run number>` artifact.
3. Unzip it — you'll get three files:
   - `Satu_EV_Demo.ino.bootloader.bin`
   - `Satu_EV_Demo.ino.partitions.bin`
   - `Satu_EV_Demo.ino.bin`

## Flashing

```
esptool --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 460800 \
  write-flash \
  0x0     Satu_EV_Demo.ino.bootloader.bin \
  0x8000  Satu_EV_Demo.ino.partitions.bin \
  0x10000 Satu_EV_Demo.ino.bin
```

**Flag — needs your confirmation on first real flash:** the port name
and the three flash offsets above are standard convention for this
partition scheme, not yet verified against this specific board. If the
board enumerates as something other than a `usbserial` device (some
native-USB ESP32-S3 boards show up differently depending on OS/driver),
adjust `--port` accordingly.

**Flag — `CDCOnBoot=cdc`:** this board's Type-C port is native ESP32-S3
USB, not a separate UART bridge chip, so the compile FQBN uses
`CDCOnBoot=cdc` rather than Satu 1.0's `CDCOnBoot=default`. Worth
confirming this is right the first time you get a successful upload —
if the board doesn't enumerate or the serial monitor stays silent after
flashing, this is the first setting to double check.

## Pin mapping — confirm before first flash

The vendor spec sheet (`JC3248W535_Specifications-EN.pdf`) confirms the
display driver chip, resolution, and connector types, but does **not**
include a GPIO-level pinout schematic. The pins in `config.h` come from
two different levels of confidence:

- **QSPI bus + backlight** (`LCD_QSPI_*`, `GFX_BL`): sourced from
  Arduino_GFX's own bundled `JC3248W535` dev-device preset — the library
  maintainer's own validated pin set for this board, not a
  community guess.
- **Touch I2C** (`TOUCH_SDA`, `TOUCH_SCL`, `TOUCH_RST`, `TOUCH_INT`):
  community-sourced, not vendor-confirmed. One other community driver
  repo reports `TOUCH_INT = 3` instead of `11` for what claims to be the
  same board — if GPIO 11 produces no touch activity on your unit, try
  GPIO 3 next.

**Please confirm all of the above against the silkscreen labels and/or
seller's wiki/manual for your physical unit before first flash, and
report back any pin that doesn't match** — see `LIBRARY_axs15231b.md`
for the full detail and sourcing.

## Verification status

No automated test suite exists for firmware (unlike the backend's
14-test suite) — none is claimed here. CI green confirms the **code**
compiles against the pinned library/core versions. It does **not**
confirm the GPIO pins are physically correct for your actual unit — that
can only be confirmed by flashing and observing the screen/touch. Expect
2-3 physical flash cycles as pin mapping gets adjusted from what you
observe. See `PROJECT_STATE.md` for current status.
