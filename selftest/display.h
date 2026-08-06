#pragma once
// selftest/display.h — COPY of ../display.h (QSPI bus + AXS15231B +
// Arduino_Canvas init, touch read, backlight control). Arduino sketch
// folders don't reliably support cross-folder includes, so this is a
// real copy, not a shared include. If a fix is found here (e.g. a
// different touch pin, a different touch byte-parsing offset), port it
// back to ../display.h too — this file does not auto-sync.
//
// R-2 still applies here: never render directly via Arduino_AXS15231B —
// always through the Arduino_Canvas framebuffer (`gfx`), and always
// call gfx->flush() after drawing a screen.

#include <Arduino_GFX_Library.h>
#include <Wire.h>
#include "config.h"

static Arduino_DataBus *g_bus = new Arduino_ESP32QSPI(
    LCD_QSPI_CS /* CS */, LCD_QSPI_SCK /* SCK */,
    LCD_QSPI_D0 /* D0 */, LCD_QSPI_D1 /* D1 */,
    LCD_QSPI_D2 /* D2 */, LCD_QSPI_D3 /* D3 */);

static Arduino_GFX *g_panel = new Arduino_AXS15231B(
    g_bus, GFX_NOT_DEFINED /* RST */, 0 /* rotation */, false /* IPS */,
    SCREEN_W, SCREEN_H,
    0 /* col offset 1 */, 0 /* row offset 1 */, 0 /* col offset 2 */, 0 /* row offset 2 */);

static Arduino_Canvas *gfx = new Arduino_Canvas(SCREEN_W, SCREEN_H, g_panel, 0, 0, 0);

struct TouchPoint {
  bool touched;
  int16_t x;
  int16_t y;
};

static bool displayInit() {
  pinMode(GFX_BL, OUTPUT);
  digitalWrite(GFX_BL, LOW);  // stay dark until first frame is flushed

  if (!gfx->begin()) {
    return false;
  }
  gfx->fillScreen(COLOR_BG);
  gfx->flush();
  digitalWrite(GFX_BL, HIGH);

  Wire.begin(TOUCH_SDA, TOUCH_SCL);
  pinMode(TOUCH_RST, OUTPUT);
  digitalWrite(TOUCH_RST, HIGH);
  delay(20);
  digitalWrite(TOUCH_RST, LOW);
  delay(20);
  digitalWrite(TOUCH_RST, HIGH);
  delay(50);
  pinMode(TOUCH_INT, INPUT);

  return true;
}

static void setBacklight(bool on) {
  digitalWrite(GFX_BL, on ? HIGH : LOW);
}

// Raw AXS15231B touch read over I2C — no dedicated Arduino touch library
// exists for this chip (see LIBRARY_axs15231b.md). Single-touch only.
static TouchPoint readTouch() {
  TouchPoint result = {false, 0, 0};

  static const uint8_t cmd[8] = {0xB5, 0xAB, 0xA5, 0x5A, 0x00, 0x00, 0x00, 0x08};

  Wire.beginTransmission(TOUCH_I2C_ADDR);
  Wire.write(cmd, sizeof(cmd));
  if (Wire.endTransmission() != 0) {
    return result;
  }

  if (Wire.requestFrom((int)TOUCH_I2C_ADDR, 8) != 8) {
    return result;
  }

  uint8_t data[8];
  for (uint8_t i = 0; i < 8; i++) {
    data[i] = Wire.read();
  }

  uint8_t touchCount = data[1];
  if (touchCount == 0) {
    return result;
  }

  result.touched = true;
  result.x = ((data[2] & 0x0F) << 8) | data[3];
  result.y = ((data[4] & 0x0F) << 8) | data[5];
  return result;
}
