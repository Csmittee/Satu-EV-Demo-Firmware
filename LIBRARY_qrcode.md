# LIBRARY_qrcode.md
> Version 1.0 — 2026-08-06
> Library: QRCode (ricmoo/QRCode, "QRCode" in Arduino Library Manager)
> Covers: QR generation API actually exported, as installed

---

## What was read

- Repo README: https://github.com/ricmoo/QRCode
- `src/qrcode.h` (public API header) — read in full for exact signatures
- `library.properties` — installed/pinned version number

## Version pinned

`library.properties` reports **version 0.0.1** — this library has never
moved past 0.0.1 in its versioning scheme despite being actively used;
that is the correct version string to pin in the CI workflow (`QRCode`
latest, which resolves to 0.0.1). Not a placeholder or a typo.

## Exact API surface (as exported by `qrcode.h`)

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
