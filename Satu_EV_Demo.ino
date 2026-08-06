// Satu_EV_Demo.ino — EV charging station sales demo firmware
// Board: Guition JC3248W535C (ESP32-S3-WROOM-1-N16R8, AXS15231B QSPI+touch)
// Fully offline by design — no WiFi, no backend calls. See RULES.md R-4.
// setup()/loop() only — all logic lives in the headers below.

#include "config.h"
#include "display.h"
#include "state_machine.h"
#include "qr.h"
#include "ui_screens.h"

void setup() {
  Serial.begin(115200);
  if (!displayInit()) {
    Serial.println("Display init failed — check QSPI wiring (see LIBRARY_axs15231b.md)");
  }
}

void loop() {
  tickScreen();
  TouchPoint tp = pollTouchEdge();
  handleTouch(tp);
  delay(10);
}
