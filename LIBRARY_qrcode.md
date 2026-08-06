# LIBRARY_qrcode.md
> Version 1.1 — 2026-08-06
> Library: QRCode (ricmoo/QRCode) — VENDORED as `qrcode_lib.c`/`qrcode_lib.h`,
> not installed via Arduino Library Manager. See "Why vendored" below.
> Covers: QR generation API actually exported, as vendored

---

## Why this is vendored, not Library-Manager-installed (KT — R-6 SKILL 6)

First CI run (`compile-check.yml`) failed with `qrcode_getBufferSize`,
`QRCode`, `qrcode_initText`, `ECC_LOW`, `qrcode_getModule` all "not
declared in this scope" inside `qr.h`, despite `arduino-cli lib install
"QRCode"` reporting success and `#include <qrcode.h>` being present. The
compiler's suggested alternatives (`esp_qrcode_get_module`,
`qrcode_display`) were the giveaway.

**IS:** `#include <qrcode.h>` resolves to a *different* header than
ricmoo's — one exposing `esp_qrcode_*`-prefixed functions instead.
**IS NOT:** the library failed to install; `arduino-cli` logged
`Installed QRCode@0.0.1` successfully, and the "Used library" table at
the end of the failed build listed `GFX Library for Arduino`, `SPI`,
`Wire` — but never `QRCode`.

**Hypothesis, confirmed:** the ESP32 Arduino core (2.0.17) bundles
Espressif's own `qrcode` component internally (used for
`RMakerQR.h`/provisioning), which also ships a header literally named
`qrcode.h`. That core-bundled header wins the name resolution before
arduino-cli's library-dependency scan ever gets to consider the
Library-Manager-installed `QRCode` library — which is exactly why
`QRCode` never appears in the "Used library" table: it was never
selected as a dependency at all. This is a real, documented collision
(see e.g. espressif/esp32-arduino-lib-builder#138), not a mistake in
this repo's install step.

**Fix:** vendor ricmoo/QRCode's `src/qrcode.c`/`src/qrcode.h` (MIT
licensed, unmodified except the renamed include) directly into the repo
as `qrcode_lib.c`/`qrcode_lib.h`, and `#include "qrcode_lib.h"` with
quotes. A quote-include always checks the including file's own directory
first, so it can never lose to the core's angle-bracket-resolved
`qrcode.h` again — regardless of arduino-cli's dependency-scan behavior.
The CI workflow no longer runs `arduino-cli lib install "QRCode"`, and
copies `qrcode_lib.c`/`.h` into the staged sketch folder alongside the
other sketch files.

**Do not revert this** by switching back to `<qrcode.h>` + Library
Manager — that's the exact configuration that broke CI.

---

## What was read

- Repo README: https://github.com/ricmoo/QRCode
- `src/qrcode.h` (public API header) — read in full for exact signatures
- `library.properties` — installed/pinned version number

## Version vendored

`library.properties` reports **version 0.0.1** — this library has never
moved past 0.0.1 in its versioning scheme despite being actively used.
Not a placeholder or a typo. `qrcode_lib.c`/`qrcode_lib.h` are vendored
from this exact version (see "Why this is vendored" above) — bump by
re-fetching `src/qrcode.c`/`src/qrcode.h` from upstream and re-applying
the same rename if a future session needs a newer version.

## Exact API surface (as exported by `qrcode_lib.h`)

```c
typedef struct QRCode {
    uint8_t version;   // 1-40
    uint8_t size;      // width/height in modules, set by init
    uint8_t ecc;        // error correction level actually used
    uint8_t mode;
    uint8_t mask;
    uint8_t *modules;   // pointer into caller-supplied buffer
} QRCode;

uint16_t qrcode_getBufferSize(uint8_t version);

int8_t qrcode_initText(QRCode *qrcode, uint8_t *modules,
                        uint8_t version, uint8_t ecc, const char *data);

int8_t qrcode_initBytes(QRCode *qrcode, uint8_t *modules,
                         uint8_t version, uint8_t ecc,
                         uint8_t *data, uint16_t length);

bool qrcode_getModule(QRCode *qrcode, uint8_t x, uint8_t y);
```

ECC enum (`ECCLEVEL` in `qrcode.h`):

```c
ECC_LOW      = 0   // ~7% recovery
ECC_MEDIUM   = 1   // ~15%
ECC_QUARTILE = 2   // ~25%
ECC_HIGH     = 3   // ~30%
```

## Usage pattern verified (matches README's simplest example)

```cpp
uint8_t qrVersion = 4;                 // fixed size, avoids runtime growth
uint8_t qrcodeData[qrcode_getBufferSize(qrVersion)];
QRCode qrcode;
qrcode_initText(&qrcode, qrcodeData, qrVersion, ECC_LOW, "text to encode");

for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
        bool on = qrcode_getModule(&qrcode, x, y);
        // draw one module as a filled square of `on ? fg : bg`
    }
}
```

## Choices made for `qr.h`

- **Version 4** (33×33 modules) — fixed, not computed at runtime. The
  placeholder payload string is short and fits comfortably at ECC_LOW;
  fixing the version avoids `qrcode_initText` silently choosing a
  different size if the string length changes later (e.g. when the
  owner swaps in a longer real-looking payload before the customer
  meeting).
- **ECC_LOW** — this is a visual placeholder never meant to be scanned
  for a real transaction (see R-5); higher ECC just shrinks module size
  for no benefit here.
- Buffer is a fixed-size `uint8_t` array sized by `qrcode_getBufferSize()`
  at compile time — no dynamic allocation, matches the rest of this
  firmware's no-heap-churn style.
- Module scaling: `320 / qrcode.size` (integer division) gives the pixel
  size per module for a near-full-width code with a small quiet-zone
  margin, per Section 4.5.

## Known issues

- Library README/issues note QR versions ≥10 can fail to scan reliably
  on some readers (issue #32) — not a concern here since we're fixed at
  version 4 for a short placeholder string.
