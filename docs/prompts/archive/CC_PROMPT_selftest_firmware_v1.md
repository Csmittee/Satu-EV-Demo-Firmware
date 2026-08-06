# CC_PROMPT_selftest_firmware_v1.md
> ✅ COMPLETE — 2026-08-06 — self-test firmware (diagnostic, not a fix)
> Repo: Satu-EV-Demo-Firmware (existing)
> Session type: Build (new sketch, same repo)

---

## 1. CC INTRO

Read and execute: CC_PROMPT_selftest_firmware_v1.md

Read CLAUDE.md, RULES.md, PROJECT_STATE.md, KNOWLEDGE_MAP.md first.

---

## 2. CONTEXT

We built the demo firmware directly on unverified hardware assumptions
(community-sourced touch pins, a hand-written I2C touch protocol with
no reference library, unmeasured flush() timing) and are now debugging
a real bug (Confirm button unresponsive, Menu button works) without
knowing whether it's a hardware/coordinate issue or a logic issue.
That ambiguity is the actual problem, not the bug itself.

Fix: a standalone self-test sketch that isolates and directly measures
the hardware/library layer — touch coordinate accuracy across the full
panel, flush() timing, an isolated reproduction of the exact button
geometry that's currently broken — BEFORE debugging the demo firmware
further. Once these are known-good (or the self-test reveals exactly
what's wrong), any remaining demo bugs are UX/logic bugs, not hardware
mysteries.

This is expected to directly explain the current Confirm-button bug —
Test 2 and Test 3 below are built specifically to surface it.

---

## 3. NEW FILES

New sketch folder, same repo, does not touch the existing demo:
- `selftest/Satu_EV_SelfTest.ino` — setup()/loop() for this sketch
- `selftest/display.h` — copy of the real demo's display.h (QSPI/
  AXS15231B/Canvas init + readTouch()), not a shared include — Arduino
  sketch folders don't reliably support cross-folder includes. Comment
  clearly at the top: "Copy of ../display.h — if a fix is found here,
  port it back to the real display.h too, this file does not auto-sync."
- `selftest/config.h` — only what's needed: SCREEN_W/H, GFX_BL, QSPI
  pins, touch pins — copied from the real config.h, same sync warning
- `selftest/tests.h` — the 6 tests below

CI:
- Extend `.github/workflows/compile-check.yml` with a build matrix (or
  a second job) so BOTH `Satu_EV_Demo` and `selftest/Satu_EV_SelfTest`
  compile on every push, each uploading its own artifact.

---

## 4. TASKS

### 4.1 — Top-level test menu
Simple list, tap to enter a test, tap a fixed "< Back" zone (top-left
corner, same spot every test) to return to the menu. No need for this
UI to be pretty — legible and functional only.

### 4.2 — Test 1: Display sanity
Cycle fullScreen fills: red, green, blue, white, black, 1 second each,
looping. Confirms color channel order and full-panel coverage. Owner
watches and confirms colors look correct (not swapped) and cover the
whole panel (no offset band at any edge).

### 4.3 — Test 2: Touch coordinate calibration (built to catch the
current bug)
Draw 5 large tap targets: four corners (inset ~20px so they're
reachable) and dead center. On tap, show the RAW `x,y` readTouch()
returned, both on-screen and via Serial, next to the target's expected
position. Do this for all 5 without auto-advancing — owner taps all 5,
reads off 5 raw coordinate pairs.

This directly answers the open question: does the touch controller's
reported Y range actually reach into the bottom ~15% of the panel
(where the Confirm/Cancel buttons live at y=380-440 of 480)? If the
bottom-corner targets report Y values that don't line up with where
they're drawn, that's the root cause of the Confirm bug, found
directly rather than guessed at.

### 4.4 — Test 3: Isolated button hit-test at the exact broken geometry
Draw ONE button at x=170, y=380, w=130, h=60 — the real Confirm
button's exact coordinates, copied from `ui_screens.h`'s
`confirmConfirmRect()`. Tap it. Show "HIT" or show nothing (no
response) — do not soften this into a friendlier UI, the point is to
see whether this exact geometry, in total isolation from the rest of
the state machine, responds or not. If it doesn't respond here either,
the bug is confirmed hardware/coordinate-range, not application logic.
If it DOES respond here, the bug is something in the demo's state
machine or touch-edge logic specifically, not the geometry or the
touch read itself — equally useful to know.

### 4.5 — Test 4: flush() timing
Loop `fillScreen()` + `flush()` 20 times, measure each with `millis()`,
report min/max/average in ms, both on-screen and via Serial. Directly
quantifies the "slow transition" symptom — if average flush time is,
say, 250ms, that fully explains the delay without any logic bug at
all.

### 4.6 — Test 5: double-tap bypass gesture, isolated
Reuse `checkDoubleTapBypass()` logic from the real demo (`ui_screens.h`
— copy the function, same sync-warning comment as the other files).
Show on screen: single tap → "single", two taps within the window →
"double". Owner confirms the timing window feels right, not too
twitchy or too sluggish.

### 4.7 — Test 6: QR render, isolated
Draw the placeholder QR (reuse `qr.h`'s generation, copied with the
same sync warning) full-screen, nothing else on screen. Confirms qr.h
works correctly independent of the state machine.

---

## 5. DO NOT TOUCH

- The real `Satu_EV_Demo.ino` and its headers — this session only adds
  a new, separate sketch folder, does not modify the existing demo
- Do not attempt to "fix" the Confirm bug in this session — this
  session's job is to produce evidence, not a fix. The fix comes in
  the next session, once we know what's actually wrong.

---

## 6. VERIFICATION

No automated pass/fail here — this firmware's entire purpose is
producing information for the owner to read off the physical screen
and report back. State plainly in PROJECT_STATE.md: "This session
produces diagnostic data, not a verified fix. Results depend entirely
on what the owner observes when running each test on hardware."

- [ ] CI green for both sketches (compile only, not functional proof)
- [ ] Owner runs all 6 tests on the physical board and reports back:
      - Test 2: the 5 raw coordinate pairs
      - Test 3: HIT or no response
      - Test 4: average flush() ms
      - Test 5: does the double-tap window feel right
      - Test 6: does the QR render cleanly

---

## 7. MANDATORY CLOSING

1. CC_CHAT_LOG.md — new entry: what was built, explicitly note results
   are pending owner's physical test run
2. PROJECT_STATE.md — new entry, the "diagnostic data, not a verified
   fix" framing above
3. KNOWLEDGE_MAP.md — add the 4 new selftest/ files
4. RULES.md — no new rule yet; add one next session once the actual
   root cause from Test 2/3 is known
5. Version-bump headers on changed/added files
6. Push, confirm CI green for both sketches
7. Archive this prompt to docs/prompts/ stamped
   ✅ COMPLETE — [date] — self-test firmware (diagnostic, not a fix)

---

## 8. PAYMENT MODE REMINDER

Not applicable — no backend/payment calls in either sketch.
