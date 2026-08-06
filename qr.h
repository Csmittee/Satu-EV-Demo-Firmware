#pragma once
// qr.h — QRCode library wrapper: generate + draw a full-screen QR from a
// text string. See LIBRARY_qrcode.md for the API this is built against.
//
// R-5: the QR content generated here is a visual placeholder only — it
// LOOKS like a payment-style payload but is NOT a real EMVCo-compliant
// PromptPay string and NOT a real merchant ID. This must never be
// presented as a scannable real payment QR.

#include <qrcode.h>
#include "config.h"
#include "display.h"

#define QR_VERSION 4      // fixed size (33x33 modules) — see LIBRARY_qrcode.md
#define QR_ECC ECC_LOW    // placeholder text, never actually scanned for payment

// Builds a FAKE, demo-only payload that looks like a structured payment
// string. Deliberately not EMVCo TLV-shaped — this must never be
// mistaken for a real PromptPay QR by anyone reading the source.
static void buildPlaceholderPayload(char *buf, size_t bufSize, uint16_t amountBaht) {
  snprintf(buf, bufSize, "DEMO-PAY|MERCHANT:SATU-EV-DEMO|AMT:%u.00|REF:NOTAREALPAYMENT",
            amountBaht);
}

// Draws the full QR screen: amount overlay + full-width QR, no other
// chrome, per Section 4.4 (screen 4 — QR).
static void drawQRScreen(uint16_t amountBaht) {
  char payload[80];
  buildPlaceholderPayload(payload, sizeof(payload), amountBaht);

  uint8_t qrData[qrcode_getBufferSize(QR_VERSION)];
  QRCode qrcode;
  qrcode_initText(&qrcode, qrData, QR_VERSION, QR_ECC, payload);

  gfx->fillScreen(COLOR_BG);

  gfx->setTextColor(COLOR_TEXT_DARK);
  gfx->setTextSize(2);
  gfx->setCursor(20, 24);
  gfx->print(g_lang.qrAmountPrefix);
  gfx->print(amountBaht);

  uint16_t moduleSize = SCREEN_W / qrcode.size;
  uint16_t qrPixelSize = moduleSize * qrcode.size;
  uint16_t offsetX = (SCREEN_W - qrPixelSize) / 2;
  uint16_t offsetY = 70;

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      uint16_t color = qrcode_getModule(&qrcode, x, y) ? COLOR_TEXT_DARK : COLOR_CARD;
      gfx->fillRect(offsetX + x * moduleSize, offsetY + y * moduleSize, moduleSize, moduleSize, color);
    }
  }

  gfx->flush();
}
