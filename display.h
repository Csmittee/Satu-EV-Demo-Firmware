#pragma once
// display.h — QSPI bus + AXS15231B + Arduino_Canvas init, touch read,
// backlight control.
//
// R-2: never render directly via Arduino_AXS15231B — always through the
// Arduino_Canvas framebuffer (`gfx`), and always call gfx->flush() after
// drawing a screen. Nothing appears on the physical panel until flush().
// See LIBRARY_axs15231b.md for why (community-reported instability with
// direct rendering, independent of the 1.6.1 regression).

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
  // Reset timing matches Espressif's esp_lcd_axs15231b reference driver
  // (200ms hold-low + 200ms settle after release) — the original 20ms/
  // 20ms/50ms sequence was too short. Querying the touch engine before
  // it finished booting after reset returned garbage (0xFF bytes, read
  // as x=y=4095) that got treated as a real touch-down, consuming the
  // touch-edge state and making the next genuine tap not register until
  // the phantom touch "lifted." See LIBRARY_axs15231b.md and RULES.md R-9.
  digitalWrite(TOUCH_RST, HIGH);
  delay(10);
  digitalWrite(TOUCH_RST, LOW);
  delay(200);
  digitalWrite(TOUCH_RST, HIGH);
  delay(200);
  pinMode(TOUCH_INT, INPUT);

  return true;
}

static void setBacklight(bool on) {
  digitalWrite(GFX_BL, on ? HIGH : LOW);
}

// Raw AXS15231B touch read over I2C — no dedicated Arduino touch library
// exists for this chip (see LIBRARY_axs15231b.md). Single-touch only:
// the AXS15231B silicon supports real multi-touch per its datasheet, but
// this protocol path only exposes one point.
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
