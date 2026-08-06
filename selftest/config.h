#pragma once
// selftest/config.h — COPY of ../config.h, pared down to what the
// self-test sketch needs (pins, screen dims, a few colors). This file
// does NOT auto-sync with ../config.h — if a pin or color changes there
// (e.g. after the owner reports a mismatch from Test 2), port the change
// here too, by hand. See LIBRARY_axs15231b.md for where these values
// come from and what's still unverified.

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
// ---------------------------------------------------------------------
#define LCD_QSPI_CS  45
#define LCD_QSPI_SCK 47
#define LCD_QSPI_D0  21
#define LCD_QSPI_D1  48
#define LCD_QSPI_D2  40
#define LCD_QSPI_D3  39

// ---------------------------------------------------------------------
// Touch (I2C, shared chip with display) — COMMUNITY-SOURCED, NOT
// vendor-confirmed. TOUCH_INT has a conflicting community report
// (GPIO 3 instead of 11) — Test 2 in this sketch exists specifically to
// help resolve pin questions like this one empirically.
// ---------------------------------------------------------------------
#define TOUCH_SDA 4
#define TOUCH_SCL 8
#define TOUCH_RST 12
#define TOUCH_INT 11
#define TOUCH_I2C_ADDR 0x3B

// ---------------------------------------------------------------------
// Colors — RGB565, same formula as ../config.h
// ---------------------------------------------------------------------
#define RGB565(r, g, b) ((uint16_t)((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3)))

#define COLOR_BG            RGB565(0xF4, 0xF6, 0xF8)
#define COLOR_PRIMARY       RGB565(0x1E, 0x5A, 0x96)
#define COLOR_TEXT_DARK     RGB565(0x22, 0x2A, 0x33)
#define COLOR_TEXT_LIGHT    RGB565(0xFF, 0xFF, 0xFF)
#define COLOR_CARD          RGB565(0xFF, 0xFF, 0xFF)
#define COLOR_CARD_BORDER   RGB565(0xD0, 0xD6, 0xDC)
#define COLOR_SUCCESS       RGB565(0x2E, 0xB8, 0x72)
#define COLOR_FAIL          RGB565(0xC6, 0x3B, 0x3B)

// ---------------------------------------------------------------------
// Timing — same double-tap window as the real demo (Test 5 verifies
// this feels right, in isolation, before trusting it in ui_screens.h)
// ---------------------------------------------------------------------
#define DOUBLE_TAP_WINDOW_MS 400

// ---------------------------------------------------------------------
// Test 3 — the real Confirm button's exact geometry, copied from
// ui_screens.h's confirmConfirmRect() (SCREEN_W/2 + 10, y=380, 130x60
// at 320-wide screen). Hardcoded here deliberately, not derived from a
// shared macro, so this file stays a true "isolated reproduction" per
// the prompt (Section 4.4) — if ui_screens.h's geometry changes, this
// must be updated by hand to still be testing the real bug.
// ---------------------------------------------------------------------
#define CONFIRM_BTN_X 170
#define CONFIRM_BTN_Y 380
#define CONFIRM_BTN_W 130
#define CONFIRM_BTN_H 60
