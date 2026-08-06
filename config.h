#pragma once
// config.h — pins, colors, demo menu data, UI strings
// Board: Guition JC3248W535C (ESP32-S3-WROOM-1-N16R8, AXS15231B QSPI+touch)
// See LIBRARY_axs15231b.md for where these pins come from and what's
// still unverified against the physical unit.

#include <Arduino.h>

// ---------------------------------------------------------------------
// Display geometry
// ---------------------------------------------------------------------
#define SCREEN_W 320
#define SCREEN_H 480

// ---------------------------------------------------------------------
// Backlight
// ---------------------------------------------------------------------
#define GFX_BL 1

// ---------------------------------------------------------------------
// QSPI display bus — from Arduino_GFX's own bundled JC3248W535 preset
// (maintainer-validated, not community-guessed — see LIBRARY_axs15231b.md)
// ---------------------------------------------------------------------
#define LCD_QSPI_CS  45
#define LCD_QSPI_SCK 47
#define LCD_QSPI_D0  21
#define LCD_QSPI_D1  48
#define LCD_QSPI_D2  40
#define LCD_QSPI_D3  39

// ---------------------------------------------------------------------
// Touch (I2C, shared chip with display) — COMMUNITY-SOURCED, NOT
// vendor-confirmed. Owner must verify against the physical unit before
// first flash and report back any pin that doesn't match silkscreen
// labels. TOUCH_INT has a conflicting community report (GPIO 3 instead
// of 11) — try that if GPIO 11 produces no touch activity.
// See LIBRARY_axs15231b.md.
// ---------------------------------------------------------------------
#define TOUCH_SDA 4
#define TOUCH_SCL 8
#define TOUCH_RST 12
#define TOUCH_INT 11
#define TOUCH_I2C_ADDR 0x3B

// ---------------------------------------------------------------------
// Colors — neutral blue/white/dark-grey EV aesthetic
// Do NOT reuse Satu 1.0's temple gold/black theme — different customer.
// RGB565, computed at compile time.
// ---------------------------------------------------------------------
#define RGB565(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

#define COLOR_BG            RGB565(0xF4, 0xF6, 0xF8)  // near-white cool grey
#define COLOR_PRIMARY       RGB565(0x1E, 0x5A, 0x96)  // EV blue
#define COLOR_PRIMARY_DARK  RGB565(0x14, 0x3D, 0x66)
#define COLOR_ACCENT        RGB565(0x3D, 0x8B, 0xFF)
#define COLOR_TEXT_DARK     RGB565(0x22, 0x2A, 0x33)
#define COLOR_TEXT_LIGHT    RGB565(0xFF, 0xFF, 0xFF)
#define COLOR_CARD          RGB565(0xFF, 0xFF, 0xFF)
#define COLOR_CARD_BORDER   RGB565(0xD0, 0xD6, 0xDC)
#define COLOR_SUCCESS       RGB565(0x2E, 0xB8, 0x72)
#define COLOR_CANCEL        RGB565(0x9A, 0xA4, 0xAF)

// ---------------------------------------------------------------------
// Timing
// ---------------------------------------------------------------------
#define DOUBLE_TAP_WINDOW_MS   400
#define FINISH_AUTO_RETURN_MS  5000
#define PROCESSING_FRAME_MS    150

// ---------------------------------------------------------------------
// Demo menu — PLACEHOLDER DATA. Owner will replace with the real menu
// before the customer meeting.
// ---------------------------------------------------------------------
struct MenuItem {
  const char *name;
  uint16_t priceBaht;
};

#define MENU_ITEM_COUNT 6

static const MenuItem g_menuItems[MENU_ITEM_COUNT] = {
  {"Quick Top-up (15 min)", 100},
  {"Standard Charge (30 min)", 180},
  {"Fast Charge (45 min)", 250},
  {"Full Charge (60 min)", 320},
  {"DC Rapid (20 min)", 280},
  {"Overnight (8 hr)", 450},
};

// ---------------------------------------------------------------------
// UI strings — English only this pass, no Thai. Structured as a struct
// (lightweight version of Satu 1.0's g_lang_th / separate-strings-file
// pattern) so a g_lang_th table can slot in later without redesigning
// ui_screens.h — screens read from `g_lang.*`, never hardcode text.
// ---------------------------------------------------------------------
struct LangStrings {
  const char *welcomeTitle;
  const char *welcomeSubtitle;
  const char *welcomeTap;
  const char *menuTitle;
  const char *confirmTitle;
  const char *confirmBtnConfirm;
  const char *confirmBtnCancel;
  const char *qrAmountPrefix;
  const char *processingText;
  const char *finishTitle;
  const char *finishTotalPrefix;
};

static const LangStrings g_lang_en = {
  "EV Charging Station",
  "Satu Payment Demo",
  "Tap to begin",
  "Select Charging Option",
  "Confirm Order",
  "Confirm",
  "Cancel",
  "Amount: THB ",
  "Processing payment...",
  "Thank You!",
  "Total: THB ",
};

#define g_lang g_lang_en
