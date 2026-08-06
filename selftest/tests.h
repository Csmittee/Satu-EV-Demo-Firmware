#pragma once
// selftest/tests.h — top-level test menu + the 6 diagnostic tests.
//
// This sketch's whole purpose is producing evidence for the owner to
// read off the physical screen (and Serial monitor) and report back —
// it does not fix anything, and does not touch the real demo's files.
// See PROJECT_STATE.md: "diagnostic data, not a verified fix."

#include <stdint.h>
#include "config.h"
#include "display.h"
#include "qrcode_lib.h"

// ---------------------------------------------------------------------
// Small drawing helpers (same pattern as ../ui_screens.h)
// ---------------------------------------------------------------------
static void drawCenteredText(const char *text, int16_t cx, int16_t cy, uint8_t textSize, uint16_t color) {
  gfx->setTextSize(textSize);
  gfx->setTextColor(color);
  int16_t x1, y1;
  uint16_t w, h;
  gfx->getTextBounds(text, 0, 0, &x1, &y1, &w, &h);
  gfx->setCursor(cx - w / 2, cy - h / 2);
  gfx->print(text);
}

static bool pointInRect(int16_t px, int16_t py, int16_t rx, int16_t ry, int16_t rw, int16_t rh) {
  return px >= rx && px < rx + rw && py >= ry && py < ry + rh;
}

// Forward declaration — defined at the bottom, needed by
// handleTestMenuTouch() above it.
static void enterTest(int8_t id);

// ---------------------------------------------------------------------
// Touch edge detection (same pattern as ../ui_screens.h)
// ---------------------------------------------------------------------
static bool _wasTouched = false;

static TouchPoint pollTouchEdge() {
  TouchPoint tp = readTouch();
  TouchPoint edge = {false, 0, 0};
  if (tp.touched && !_wasTouched) {
    edge = tp;
  }
  _wasTouched = tp.touched;
  return edge;
}

// ---------------------------------------------------------------------
// "< Back" zone — fixed top-left corner, identical on every test screen,
// so there's one muscle-memory spot to return to the menu from.
// ---------------------------------------------------------------------
#define BACK_ZONE_X 0
#define BACK_ZONE_Y 0
#define BACK_ZONE_W 70
#define BACK_ZONE_H 40

static void drawBackZone() {
  gfx->fillRect(BACK_ZONE_X, BACK_ZONE_Y, BACK_ZONE_W, BACK_ZONE_H, COLOR_PRIMARY);
  drawCenteredText("<Back", BACK_ZONE_X + BACK_ZONE_W / 2, BACK_ZONE_Y + BACK_ZONE_H / 2, 1, COLOR_TEXT_LIGHT);
}

static bool isBackZoneTap(const TouchPoint &tp) {
  return pointInRect(tp.x, tp.y, BACK_ZONE_X, BACK_ZONE_Y, BACK_ZONE_W, BACK_ZONE_H);
}

// ---------------------------------------------------------------------
// Top-level test menu
// ---------------------------------------------------------------------
#define TEST_DISPLAY_SANITY    0
#define TEST_TOUCH_CALIBRATION 1
#define TEST_BUTTON_HITTEST    2
#define TEST_FLUSH_TIMING      3
#define TEST_DOUBLE_TAP        4
#define TEST_QR_RENDER         5
#define TEST_COUNT             6

static const char *TEST_NAMES[TEST_COUNT] = {
  "1. Display Sanity",
  "2. Touch Calibration",
  "3. Button Hit-Test",
  "4. Flush Timing",
  "5. Double-Tap",
  "6. QR Render",
};

#define MENU_TOP 55
#define MENU_MARGIN 16
#define MENU_ROW_H 55
#define MENU_ROW_GAP 8

static void menuRowRect(uint8_t i, int16_t *x, int16_t *y, int16_t *w, int16_t *h) {
  *x = MENU_MARGIN;
  *y = MENU_TOP + i * (MENU_ROW_H + MENU_ROW_GAP);
  *w = SCREEN_W - 2 * MENU_MARGIN;
  *h = MENU_ROW_H;
}

static void drawTestMenu() {
  gfx->fillScreen(COLOR_BG);
  drawCenteredText("Self-Test Menu", SCREEN_W / 2, 20, 2, COLOR_TEXT_DARK);

  for (uint8_t i = 0; i < TEST_COUNT; i++) {
    int16_t x, y, w, h;
    menuRowRect(i, &x, &y, &w, &h);
    gfx->fillRoundRect(x, y, w, h, 8, COLOR_CARD);
    gfx->drawRoundRect(x, y, w, h, 8, COLOR_CARD_BORDER);
    drawCenteredText(TEST_NAMES[i], x + w / 2, y + h / 2, 2, COLOR_TEXT_DARK);
  }

  gfx->flush();
}

static void handleTestMenuTouch(const TouchPoint &tp) {
  for (uint8_t i = 0; i < TEST_COUNT; i++) {
    int16_t x, y, w, h;
    menuRowRect(i, &x, &y, &w, &h);
    if (pointInRect(tp.x, tp.y, x, y, w, h)) {
      enterTest(i);
      return;
    }
  }
}

// =========================================================================
// Test 1 — Display sanity (cycle full-screen fills, confirm color order
// and full-panel coverage)
// =========================================================================
static const uint16_t CYCLE_COLORS[5] = {
  RGB565(255, 0, 0), RGB565(0, 255, 0), RGB565(0, 0, 255), RGB565(255, 255, 255), RGB565(0, 0, 0)
};
static const char *CYCLE_NAMES[5] = {"RED", "GREEN", "BLUE", "WHITE", "BLACK"};
#define CYCLE_STEP_MS 1000

static uint32_t _cycleStartMs = 0;
static uint8_t _lastCycleIndex = 255;  // sentinel, forces first draw

static void tickDisplaySanity() {
  uint32_t elapsed = millis() - _cycleStartMs;
  uint8_t idx = (elapsed / CYCLE_STEP_MS) % 5;
  if (idx == _lastCycleIndex) return;
  _lastCycleIndex = idx;

  gfx->fillScreen(CYCLE_COLORS[idx]);
  gfx->fillRect(SCREEN_W - 140, 0, 140, 30, COLOR_CARD);
  drawCenteredText(CYCLE_NAMES[idx], SCREEN_W - 70, 15, 2, COLOR_TEXT_DARK);
  drawBackZone();
  gfx->flush();
}

// =========================================================================
// Test 2 — Touch coordinate calibration (built to catch the Confirm bug)
// Tap the 5 targets in order (TL, TR, BL, BR, C); each tap records the
// RAW x,y readTouch() returned, shown on screen and via Serial, against
// that target's expected position. No auto-advance to another test.
// =========================================================================
struct CalibTarget {
  const char *label;
  int16_t x, y;
};

static const CalibTarget CALIB_TARGETS[5] = {
  {"TL", 20, 20},
  {"TR", SCREEN_W - 20, 20},
  {"BL", 20, SCREEN_H - 20},
  {"BR", SCREEN_W - 20, SCREEN_H - 20},
  {"C", SCREEN_W / 2, SCREEN_H / 2},
};

struct CalibResult {
  bool captured;
  int16_t rawX, rawY;
};

static CalibResult _calibResults[5];
static uint8_t _calibNextIndex = 0;

static void drawTouchCalibrationScreen() {
  gfx->fillScreen(COLOR_BG);
  drawCenteredText("Tap targets in order: TL,TR,BL,BR,C", SCREEN_W / 2, 55, 1, COLOR_TEXT_DARK);

  for (uint8_t i = 0; i < 5; i++) {
    uint16_t color = _calibResults[i].captured ? COLOR_SUCCESS : COLOR_PRIMARY;
    gfx->fillCircle(CALIB_TARGETS[i].x, CALIB_TARGETS[i].y, 18, color);
    drawCenteredText(CALIB_TARGETS[i].label, CALIB_TARGETS[i].x, CALIB_TARGETS[i].y, 1, COLOR_TEXT_LIGHT);
  }

  for (uint8_t i = 0; i < 5; i++) {
    if (!_calibResults[i].captured) continue;
    char buf[40];
    snprintf(buf, sizeof(buf), "%s expect(%d,%d) raw(%d,%d)",
             CALIB_TARGETS[i].label, CALIB_TARGETS[i].x, CALIB_TARGETS[i].y,
             _calibResults[i].rawX, _calibResults[i].rawY);
    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_TEXT_DARK);
    gfx->setCursor(60, 270 + i * 16);
    gfx->print(buf);
  }

  drawBackZone();
  gfx->flush();
}

static void handleTouchCalibrationTouch(const TouchPoint &tp) {
  if (_calibNextIndex >= 5) return;

  CalibResult &r = _calibResults[_calibNextIndex];
  r.captured = true;
  r.rawX = tp.x;
  r.rawY = tp.y;

  Serial.printf("Calib[%s] expected=(%d,%d) raw=(%d,%d)\n",
                CALIB_TARGETS[_calibNextIndex].label,
                CALIB_TARGETS[_calibNextIndex].x, CALIB_TARGETS[_calibNextIndex].y,
                tp.x, tp.y);

  _calibNextIndex++;
  drawTouchCalibrationScreen();
}

// =========================================================================
// Test 3 — Isolated button hit-test at the exact broken geometry
// (CONFIRM_BTN_X/Y/W/H in config.h, copied from ui_screens.h's
// confirmConfirmRect()). Shows "HIT" on tap-inside, nothing otherwise —
// deliberately not softened with a "MISS" indicator, per the prompt: the
// point is to see whether this exact geometry responds at all.
// =========================================================================
static bool _btnHit = false;

static void drawButtonHitTest() {
  gfx->fillScreen(COLOR_BG);
  gfx->fillRoundRect(CONFIRM_BTN_X, CONFIRM_BTN_Y, CONFIRM_BTN_W, CONFIRM_BTN_H, 8, COLOR_PRIMARY);
  drawCenteredText("Confirm", CONFIRM_BTN_X + CONFIRM_BTN_W / 2, CONFIRM_BTN_Y + CONFIRM_BTN_H / 2, 2, COLOR_TEXT_LIGHT);

  if (_btnHit) {
    drawCenteredText("HIT", SCREEN_W / 2, 200, 4, COLOR_SUCCESS);
  }

  drawBackZone();
  gfx->flush();
}

static void handleButtonHitTestTouch(const TouchPoint &tp) {
  if (!pointInRect(tp.x, tp.y, CONFIRM_BTN_X, CONFIRM_BTN_Y, CONFIRM_BTN_W, CONFIRM_BTN_H)) {
    return;  // no response at all — that silence IS the data point
  }
  _btnHit = true;
  drawButtonHitTest();
}

// =========================================================================
// Test 4 — flush() timing. Runs once on entry (blocking, by design — this
// measures exactly how long those blocking calls take), then just shows
// the results until Back is tapped.
// =========================================================================
static uint32_t _flushMinMs = 0, _flushMaxMs = 0, _flushAvgMs = 0;

static void runFlushTimingTest() {
  uint32_t total = 0;
  uint32_t minMs = UINT32_MAX, maxMs = 0;

  for (uint8_t i = 0; i < 20; i++) {
    uint16_t color = (i % 2 == 0) ? COLOR_CARD : COLOR_BG;
    uint32_t start = millis();
    gfx->fillScreen(color);
    gfx->flush();
    uint32_t elapsed = millis() - start;

    total += elapsed;
    if (elapsed < minMs) minMs = elapsed;
    if (elapsed > maxMs) maxMs = elapsed;
    Serial.printf("flush[%u] = %lu ms\n", i, (unsigned long)elapsed);
  }

  _flushMinMs = minMs;
  _flushMaxMs = maxMs;
  _flushAvgMs = total / 20;
  Serial.printf("flush min=%lu max=%lu avg=%lu ms\n",
                (unsigned long)_flushMinMs, (unsigned long)_flushMaxMs, (unsigned long)_flushAvgMs);
}

static void drawFlushTimingResults() {
  gfx->fillScreen(COLOR_BG);
  drawCenteredText("flush() Timing (20 runs)", SCREEN_W / 2, 70, 2, COLOR_TEXT_DARK);

  char buf[32];
  snprintf(buf, sizeof(buf), "min: %lu ms", (unsigned long)_flushMinMs);
  drawCenteredText(buf, SCREEN_W / 2, 150, 2, COLOR_TEXT_DARK);
  snprintf(buf, sizeof(buf), "max: %lu ms", (unsigned long)_flushMaxMs);
  drawCenteredText(buf, SCREEN_W / 2, 190, 2, COLOR_TEXT_DARK);
  snprintf(buf, sizeof(buf), "avg: %lu ms", (unsigned long)_flushAvgMs);
  drawCenteredText(buf, SCREEN_W / 2, 230, 2, COLOR_PRIMARY);

  drawBackZone();
  gfx->flush();
}

// =========================================================================
// Test 5 — double-tap bypass gesture, isolated. Same debounce pattern as
// checkDoubleTapBypass() in ../ui_screens.h — copied, not shared, since
// this needs to work standalone. If this ever changes, port the fix back.
// =========================================================================
static uint32_t _dtLastTapMs = 0;
static const char *_dtLastResult = "-";

static void drawDoubleTapTest() {
  gfx->fillScreen(COLOR_BG);
  drawCenteredText("Tap anywhere below", SCREEN_W / 2, 140, 2, COLOR_TEXT_DARK);
  drawCenteredText(_dtLastResult, SCREEN_W / 2, 220, 4, COLOR_PRIMARY);
  drawBackZone();
  gfx->flush();
}

static void handleDoubleTapTouch(const TouchPoint &tp) {
  uint32_t now = millis();
  bool isDoubleTap = (_dtLastTapMs != 0) && (now - _dtLastTapMs <= DOUBLE_TAP_WINDOW_MS);
  _dtLastTapMs = now;
  _dtLastResult = isDoubleTap ? "double" : "single";
  drawDoubleTapTest();
}

// =========================================================================
// Test 6 — QR render, isolated. Reuses the placeholder-payload + module-
// drawing logic from ../qr.h (copied, not shared — see qrcode_lib.h in
// this folder for why). Full-screen, nothing else on screen except the
// small Back zone — a functional exception to "nothing else," since
// without it this test would be a dead end with no way back.
// =========================================================================
#define QR_TEST_VERSION 4
#define QR_TEST_ECC ECC_LOW

static void buildPlaceholderPayload(char *buf, size_t bufSize, uint16_t amountBaht) {
  snprintf(buf, bufSize, "DEMO-PAY|MERCHANT:SATU-EV-DEMO|AMT:%u.00|REF:NOTAREALPAYMENT", amountBaht);
}

static void drawQrRenderTest() {
  char payload[80];
  buildPlaceholderPayload(payload, sizeof(payload), 180);

  uint8_t qrData[qrcode_getBufferSize(QR_TEST_VERSION)];
  QRCode qrcode;
  qrcode_initText(&qrcode, qrData, QR_TEST_VERSION, QR_TEST_ECC, payload);

  gfx->fillScreen(COLOR_BG);

  uint16_t moduleSize = SCREEN_W / qrcode.size;
  uint16_t qrPixelSize = moduleSize * qrcode.size;
  uint16_t offsetX = (SCREEN_W - qrPixelSize) / 2;
  uint16_t offsetY = (SCREEN_H - qrPixelSize) / 2;

  for (uint8_t y = 0; y < qrcode.size; y++) {
    for (uint8_t x = 0; x < qrcode.size; x++) {
      uint16_t color = qrcode_getModule(&qrcode, x, y) ? COLOR_TEXT_DARK : COLOR_CARD;
      gfx->fillRect(offsetX + x * moduleSize, offsetY + y * moduleSize, moduleSize, moduleSize, color);
    }
  }

  drawBackZone();
  gfx->flush();
}

// =========================================================================
// Top-level dispatch — called from Satu_EV_SelfTest.ino's loop()
// =========================================================================
static int8_t _currentTest = -1;  // -1 = menu

static void enterTest(int8_t id) {
  _currentTest = id;

  switch (id) {
    case TEST_DISPLAY_SANITY:
      _cycleStartMs = millis();
      _lastCycleIndex = 255;
      break;
    case TEST_TOUCH_CALIBRATION:
      _calibNextIndex = 0;
      for (uint8_t i = 0; i < 5; i++) _calibResults[i].captured = false;
      drawTouchCalibrationScreen();
      break;
    case TEST_BUTTON_HITTEST:
      _btnHit = false;
      drawButtonHitTest();
      break;
    case TEST_FLUSH_TIMING:
      runFlushTimingTest();
      drawFlushTimingResults();
      break;
    case TEST_DOUBLE_TAP:
      _dtLastTapMs = 0;
      _dtLastResult = "-";
      drawDoubleTapTest();
      break;
    case TEST_QR_RENDER:
      drawQrRenderTest();
      break;
  }
}

static void returnToMenu() {
  _currentTest = -1;
  drawTestMenu();
}

// Call every loop(). Only Test 1 (Display Sanity) needs periodic ticking
// — everything else is purely event-driven off touch.
static void tickSelfTest() {
  if (_currentTest == TEST_DISPLAY_SANITY) {
    tickDisplaySanity();
  }
}

// Call every loop() with the latest touch-down edge (see pollTouchEdge()).
static void handleSelfTestTouch(const TouchPoint &tp) {
  if (!tp.touched) return;

  if (_currentTest == -1) {
    handleTestMenuTouch(tp);
    return;
  }

  if (isBackZoneTap(tp)) {
    returnToMenu();
    return;
  }

  switch (_currentTest) {
    case TEST_TOUCH_CALIBRATION: handleTouchCalibrationTouch(tp); break;
    case TEST_BUTTON_HITTEST:    handleButtonHitTestTouch(tp); break;
    case TEST_DOUBLE_TAP:        handleDoubleTapTouch(tp); break;
    default: break;  // Display Sanity, Flush Timing, QR Render: Back only
  }
}
