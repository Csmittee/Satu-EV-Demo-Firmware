# CC_PROMPT_ev_demo_bootstrap.md
> ✅ COMPLETE — 2026-08-06 — EV demo bootstrap build
> Codename: General Online Payment Firmware 3.5
> Repo: NEW — first session in this repo, no CLAUDE.md/RULES.md exist yet
> Session type: Build (bootstrap + full demo)

---

## 1. CC INTRO

Read and execute: CC_PROMPT_ev_demo_bootstrap.md

New session. This is a BRAND NEW, EMPTY repo — ignore all context from
Satu-Vending-Firmware or Satu-vending-backend. Do not assume any of their
rules apply here except where this prompt explicitly says so.

Repo: https://github.com/Csmittee/Satu-EV-Demo-Firmware
(Owner: if you created the repo under a different name, tell Chat the
actual URL before next session — README below must match.)

This repo has no CLAUDE.md, RULES.md, or CC_SKILL.md yet — you are
creating them as part of this session (see Section 3). There is nothing
to read before starting except this prompt itself.

State "Bootstrap session — no prior governance files, creating fresh"
before writing any code.

---

## 2. CONTEXT

Owner (solo founder of the Satu temple-donation project) is meeting a
new customer who runs electric-vehicle charging stations. Goal: a
standalone hardware demo proving the Satu backend's donation/purchase
flow (product select → QR pay → dispense) generalizes to EV charging
session sales. This is a SALES DEMO ONLY — no real payment, no real
backend call, no WiFi requirement. It must look convincing running
disconnected on a table at a sales meeting.

Target hardware: Guition JC3248W535C ("W5" 3.5" series) — ESP32-S3-
WROOM-1-N16R8, 320×480 capacitive touch, AXS15231B combined display+
touch controller, QSPI display bus + I2C touch on the same chip.

This hardware is NOT the same as Satu 1.0's ESP32-8048S070C (RGB panel
+ separate GT911 touch chip). Do not reuse Satu 1.0's display.h, ui.h,
or touch code — the bus type, driver class, and touch protocol are all
different.

Owner has the vendor spec sheet (JC3248W535_Specifications-EN.pdf) —
confirms driver chip AXS15231B, 320×480, ESP32-S3-WROOM-1, 5V/~150mA,
connector types (HC1.0 4P, JST1.25 4P, JST1.25 8P IO, Speaker, Type-C,
battery). It does NOT include a GPIO-level pinout schematic — see
Section 4.2, pin mapping is still community-sourced, not vendor-
confirmed.

Compile strategy: this repo gets its OWN GitHub Actions compile-check
workflow, same pattern already proven on Satu-Vending-Firmware
(arduino-cli in an isolated cloud runner → .bin artifacts → owner
flashes with esptool, no local Arduino IDE involved at all). Two
separate repos means two fully isolated CI runners — GFX 1.4.9 in one,
GFX 1.6.x in this one, never on the same filesystem, zero conflict by
construction. See Section 5. Do NOT tell the owner to install anything
in a local Arduino library folder.

Scope for this phase, explicit: DISPLAY AND TOUCH ONLY. No relays, no
motors, no MCP23017, no GPIO output control of any kind — there is no
physical dispensing mechanism attached to this board and none is being
designed yet. Do not add anything resembling hardware.h's relay/sensor
abstraction. If a future session adds real hardware control, that is
its own separate scope decision, not an extension to assume here.

Sequence position: Prompt 1 of 1 (single build session, per owner
request — but firmware compile/flash iteration afterward is normal
and expected; see Section 6).

---

## 3. NEW FILES

Firmware:
- `Satu_EV_Demo.ino` — setup()/loop() only, includes the headers below
- `config.h` — pin definitions, colors, the 6 demo item names/prices
- `display.h` — QSPI bus + AXS15231B + Arduino_Canvas init, touch read,
  backlight control
- `qr.h` — QRCode library wrapper: generate + draw a full-screen QR
  from a text string
- `ui_screens.h` — the 6 screens (draw + touch-handler pair each)
- `state_machine.h` — state enum + transition table

CI:
- `.github/workflows/compile-check.yml` — arduino-cli compile in cloud
  runner, produces bootloader/partitions/app .bin as a downloadable
  artifact (see Section 5 — mirrors Satu-Vending-Firmware's existing
  workflow, this repo's own independent copy, not shared)

Docs (repo root):
- `CLAUDE.md` — ≤30 lines, repo compass (what this repo is, what it is
  NOT, link to Satu-Vending-Firmware for the unrelated sibling repo)
- `RULES.md` — start with R-1 through R-5 (see Section 4 for content)
- `LIBRARY_axs15231b.md` — library onboarding doc (R-121 pattern from
  the main project): what you read, what you tested, pin map used,
  known issues (1.6.1 bug), version pinned and why
- `LIBRARY_qrcode.md` — same pattern for the QRCode library
- `PROJECT_STATE.md` — one entry: this session, what's built, what's
  unverified
- `KNOWLEDGE_MAP.md` — one-line-per-file index of everything above
- `CC_CHAT_LOG.md` — this session's entry (see Section 7)
- `README.md` — setup instructions (see Section 6 content requirements)

No CC_SKILL.md — owner is copying that in manually from the sibling
repo (do not create a placeholder; leave it absent this session).

---

## 4. TASKS

### 4.1 — Library onboarding FIRST (do not skip)
Before writing any UI code:
1. Read moononournation/Arduino_GFX README + the AXS15231B/QSPI/Canvas
   source headers. Confirm current release version and whether 1.6.1's
   known issue (reported broken vs 1.6.0 for this exact board) is
   fixed in later releases — pin to whichever version you confirm
   works, document the reason in LIBRARY_axs15231b.md.
2. Read the QRCode library (Richard Moore / "QRCode" in Library
   Manager) README and its simplest example before wiring it into
   qr.h. Document buffer-size/version/ECC-level function signatures
   actually exported by the installed version in LIBRARY_qrcode.md —
   do not guess the API surface.
3. Write both LIBRARY_*.md files before touching ui_screens.h.

### 4.2 — display.h
- Bus: `Arduino_ESP32QSPI`, display: `Arduino_AXS15231B`, wrapped in
  `Arduino_Canvas(320, 480, ...)` — framebuffer pattern, NOT direct
  AXS15231B rendering (confirmed unstable by community reports).
- Every screen redraw must end with an explicit `gfx->flush()` call —
  nothing appears on the physical panel until flush() is called.
- Touch: I2C at address 0x3B, shared chip with the display. Pin
  candidates from a working community reference for this exact board
  (GFX_BL=1, TOUCH_SDA=4, TOUCH_SCL=8, TOUCH_RST=12, TOUCH_INT=11,
  QSPI bus pins 45/47/21/48/40/39) — these are COMMUNITY-REPORTED for
  the JC3248W535C_I_Y variant, not verified against the owner's
  physical unit. Flag this explicitly in README: owner must confirm
  against the seller's wiki/manual before first flash, and report
  back any pin that doesn't match silkscreen labels.
- Prefer checking Arduino_GFX's own bundled dev-device presets
  (PDQgraphicstest → Arduino_GFX_dev_device.h) for an existing
  JC3248W535/W535 entry before hand-wiring pins — if the library
  maintainer already validated this board, use that instead of the
  community pin set above.

### 4.3 — config.h
6 placeholder EV charging items (owner will replace with real menu
before the customer meeting — flag this clearly as placeholder data):
1. Quick Top-up (15 min) — ฿100
2. Standard Charge (30 min) — ฿180
3. Fast Charge (45 min) — ฿250
4. Full Charge (60 min) — ฿320
5. DC Rapid (20 min) — ฿280
6. Overnight (8 hr) — ฿450

Color theme: neutral blue/white/dark-grey EV aesthetic — do NOT reuse
the temple gold/black theme, this is a different customer's brand.

### 4.4 — state_machine.h + ui_screens.h — the 6 screens
1. **Welcome** — title + "Tap to begin"
2. **Menu** — 6 items, 2×3 or 3×2 touch grid, name + price per tile
3. **Confirm** — selected item + price, Confirm / Cancel buttons
4. **QR** — full-screen QR code (see 4.5), amount overlay, no other
   chrome. Advances on the demo bypass gesture (4.4a) — a double-tap
   anywhere on this screen simulates "customer scanned and paid,"
   moving to Processing.
5. **Processing** — a looping visual only (progress bar or dot
   animation, no fixed duration) — no network/backend call of any
   kind. Advances on the demo bypass gesture (4.4a) — a double-tap
   anywhere on this screen simulates "payment confirmed, product
   dispensed," moving to Finish.
6. **Finish** — item + amount summary, "Thank you", auto-return to
   Welcome after ~5 sec or on tap

No product images, no photos, no Thai strings this pass — shapes and
text only, per owner's explicit scope. Structure the string constants
so Thai can be added later without a redesign (mirror the
`g_lang_th` / separate-strings-file pattern from Satu 1.0's ui.h,
lightweight version — don't build the full bitmap-font machinery,
just don't hardcode English strings inline where a translation would
need to slot in later).

### 4.4a — Demo bypass gesture (owner requirement, not a real trigger)

There is no backend, no WiFi, no way for the board to know a real scan
or a real payment happened — this is offline by design (4.6). For a
live sales pitch, the person demoing it needs to control pacing
themselves rather than wait on a fixed timer or have no way to advance
at all. The mechanism: **double-tap** (two single-finger taps at the
same point within ~400ms of each other — NOT two simultaneous
fingers) on the QR screen or the Processing screen manually advances
to the next screen.

Assumption stated plainly: "double-tap" here means two sequential
taps, timed, single-touch. This chip's commonly available Arduino
touch library is confirmed single-touch only (AXS15231B silicon
supports real multi-touch per its datasheet, but the community
wrapper libraries in use do not expose it) — so a simultaneous
two-finger gesture is not reliably implementable right now. If the
owner actually wants a two-finger gesture instead, say so and this
needs to be re-scoped, not assumed.

Implementation notes for CC:
- Reuse the same static-timestamp debounce pattern already used
  elsewhere in Satu firmware (e.g. `_lastWelMs` in ui_screens.h) —
  track the timestamp of the most recent tap on that screen; if a new
  tap lands within ~400ms of the previous one, treat it as the bypass
  gesture and transition state. A single isolated tap on these two
  screens does nothing (there's nothing else to tap there anyway).
- Comment this code block explicitly and unmissably as: "DEMO BYPASS
  — simulates an external trigger this offline demo has no way to
  receive for real. Not present in Satu 1.0's real payment flow. If
  this firmware is ever extended toward a real backend integration,
  this entire gesture must be removed, not left in as a hidden
  debug/cheat input."
- Add this as its own line item to RULES.md (R-7, see Section 7).

### 4.5 — qr.h
Generate the QR content as a plain placeholder text string that LOOKS
like a PromptPay-style payload (NOT a real EMVCo-compliant string, NOT
a real merchant ID) — this is a visual demo only, it must never be
presented as a scannable real payment QR. Comment this clearly in the
code. Size/scale the QR modules to nearly fill the 320px width while
leaving a small quiet-zone margin.

### 4.6 — Fully offline by design
Do not add WiFi connection code, do not add any HTTP/backend calls.
This is an explicit design decision for demo reliability (a sales
pitch should not depend on venue WiFi). Note this decision and the
reason in PROJECT_STATE.md.

---

## 5. COMPILE VIA GITHUB ACTIONS (no local Arduino IDE)

Owner does not want to manage a local Arduino library folder for this
project — and doesn't need to. Satu-Vending-Firmware already proved
this pattern: GitHub Actions installs arduino-cli + a pinned core +
pinned libraries inside an ephemeral cloud runner, compiles, and
uploads the .bin files as a downloadable artifact. Owner downloads and
flashes with `esptool` — no Arduino IDE opened, ever, for this repo.

Build `.github/workflows/compile-check.yml`:
- Trigger: push to any branch, PR to main
- Install arduino-cli
- Install ESP32 core — version UNCONFIRMED for this display combo
  (see Section 6), start with the same 2.0.17 Satu 1.0 uses since it's
  the same WROOM-1-N16R8 chip family; if compile fails on core-version
  grounds, that failure IS the confirmation this board needs a newer
  core — bump it, document why in LIBRARY_axs15231b.md, do not silently
  retry with guesses
- Install libraries: `GFX Library for Arduino`@[version pinned in
  Section 4.1], `QRCode` (latest)
- FQBN: same board family as Satu 1.0 (ESP32-S3-WROOM-1-N16R8) —
  `esp32:esp32:esp32s3:CDCOnBoot=cdc,FlashSize=16M,PartitionScheme=app3M_fat9M_16MB,PSRAM=opi,UploadSpeed=460800`
  as a starting point (this repo has no SD/FAT need, but reusing a
  known-working partition scheme is lower-risk than inventing an
  unverified one for a first pass — revisit if flash space is ever
  tight). CDCOnBoot=cdc because this board's Type-C is native ESP32-S3
  USB, not a separate UART bridge chip — flag this as worth confirming
  on first successful upload.
- Compile, then `actions/upload-artifact@v4` with the three build
  outputs: `*.ino.bootloader.bin`, `*.ino.partitions.bin`, `*.ino.bin`
- retention-days: 7

README.md flashing section — use current esptool syntax (not the
older `esptool.py`/`write_flash`/921600 form retired on Satu 1.0):
```
esptool --chip esp32s3 --port /dev/cu.usbserial-XXXX --baud 460800 \
  write-flash \
  0x0     Satu_EV_Demo.ino.bootloader.bin \
  0x8000  Satu_EV_Demo.ino.partitions.bin \
  0x10000 Satu_EV_Demo.ino.bin
```
Flag this exact command as needing owner confirmation on first real
flash — port name and offsets are standard-convention, not yet
verified against this specific board/partition scheme.

---

## 6. VERIFICATION

No automated test suite exists for this repo (unlike the backend's
14-test suite) — do not claim one passed. Verification for firmware
sessions is: GitHub Actions compile check must go green.

Important distinction to state explicitly in PROJECT_STATE.md: CI
green confirms the CODE compiles against the pinned library/core
versions. It does NOT confirm the GPIO pin numbers in display.h are
physically correct for the owner's actual unit — that can only be
confirmed by the owner flashing and observing the screen/touch. Do not
conflate the two when reporting status.

README.md must state, exactly and completely:
- ESP32 Arduino core version: UNCONFIRMED for this library combo —
  starting from 2.0.17 (Section 5), report whatever version the CI run
  actually ends up needing once it goes green
- Exact library versions pinned, and why (from Section 4.1)
- Owner needs `esptool` installed locally (owner already has this for
  Satu 1.0 flashing) and nothing else — no Arduino IDE, no local
  libraries

Expect at least 1 CI run to get compile green (fast iteration, no
owner hardware needed); then at least 1 physical flash cycle,
realistically 2-3, since pin mapping is unverified against the
physical unit and may need adjustment from what the owner observes on
screen/serial.

---

## 7. MANDATORY CLOSING (bootstrap session — adapted)

1. Create CC_CHAT_LOG.md fresh, write entry #1 (files created, library
   versions chosen, what's unverified)
2. Create RULES.md fresh, entries R-1 through R-6 minimum:
   - R-1: GFX Library for Arduino version pinned to [X] — do not
     upgrade without re-reading LIBRARY_axs15231b.md
   - R-2: Never render directly via Arduino_AXS15231B — always through
     Arduino_Canvas, always flush() after drawing
   - R-3: Pin mapping is community-sourced, not vendor-confirmed —
     verify against physical unit before every new board of this type
   - R-4: Fully offline by design — no WiFi/backend calls in this repo
   - R-5: QR content is a visual placeholder only, never a real
     payment payload
   - R-6: Compile via GitHub Actions only — never instruct owner to
     install these libraries in a local Arduino library folder
     (would collide with Satu-Vending-Firmware's locked versions)
   - R-7: QR and Processing screens advance via a double-tap demo
     bypass gesture, not a real trigger — this simulates external
     events the offline demo has no way to receive. Must be removed
     entirely, not just hidden, before any real backend integration.
3. Create PROJECT_STATE.md, KNOWLEDGE_MAP.md fresh
4. Version-header every .md file created
5. Push to main, confirm `.github/workflows/compile-check.yml` runs
   and goes green before considering this session done — if red, fix
   and push again, same discipline as Satu 1.0's CI rule
6. Archive this prompt file to `docs/prompts/` stamped
   ✅ COMPLETE — [date] — EV demo bootstrap build

---

## 8. PAYMENT MODE REMINDER

Not applicable in the usual sense — this repo never calls the Satu
backend or Omise at all. State explicitly in README.md and
CLAUDE.md: "This firmware simulates payment only. It must never be
modified to make a real payment call without a separate, explicit
owner-approved session — see Satu-Vending-Firmware's PAYMENT_MODE
pattern if/when this demo becomes a real integration."
