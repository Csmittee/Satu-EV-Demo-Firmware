// selftest/Satu_EV_SelfTest.ino — standalone hardware diagnostic sketch
// Board: Guition JC3248W535C (ESP32-S3-WROOM-1-N16R8, AXS15231B QSPI+touch)
//
// Isolates and directly measures the hardware/library layer (touch
// coordinate accuracy, flush() timing, the exact Confirm-button
// geometry currently unresponsive in the real demo, the double-tap
// bypass gesture, QR render) independent of the demo's state machine.
// Produces evidence for the owner to read off the physical screen and
// Serial monitor — this sketch does not fix anything itself, and does
// not touch Satu_EV_Demo.ino or its headers. See
// docs/prompts/archive/CC_PROMPT_selftest_firmware_v1.md.

#include "config.h"
#include "display.h"
#include "tests.h"

void setup() {
  Serial.begin(115200);
  if (!displayInit()) {
    Serial.println("Display init failed — check QSPI wiring (see ../LIBRARY_axs15231b.md)");
    return;
  }
  drawTestMenu();
}

void loop() {
  tickSelfTest();
  TouchPoint tp = pollTouchEdge();
  handleSelfTestTouch(tp);
  delay(10);
}
