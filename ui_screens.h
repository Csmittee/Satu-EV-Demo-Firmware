#pragma once
// ui_screens.h — the 6 demo screens (draw + touch-handler pair each) and
// the top-level dispatch (tickScreen / handleTouch) called from loop().
//
// No product images, no photos, no Thai strings this pass — shapes and
// text only. All strings come from g_lang (config.h) so a Thai table can
// slot in later without touching this file.

#include "config.h"
#include "display.h"
#include "state_machine.h"
#include "qr.h"

// -----------------------------------------------------------------------
// Small drawing helpers
// -----------------------------------------------------------------------
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

// -----------------------------------------------------------------------
// Touch edge detection — fires only on the frame a touch begins, not
// every frame it's held.
// -----------------------------------------------------------------------
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

// -----------------------------------------------------------------------
// DEMO BYPASS GESTURE — simulates an external trigger this offline demo
// has no way to receive for real (a customer's QR scan, or a payment
// webhook confirming dispensing). Two sequential single-finger taps at
// the same screen within DOUBLE_TAP_WINDOW_MS count as the gesture. NOT
// present in Satu 1.0's real payment flow. If this firmware is ever
// extended toward a real backend integration, this entire gesture must
// be removed, not left in as a hidden debug/cheat input. See RULES.md R-7.
// -----------------------------------------------------------------------
struct DoubleTapTracker {
  uint32_t lastTapMs;
};

static DoubleTapTracker _qrTapTracker = {0};
static DoubleTapTracker _procTapTracker = {0};

static bool checkDoubleTapBypass(DoubleTapTracker &tracker, uint32_t nowMs) {
  bool isDoubleTap = (tracker.lastTapMs != 0) && (nowMs - tracker.lastTapMs <= DOUBLE_TAP_WINDOW_MS);
  tracker.lastTapMs = nowMs;
  return isDoubleTap;
}

// =========================================================================
// Screen 1 — Welcome
// =========================================================================
static void drawWelcomeScreen() {
  gfx->fillScreen(COLOR_BG);
  gfx->fillRect(0, 0, SCREEN_W, 160, COLOR_PRIMARY);
  drawCenteredText(g_lang.welcomeTitle, SCREEN_W / 2, 80, 2, COLOR_TEXT_LIGHT);
  drawCenteredText(g_lang.welcomeSubtitle, SCREEN_W / 2, 120, 1, COLOR_TEXT_LIGHT);
  drawCenteredText(g_lang.welcomeTap, SCREEN_W / 2, 420, 2, COLOR_PRIMARY_DARK);
  loggedFlush();
}

static void handleWelcomeTouch(const TouchPoint &tp) {
  if (!tp.touched) return;
  transitionTo(STATE_MENU);
}

// =========================================================================
// Screen 2 — Menu (2x3 touch grid)
// =========================================================================
#define MENU_COLS 2
#define MENU_ROWS 3
#define MENU_MARGIN 16
#define MENU_GAP 12
#define MENU_TOP 90
#define MENU_BOTTOM_MARGIN 20
#define MENU_TILE_W ((SCREEN_W - 2 * MENU_MARGIN - (MENU_COLS - 1) * MENU_GAP) / MENU_COLS)
#define MENU_TILE_H ((SCREEN_H - MENU_TOP - MENU_BOTTOM_MARGIN - (MENU_ROWS - 1) * MENU_GAP) / MENU_ROWS)

static void menuTileRect(uint8_t index, int16_t *x, int16_t *y, int16_t *w, int16_t *h) {
  uint8_t col = index % MENU_COLS;
  uint8_t row = index / MENU_COLS;
  *x = MENU_MARGIN + col * (MENU_TILE_W + MENU_GAP);
  *y = MENU_TOP + row * (MENU_TILE_H + MENU_GAP);
  *w = MENU_TILE_W;
  *h = MENU_TILE_H;
}

static void drawMenuScreen() {
  gfx->fillScreen(COLOR_BG);
  drawCenteredText(g_lang.menuTitle, SCREEN_W / 2, 40, 2, COLOR_TEXT_DARK);

  for (uint8_t i = 0; i < MENU_ITEM_COUNT; i++) {
    int16_t x, y, w, h;
    menuTileRect(i, &x, &y, &w, &h);

    gfx->fillRoundRect(x, y, w, h, 8, COLOR_CARD);
    gfx->drawRoundRect(x, y, w, h, 8, COLOR_CARD_BORDER);

    gfx->setTextSize(1);
    gfx->setTextColor(COLOR_TEXT_DARK);
    gfx->setCursor(x + 8, y + 10);
    gfx->print(g_menuItems[i].name);

    char priceBuf[16];
    snprintf(priceBuf, sizeof(priceBuf), "THB %u", g_menuItems[i].priceBaht);
    drawCenteredText(priceBuf, x + w / 2, y + h - 24, 2, COLOR_PRIMARY);
  }

  loggedFlush();
}

static void handleMenuTouch(const TouchPoint &tp) {
  if (!tp.touched) return;
  for (uint8_t i = 0; i < MENU_ITEM_COUNT; i++) {
    int16_t x, y, w, h;
    menuTileRect(i, &x, &y, &w, &h);
    if (pointInRect(tp.x, tp.y, x, y, w, h)) {
      g_ctx.selectedItem = i;
      transitionTo(STATE_CONFIRM);
      return;
    }
  }
}

// =========================================================================
// Screen 3 — Confirm
// =========================================================================
#define CONFIRM_BTN_W 130
#define CONFIRM_BTN_H 60
#define CONFIRM_BTN_Y 380
#define CONFIRM_BTN_GAP 20

static void confirmCancelRect(int16_t *x, int16_t *y, int16_t *w, int16_t *h) {
  *w = CONFIRM_BTN_W;
  *h = CONFIRM_BTN_H;
  *x = SCREEN_W / 2 - CONFIRM_BTN_GAP / 2 - CONFIRM_BTN_W;
  *y = CONFIRM_BTN_Y;
}

static void confirmConfirmRect(int16_t *x, int16_t *y, int16_t *w, int16_t *h) {
  *w = CONFIRM_BTN_W;
  *h = CONFIRM_BTN_H;
  *x = SCREEN_W / 2 + CONFIRM_BTN_GAP / 2;
  *y = CONFIRM_BTN_Y;
}

static void drawConfirmScreen() {
  gfx->fillScreen(COLOR_BG);
  drawCenteredText(g_lang.confirmTitle, SCREEN_W / 2, 50, 2, COLOR_TEXT_DARK);

  if (g_ctx.selectedItem >= 0) {
    const MenuItem &item = g_menuItems[g_ctx.selectedItem];

    gfx->fillRoundRect(20, 140, SCREEN_W - 40, 160, 10, COLOR_CARD);
    gfx->drawRoundRect(20, 140, SCREEN_W - 40, 160, 10, COLOR_CARD_BORDER);
    drawCenteredText(item.name, SCREEN_W / 2, 190, 1, COLOR_TEXT_DARK);

    char priceBuf[24];
    snprintf(priceBuf, sizeof(priceBuf), "THB %u", item.priceBaht);
    drawCenteredText(priceBuf, SCREEN_W / 2, 250, 3, COLOR_PRIMARY);
  }

  int16_t x, y, w, h;
  confirmCancelRect(&x, &y, &w, &h);
  gfx->fillRoundRect(x, y, w, h, 8, COLOR_CANCEL);
  drawCenteredText(g_lang.confirmBtnCancel, x + w / 2, y + h / 2, 2, COLOR_TEXT_LIGHT);

  confirmConfirmRect(&x, &y, &w, &h);
  gfx->fillRoundRect(x, y, w, h, 8, COLOR_PRIMARY);
  drawCenteredText(g_lang.confirmBtnConfirm, x + w / 2, y + h / 2, 2, COLOR_TEXT_LIGHT);

  loggedFlush();
}

static void handleConfirmTouch(const TouchPoint &tp) {
  if (!tp.touched) return;

  int16_t x, y, w, h;

  confirmConfirmRect(&x, &y, &w, &h);
  if (pointInRect(tp.x, tp.y, x, y, w, h)) {
    transitionTo(STATE_QR);
    return;
  }

  confirmCancelRect(&x, &y, &w, &h);
  if (pointInRect(tp.x, tp.y, x, y, w, h)) {
    g_ctx.selectedItem = -1;
    transitionTo(STATE_MENU);
    return;
  }
}

// =========================================================================
// Screen 4 — QR (full-screen QR + amount overlay, drawing lives in qr.h)
// Advances via the demo bypass gesture only — see checkDoubleTapBypass above.
// =========================================================================
static void drawQrScreenWrapper() {
  uint16_t amount = (g_ctx.selectedItem >= 0) ? g_menuItems[g_ctx.selectedItem].priceBaht : 0;
  drawQRScreen(amount);
}

static void handleQrTouch(const TouchPoint &tp) {
  if (!tp.touched) return;
  if (checkDoubleTapBypass(_qrTapTracker, millis())) {
    transitionTo(STATE_PROCESSING);
  }
}

// =========================================================================
// Screen 5 — Processing (looping dot animation, no fixed duration)
// Advances via the demo bypass gesture only — see checkDoubleTapBypass above.
// =========================================================================
#define PROCESSING_DOT_COUNT 5
#define PROCESSING_DOT_RADIUS 10
#define PROCESSING_DOT_GAP 30
#define PROCESSING_DOT_Y 260

static void drawProcessingScreen() {
  gfx->fillScreen(COLOR_BG);
  drawCenteredText(g_lang.processingText, SCREEN_W / 2, 200, 2, COLOR_TEXT_DARK);

  uint32_t frame = (millis() - g_ctx.stateEnteredMs) / PROCESSING_FRAME_MS;
  uint8_t activeDot = frame % PROCESSING_DOT_COUNT;

  int16_t rowWidth = (PROCESSING_DOT_COUNT - 1) * PROCESSING_DOT_GAP;
  int16_t startX = SCREEN_W / 2 - rowWidth / 2;

  for (uint8_t i = 0; i < PROCESSING_DOT_COUNT; i++) {
    int16_t cx = startX + i * PROCESSING_DOT_GAP;
    uint16_t color = (i == activeDot) ? COLOR_PRIMARY : COLOR_CARD_BORDER;
    gfx->fillCircle(cx, PROCESSING_DOT_Y, PROCESSING_DOT_RADIUS, color);
  }

  loggedFlush();
}

static void handleProcessingTouch(const TouchPoint &tp) {
  if (!tp.touched) return;
  if (checkDoubleTapBypass(_procTapTracker, millis())) {
    transitionTo(STATE_FINISH);
  }
}

// =========================================================================
// Screen 6 — Finish (summary, auto-return after ~5s or on tap)
// =========================================================================
static void drawFinishScreen() {
  gfx->fillScreen(COLOR_BG);
  drawCenteredText(g_lang.finishTitle, SCREEN_W / 2, 160, 3, COLOR_SUCCESS);

  if (g_ctx.selectedItem >= 0) {
    const MenuItem &item = g_menuItems[g_ctx.selectedItem];
    drawCenteredText(item.name, SCREEN_W / 2, 220, 1, COLOR_TEXT_DARK);

    char totalBuf[32];
    snprintf(totalBuf, sizeof(totalBuf), "%s%u", g_lang.finishTotalPrefix, item.priceBaht);
    drawCenteredText(totalBuf, SCREEN_W / 2, 260, 2, COLOR_PRIMARY);
  }

  loggedFlush();
}

static void handleFinishTouch(const TouchPoint &tp) {
  if (!tp.touched) return;
  g_ctx.selectedItem = -1;
  transitionTo(STATE_WELCOME);
}

// =========================================================================
// Top-level dispatch — called from Satu_EV_Demo.ino's loop()
// =========================================================================
static AppState _lastDrawnState = (AppState)-1;

static void drawCurrentScreen() {
  switch (g_ctx.state) {
    case STATE_WELCOME:    drawWelcomeScreen(); break;
    case STATE_MENU:       drawMenuScreen(); break;
    case STATE_CONFIRM:    drawConfirmScreen(); break;
    case STATE_QR:          drawQrScreenWrapper(); break;
    case STATE_PROCESSING: drawProcessingScreen(); break;
    case STATE_FINISH:     drawFinishScreen(); break;
  }
}

// Redraws once on state entry, keeps the Processing animation ticking,
// and checks the Finish screen's auto-return timeout. Call every loop().
static void tickScreen() {
  if (g_ctx.state != _lastDrawnState) {
    _lastDrawnState = g_ctx.state;
    drawCurrentScreen();
    return;
  }

  if (g_ctx.state == STATE_PROCESSING) {
    static uint32_t lastFrameMs = 0;
    uint32_t now = millis();
    if (now - lastFrameMs >= PROCESSING_FRAME_MS) {
      lastFrameMs = now;
      drawProcessingScreen();
    }
  } else if (g_ctx.state == STATE_FINISH) {
    if (millis() - g_ctx.stateEnteredMs >= FINISH_AUTO_RETURN_MS) {
      g_ctx.selectedItem = -1;
      transitionTo(STATE_WELCOME);
    }
  }
}

// Call every loop() with the latest touch-down edge (see pollTouchEdge()).
static void handleTouch(const TouchPoint &tp) {
  switch (g_ctx.state) {
    case STATE_WELCOME:    handleWelcomeTouch(tp); break;
    case STATE_MENU:       handleMenuTouch(tp); break;
    case STATE_CONFIRM:    handleConfirmTouch(tp); break;
    case STATE_QR:          handleQrTouch(tp); break;
    case STATE_PROCESSING: handleProcessingTouch(tp); break;
    case STATE_FINISH:     handleFinishTouch(tp); break;
  }
}
