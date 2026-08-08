# LIBRARY_axs15231b.md
> Version 1.1 — 2026-08-08
> Library: GFX Library for Arduino (moononournation/Arduino_GFX)
> Covers: display bus, AXS15231B driver, Arduino_Canvas framebuffer, touch read

---

## What was read

- Repo README: https://github.com/moononournation/Arduino_GFX
- `examples/PDQgraphicstest/Arduino_GFX_dev_device.h` — bundled dev-device
  presets, specifically the `JC3248W535` entry
- Release list: https://github.com/moononournation/Arduino_GFX/releases
- GitHub issue #803 ("Bug: AXS15231B QSPI initialization regression in
  1.6.1+") and related closed issues (#766, #713, #593, #513) — all touch
  or display regressions on this exact controller/board family
- Espressif `esp_lcd_axs15231b` component source
  (`NorthernMan54/JC3248W535EN/src/esp_lcd_axs15231b.c`) — same silicon,
  used to cross-check the I2C touch protocol at the byte level
- Community driver repo `me-processware/JC3248W535-Driver` — cross-check
  for touch I2C address/pins
- `NorthernMan54/JC3248W535EN/src/esp_lcd_axs15231b.c` (Espressif's
  official `esp_lcd_axs15231b` component, vendored into a community
  repo) — read again for the exact `touch_axs15231b_reset()` timing,
  2026-08-08, to fix the garbage `4095,4095` touch reads

## Version pinned: v1.6.0 — NOT the latest (v1.6.7)

Latest release at time of writing is **v1.6.7** (2026-07-18). We are
pinning to **v1.6.0** instead.

**Reason:** GitHub issue #803, filed 2026-06-18 against this repo, reports
that AXS15231B QSPI initialization on the Guition JC3248W535 (this exact
board) is broken in v1.6.1 and "all subsequent releases," and that v1.6.0
is the last version confirmed to work for this board. The issue was
closed the same day it was filed with no visible maintainer comment or
linked fix commit — that is NOT evidence the regression was fixed, it is
just evidence the issue tracker no longer shows it open. Per R-1 (task
4.1), the instruction is to pin to whichever version is *confirmed*
working, not to assume a later release fixed a reported break. v1.6.0 is
the confirmed-working version for this board; v1.6.1+ is not.

If a future session confirms (via changelog entry naming AXS15231B, or a
clean compile+flash test on the physical unit) that a specific later
version fixes this, bump the pin and update this file with the evidence.
Do not bump on assumption.

## Classes used — confirmed via library's own bundled preset

The library ships its own dev-device preset for this exact board
(`JC3248W535` entry in `Arduino_GFX_dev_device.h`), so per Section 4.2's
instruction to prefer the maintainer's own preset over a hand-wired
community pin set where one exists, the bus/backlight pins below come
from that preset — they are maintainer-validated, not community-guessed:

```cpp
Arduino_DataBus *bus = new Arduino_ESP32QSPI(
    45 /* CS */, 47 /* SCK */, 21 /* D0 */, 48 /* D1 */, 40 /* D2 */, 39 /* D3 */);

Arduino_GFX *panel = new Arduino_AXS15231B(
    bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */, false /* IPS */,
    320 /* width */, 480 /* height */,
    0, 0, 0, 0);

Arduino_Canvas *gfx = new Arduino_Canvas(320, 480, panel, 0, 0, 0);

#define GFX_BL 1  // backlight pin, from same preset
```

These pins match exactly what Section 4.2 of the build prompt listed as
"community-reported" (CS=45, SCK=47, D0=21, D1=48, D2=40, D3=39, GFX_BL=1)
— the maintainer preset and the community set agree here, which raises
confidence, but this is still not a substitute for the owner confirming
against the physical unit's silkscreen (R-3).

**R-2 applies:** never call draw operations on `panel` directly — always
through `gfx` (the `Arduino_Canvas` wrapper), and every screen redraw
must end with `gfx->flush()`. Direct AXS15231B rendering without the
canvas framebuffer is reported unstable by multiple community sources
(tearing, partial/garbled frames) independent of the 1.6.1 regression
above.

## Touch — I2C, address 0x3B, NOT covered by the display preset

The bundled `JC3248W535` dev-device preset is display-only; it does not
configure touch. Touch pins remain community-sourced (per Section 4.2):

```
TOUCH_SDA = 4
TOUCH_SCL = 8
TOUCH_RST = 12
TOUCH_INT = 11
```

**Flag:** one other community repo (`me-processware/JC3248W535-Driver`)
lists `TOUCH_INT = 3` instead of 11 for what claims to be the same board.
This is exactly the kind of unverified-pin conflict Section 4.2 warns
about — owner must confirm both INT candidates against the physical
unit; if GPIO 11 produces no interrupt/read activity, try GPIO 3 before
assuming the touch chip itself is dead.

### Touch read protocol (no dedicated Arduino touch library — raw I2C)

There is no widely-adopted Arduino wrapper library for AXS15231B touch.
Protocol below is cross-checked between Espressif's `esp_lcd_axs15231b`
component (same silicon) and community JC3248W535 driver repos; both
agree on this byte layout:

1. `Wire.beginTransmission(0x3B)`, write 8 command bytes:
   `{0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00, 0x00, 0x08}`, `endTransmission()`
2. `Wire.requestFrom(0x3B, 8)` — read 8 response bytes
3. Parse (single-touch mode only — see below):
   - `touchCount = data[1]`
   - `x = ((data[2] & 0x0F) << 8) | data[3]`
   - `y = ((data[4] & 0x0F) << 8) | data[5]`
   - `touchCount == 0` → no touch active this poll

**Single-touch only, confirmed:** the AXS15231B silicon supports real
multi-touch per its datasheet, but this protocol path (and every Arduino
wrapper found) exposes single-point data only. This is why the double-tap
gesture (R-7) is implemented as two sequential single-point taps, not a
simultaneous two-finger gesture — see `ui_screens.h`.

### Touch reset timing — fixed 2026-08-08 (was the root cause of garbage `4095,4095` reads)

**Evidence (owner, physical board, Serial capture via selftest Test 2):**
all four corner taps — four different physical locations — returned the
identical raw value `(4095, 4095)` (`0x0FFF`, the max a 12-bit field
holds). A center tap and a retap of the same corner both returned
different, plausible small values. `Wire.requestFrom()` still returned 8
bytes each time — the I2C transaction "succeeded," the content was
garbage (consistent with all of `data[2..5]` being `0xFF`).

**Root cause:** `displayInit()`'s touch reset sequence
(`HIGH → delay(20) → LOW → delay(20) → HIGH → delay(50)`) was too short.
Espressif's own `esp_lcd_axs15231b` reference driver (same silicon,
via `esp_lcd_touch_new_i2c_axs15231b()` → `touch_axs15231b_reset()`)
uses `delay(200)` on **both** sides of the reset toggle — 4-10x longer
than what this repo had. Querying the touch engine before it finished
booting after reset returned garbage that got read as valid touch data.
The reference driver's `touch_axs15231b_read_data()` does **no**
garbage-value filtering either (it only checks the reported point
count) — reinforcing that correct reset timing, not defensive read
validation, is the actual fix; a working implementation of this exact
chip doesn't need one.

**Fix:** reset delays changed to match the reference — 200ms hold-low +
200ms settle after release, in both `display.h` and `selftest/display.h`.
See RULES.md R-9.

**Not independently re-verified from this session** (no physical
hardware access here) — owner must re-flash and re-run Test 2/3 to
confirm no more `4095,4095` readings and that Test 3 now responds
reliably on first tap. If the symptom persists after this fix, the next
hypothesis to check is the command-byte sequence itself
(`0xB5 0xAB 0xA5 0x5A 0x00 0x00 0x00 0x08`) being a community
reverse-engineered guess that may return valid-looking-but-wrong data
independent of reset timing (see Section 3.2 candidate 2 in
`docs/prompts/archive/CC_PROMPT_fix_touch_garbage_read_v1.md`).

## Known issues summary (for future sessions)

- Touch reset sequence needed 200ms/200ms, not 20ms/20ms/50ms — fixed
  2026-08-08, see "Touch reset timing" above. If garbage `4095,4095`
  reads ever reappear, this is the first place to re-check, not the
  second.
- v1.6.1+: AXS15231B QSPI init regression on this board (#803) — reason
  we're pinned to v1.6.0, see above
- Multiple older issues (#766, #713, #593, #513) on this same
  controller/board family — pattern suggests this display combo is a
  recurring pain point in the library, not a one-off. Re-check the issue
  tracker before ever bumping this pin.
- Direct (non-Canvas) rendering reported unstable across several
  independent community reports — this is why R-2 exists.
