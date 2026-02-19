# Technical Debt Tracking

This document tracks known technical debt items (TODO/FIXME comments) in the
MangoWC codebase. These items represent future improvements or issues that need
investigation but don't block current functionality.

**Status:** All items are non-critical. The code functions correctly despite
these notes.

---

## TODO Items

### 1. Mouse Bindings: Scroll Wheel Support

**Location:** `src/mango.c:1803-1804`

**Current Code:**

```c
/* TODO: allow usage of scroll whell for mousebindings, it can be
 * implemented checking the event's orientation and the delta of the event
```

**Description:**
Mouse bindings currently don't support scroll wheel events. Implementation would
require checking the event's orientation and delta values.

**Priority:** Low  
**Estimated Effort:** Medium (2-4 hours)  
**Impact:** Quality of life improvement for users wanting scroll-based keybindings

---

### 2. Input Device Type Support

**Location:** `src/mango.c:3537`

**Current Code:**

```c
/* TODO handle other input device types */
```

**Description:**
The input device handling code may not support all input device types. Current
implementation covers keyboard, pointer, touch, tablet, and switch devices, but
there may be edge cases or newer device types not yet handled.

**Priority:** Low  
**Estimated Effort:** Small-Medium (1-3 hours)  
**Impact:** Better support for specialized input devices

---

### 3. Cursor Requirement Question

**Location:** `src/mango.c:3545`

**Current Code:**

```c
/* TODO do we actually require a cursor? */
```

**Description:**
Question about whether a cursor is always required in the compositor. This may
relate to headless or server-only configurations where a cursor might not be
needed.

**Priority:** Very Low  
**Estimated Effort:** Research + potential refactor (variable)  
**Impact:** Could enable headless compositor configurations

---

### 4. Cursor Initial Position Hack

**Location:** `src/mango.c:4782-4783`

**Current Code:**

```c
/* TODO hack to get cursor to display in its initial location (100, 100)
 * instead of (0, 0) and then jumping. still may not be fully
```

**Description:**
Current implementation uses a workaround to position the cursor at (100, 100)
instead of (0, 0) to avoid a visual jump. This is marked as a hack that should
be properly fixed.

**Priority:** Low  
**Estimated Effort:** Medium (requires investigation + fix, 3-6 hours)  
**Impact:** Minor cosmetic improvement during startup

---

## FIXME Items

### 5. Cursor Position After Monitor Power On

**Location:** `src/mango.c:5982-5983`

**Current Code:**

```c
/* FIXME: figure out why the cursor image is at 0,0 after turning all
 * the monitors on.
```

**Description:**
After turning all monitors on, the cursor image appears at position (0, 0)
instead of maintaining its previous position. Root cause is not yet understood.

**Priority:** Medium  
**Estimated Effort:** Medium-Large (requires debugging, 4-8 hours)  
**Impact:** User experience issue when recovering from monitor power-off state

---

## How to Contribute

If you're interested in addressing any of these items:

1. **Research:** Investigate the issue thoroughly
2. **Discuss:** Open a GitHub issue to discuss your approach
3. **Implement:** Create a PR with your fix
4. **Test:** Ensure the fix doesn't introduce regressions
5. **Update:** Remove the item from this document and the source code comment

---

## Statistics

- **Total Items:** 5
- **TODO Items:** 4
- **FIXME Items:** 1
- **Priority Breakdown:**
  - Very Low: 1
  - Low: 3
  - Medium: 1
  - High: 0

---

**Last Updated:** 2026-02-19  
**Documented By:** Code Review Process
