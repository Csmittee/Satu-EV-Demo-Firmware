# CC_PROMPT_fix_touch_garbage_read_v1.md
> ✅ COMPLETE — 2026-08-08 — touch reset timing fix (garbage 4095,4095 reads)
> Repo: Satu-EV-Demo-Firmware (existing)
> Session type: Fix — grounded in real captured hardware data, not speculation

---

## 1. CC INTRO

Read and execute: CC_PROMPT_fix_touch_garbage_read_v1.md

Read CLAUDE.md, RULES.md, PROJECT_STATE.md first. Do not re-run the
earlier instrumentation prompt if it's sitting in docs/prompts/inbox/
— the data it would have gathered was already captured manually via
Serial Monitor. Move it to archive/ unexecuted, note why in
CC_CHAT_LOG.md.

---

## 2. EVIDENCE (real, from the physical board, Serial Monitor capture)

Owner ran Test 2 (touch calibration) and captured raw output:

```
Calib[TL] expected=(20,20)   raw=(4095,4095)
Calib[TR] expected=(300,20)  raw=(4095,4095)
Calib[BL] expected=(20,460)  raw=(4095,4095)
Calib[BR] expected=(300,460) raw=(4095,4095)
Calib[C]  expected=(160,240) raw=(264,20)
Calib[TL] expected=(20,20)   raw=(82,158)     ← retap, same corner as first
```

All four corners — four different physical tap locations — returned
the identical raw value `(4095, 4095)`. That's `0x0FFF` for both X and
Y, the max a 12-bit field holds — consistent with `readTouch()` in
display.h receiving all-`0xFF` bytes over I2C on these reads (8 bytes
still came back, so the existing `requestFrom(...) != 8` check didn't
catch it — the transaction "succeeded," the content was garbage). The
center tap and a retap of the same top-left corner both returned
different, plausible small values.

This is a strong, evidence-based lead — not proven yet — for why
Confirm-button (Test 3), and the Test 4/5 menu entries, appeared
unresponsive: if a garbage max-value reading is treated as a real
touch-down, it consumes the touch-edge state (`_wasTouched` in
`pollTouchEdge()`), and the next genuine tap doesn't register as a new
edge until the (phantom) touch "lifts." Whether a given screen appeared
to work was likely down to whether a garbage read happened to land
first, not whether that screen's logic is broken.

---

## 3. TASKS

### 3.1 — Confirm the hypothesis before fixing
Add a temporary check (or reuse existing Serial logging) that flags
specifically when `data[2..5]` in `readTouch()` are all `0xFF`, or
when the computed x/y equal exactly `4095,4095`, separately from
normal touch logging. This isolates "garbage read" from "real touch
that happens to be near max" (unlikely given the screen is only
320×480, but confirm rather than assume).

### 3.2 — Investigate root cause in LIBRARY_axs15231b.md
Likely candidates, in order of likelihood — investigate before
picking one:
- The chip isn't ready to answer the touch-query command immediately
  after the reset sequence in `displayInit()` (`TOUCH_RST` toggled
  low/high with only 20-50ms delays) — some touch controllers need
  longer, or need one discarded "warm-up" read before real data flows.
- The command bytes (`0xB5 0xAB 0xA5 0x5A 0x00 0x00 0x00 0x08`) are a
  community-reverse-engineered guess (per LIBRARY_axs15231b.md) — it's
  possible this returns valid-looking-but-wrong data before the
  touch engine's first real scan cycle completes, independent of
  reset timing.
- Search for whether other community AXS15231B implementations
  discard the first N reads after boot, or check a status/ready bit
  elsewhere in the response before trusting x/y.

### 3.3 — Fix
Once cause is identified, implement the actual fix — likely one of:
- A longer or different delay sequence around `TOUCH_RST` in
  `displayInit()`, if it's a reset-timing issue
- Detect and discard reads where all of `data[2..5]` are `0xFF` (or
  equivalently x==4095 && y==4095) — treat as `touched=false`, not a
  real touch, directly in `readTouch()`
- Whichever the evidence in 3.1/3.2 actually points to — do not apply
  both defensively without knowing which one is the real cause;
  document which it was and why in CC_CHAT_LOG.md

Apply the fix in BOTH `display.h` (real demo) and `selftest/display.h`
(self-test) — these are duplicated files per the bootstrap prompt's
sync-warning convention, both need the same fix or they'll diverge.

---

## 4. DO NOT TOUCH

- ui_screens.h's button geometry, hit-test logic — not implicated by
  this evidence, don't touch on a hunch
- Anything WiFi/backend-related

---

## 5. VERIFICATION

- [ ] Root cause stated plainly in CC_CHAT_LOG.md — which hypothesis
      from 3.2 it actually was, with evidence, not just "fixed it"
- [ ] Fix applied to both display.h copies
- [ ] CI green
- Owner must re-flash the self-test build, re-run Test 2 (calibration)
  and Test 3 (isolated Confirm-geometry hit-test) specifically, and
  confirm: no more `4095,4095` readings, and Test 3 now responds
  reliably on first tap, not just sometimes

---

## 6. MANDATORY CLOSING

1. CC_CHAT_LOG.md — root cause, fix applied, what owner must verify
2. RULES.md — new rule documenting this finding (touch read validation
   needed / reset timing requirement — whichever it turns out to be)
3. PROJECT_STATE.md — update
4. Version-bump headers on changed files
5. Push, confirm CI green (both compile-demo and compile-selftest)
6. Archive this prompt per R-8 (inbox/archive convention)

---

## 7. PAYMENT MODE REMINDER

Not applicable — no backend/payment calls.
