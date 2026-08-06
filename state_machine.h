#pragma once
// state_machine.h — state enum + transition table for the 6 demo screens.
//
// Welcome -> Menu -> Confirm -> QR -> Processing -> Finish -> Welcome
// Confirm can also go back to Menu (Cancel).

#include <Arduino.h>
#include "config.h"

enum AppState {
  STATE_WELCOME,
  STATE_MENU,
  STATE_CONFIRM,
  STATE_QR,
  STATE_PROCESSING,
  STATE_FINISH,
};

struct AppContext {
  AppState state;
  int8_t selectedItem;     // index into g_menuItems, -1 = none selected
  uint32_t stateEnteredMs; // millis() timestamp of last transition
};

static AppContext g_ctx = {STATE_WELCOME, -1, 0};

static void transitionTo(AppState newState) {
  g_ctx.state = newState;
  g_ctx.stateEnteredMs = millis();
}
