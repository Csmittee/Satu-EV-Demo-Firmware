# CC_PROMPT_fix_back_collision_and_instrument_v1.md
> ✅ COMPLETE — 2026-08-11 — Back zone repositioned (R-10) + always-on touch/flush/loop instrumentation (R-11); Test 3/slowness still open pending owner's Serial log
> Repo: Satu-EV-Demo-Firmware (existing)
> Session type: Fix (certain) + lightweight always-on instrumentation
> Owner note: multiple fix rounds already spent on touch — this session
> should fix what's certain and stop guessing on what isn't.

---

## 1. CC INTRO

Read and execute: CC_PROMPT_fix_back_collision_and_instrument_v1.md

Read CLAUDE.md, RULES.md, PROJECT_STATE.md, selftest/tests.h first.

---

## 2. BUG 1 — CONFIRMED, definite fix (owner reproduced directly)

The self-test's "< Back" button was built at the same screen position
as the TR (top-right) calibration target in Test 2. The original
bootstrap prompt (Section 4.1) specified Back should sit top-left,
away from all test targets — this wasn't followed. Owner confirmed:
tapping TR during calibration is consumed as "exit test," so TR can
never be calibrated — not a hardware issue, not a library issue, a
straightforward layout collision.

**Fix:** reposition Back to a location that does NOT overlap any of
Test 2's 5 targets (TL, TR, BL, BR, Center) with a generous margin —
check its hit-rect against all 5 target hit-rects programmatically
(not just eyeballed) before considering this done. State the exact
new coordinates and confirm no overlap in CC_CHAT_LOG.md.

---

## 3. UNRESOLVED — do not guess-fix, instrument instead

Owner also reports, after the PR #3 reset-timing fix:
- Self-test responds fast at first (Display sanity test), then
  becomes unresponsive after running for a while
- Test 3 (isolated Confirm-button geometry) still doesn't respond —
  unclear if this is the Bug 1 collision bleeding into it, the
  reset-timing fix being insufficient, or a separate time-based
  degradation
- General slowness, "have to hit Back and it takes so long"

This may partly be explained by Bug 1 alone (if Back's old position
also overlapped something used to navigate INTO Test 3 — check this
specifically). But do not assume Bug 1 explains all of it. Add
instrumentation so the next physical run produces hard data instead of
another subjective report:

Add Serial logging (already at 115200), always-on, not just for this
debug session — cheap enough to leave in permanently:
1. Every I2C touch transaction: success/fail, and if fail, which step
   failed (`Wire.endTransmission()` non-zero vs `requestFrom()` byte
   count mismatch) — these are different failure modes, log them
   differently
2. A running counter of I2C failures since boot, logged every 50
   failures (not every single one — avoid flooding)
3. Every `flush()` call's duration in ms
4. If any single `loop()` iteration exceeds 100ms, log it

Format, compact and grep-able:
```
[12345] TOUCH ok touched=1 x=170 y=395
[12346] TOUCH fail i2c_end_transmission
[12400] FLUSH dur=180ms
[50000] TOUCH fail_count=50
```

---

## 4. DO NOT TOUCH

- The reset-timing values from PR #3 (200ms/200ms) — do not change
  these speculatively in this session; if the log data from this
  session points at them being insufficient, that's a future session
  with evidence, not a guess now
- Confirm/Cancel button geometry in ui_screens.h — not implicated yet

---

## 5. VERIFICATION

- [ ] Back button's new hit-rect checked against all 5 Test 2 targets
      in code, not just visually — state the check was done
- [ ] CI green
- [ ] Owner re-runs: full Test 2 sequence (all 5 targets, including
      TR) with Back no longer in the way; Test 3; and if slowness
      recurs, sends back the Serial log this time, not a subjective
      description

---

## 6. MANDATORY CLOSING

1. CC_CHAT_LOG.md — Bug 1 root cause + fix, instrumentation added,
   explicitly note item 3 remains open pending real log data
2. RULES.md — new rule: test-target and control hit-rects must be
   checked for overlap programmatically, not eyeballed
3. PROJECT_STATE.md — update, mark touch investigation as "Bug 1
   fixed and confirmed by code check; timing/degradation still open,
   instrumented, awaiting next physical run's log"
4. Push, confirm CI green
5. Archive this prompt per R-8

---

## 7. PAYMENT MODE REMINDER

Not applicable.
