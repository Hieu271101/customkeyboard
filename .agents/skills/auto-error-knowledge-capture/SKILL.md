---
name: auto-error-knowledge-capture
description: Automatically create or update skills and knowledge notes when encountering errors, compilation failures, build issues, or completing tasks. Use this skill whenever you encounter an error, fail to compile, hit a build error, notice a pattern in debugging, complete a multi-step task, or the user says "note this down", "remember this", "save this fix", "what did we learn", or similar phrases about capturing knowledge. Also use after successfully resolving any error to record the solution for future reference.
---

# Auto Error & Knowledge Capture

This skill automatically creates and maintains project knowledge by capturing errors, fixes, and lessons learned as structured skills and notes.

## When to Trigger

- Compilation or build errors (pio run fails, compiler errors)
- Runtime errors (crashes, BLE failures, serial output anomalies)
- After successfully fixing any error (capture the solution)
- When completing complex multi-step tasks
- User says "note this", "remember this", "save this", "write this down"
- Repeated errors on the same issue (update existing knowledge)
- Discovery of API changes, deprecations, or version-specific behavior

## Knowledge Storage Structure

```
.agents/
├── skills/
│   ├── esp32-nimble-hid-keyboard/SKILL.md    # Project-specific skill
│   └── auto-error-knowledge-capture/SKILL.md  # This skill
└── notes/
    ├── errors/
    │   └── YYYY-MM-DD_error-signature.md      # Error logs (problem signature as key)
    ├── fixes/
    │   └── YYYY-MM-DD_fix-signature.md        # Solution records (same signature as error)
    └── lessons/
        └── YYYY-MM-DD_lesson-signature.md     # General lessons
```

## How Error Signature Lookup Works

Each error/fix/lesson is stored using a **descriptive signature name** that serves as the unique key. When an error is encountered:

1. **Extract the error signature** from compiler output, runtime behavior, or symptom description
2. **Search notes** by that signature name (or partial match) to see if it's already been solved
3. **If found**: apply the existing fix immediately, update it if needed
4. **If not found**: create a new note with that signature as the filename key

| Concept | Example Signature | Example Filename |
|---|---|---|
| Problem → Unique key | `ble-advertising-not-visible` | `2026-06-21_ble-advertising-not-visible.md` |
| Symptom → Unique key | `nimble-2.5.0-api-changes` | `2026-06-21_nimble-2.5.0-api-changes.md` |
| Compiler error → Unique key | `setAdvertisementType-not-found` | `2026-06-21_setAdvertisementType-not-found.md` |
| Runtime crash → Unique key | `guru-meditation-esp32-s3` | `2026-06-21_guru-meditation-esp32-s3.md` |
| User request → Unique key | `customize-hid-report-descriptor` | `2026-06-21_customize-hid-report-descriptor.md` |

## Automatic Capture Workflow

### Step 1: Detect the Event

When you encounter an error or complete a task, immediately identify:

- **Error type**: compilation, runtime, build, BLE, API, etc.
- **Context**: what you were trying to do, what changed
- **Root cause**: the actual underlying issue (if known)
- **Solution**: what fixed it (if resolved)
- **Prevention**: how to avoid this in the future

### Step 2: Create or Update Knowledge Files

#### For Errors (not yet resolved):

```markdown
# Error: [Brief Description]

**Date**: YYYY-MM-DD HH:MM
**Context**: What was being attempted
**Error Message**: Full error output

## What Happened
Description of the error sequence.

## Attempted Solutions
- [ ] Solution 1 — Result
- [ ] Solution 2 — Result

## Root Cause
(To be filled when resolved)
```

#### For Fixes (error resolved):

```markdown
# Fix: [Brief Description]

**Date**: YYYY-MM-DD HH:MM
**Error Reference**: Link to error file
**Context**: What was being attempted

## Problem
The error that occurred.

## Solution
The exact fix applied. Include code changes if relevant.

## Root Cause
Why this happened — the underlying reason.

## Prevention
How to avoid this error in the future.

## Verification
How to confirm the fix works (build command, test, etc.).
```

#### For Lessons (completed tasks, discoveries):

```markdown
# Lesson: [Topic]

**Date**: YYYY-MM-DD HH:MM
**Context**: What was being done

## What We Learned
Key insight or pattern discovered.

## Application
When/how to apply this knowledge in the future.

## Related
Links to other notes or relevant files.
```

### Step 3: Error Signature Extraction

When an error occurs, derive a unique signature name from the error itself:

**From compiler errors**: Use the first unique identifier (e.g., function name, macro name, error code).
- Compiler says: `error: 'setAdvertisementType' was not declared in this scope`
- Signature: `setAdvertisementType-not-found`
- Search: `ls .agents/notes/fixes/*setAdvertisementType*` or `ls .agents/notes/fixes/*not-found*`

**From runtime behavior**: Use the symptom description.
- Symptom: "ESP32-S3 not showing up in nRF Connect"
- Signature: `ble-advertising-not-visible`
- Search: `ls .agents/notes/fixes/*advertising*` or `ls .agents/notes/fixes/*not-visible*`

**From error codes/messages**: Use the key phrase.
- Error: "Guru Meditation Error: Core 1 panic'ed (LoadProhibited)"
- Signature: `guru-meditation-load-prohibited`
- Search: `ls .agents/notes/fixes/*guru*` or `ls .agents/notes/fixes/*load-prohibited*`

**From task context**: Use what you were doing.
- Task: "Fix BLE advertising"
- Signature: `ble-advertising-fix`
- Search: `ls .agents/notes/fixes/*ble*`

### Step 4: Lookup Before Create — Search by Signature

Before creating any new note, **always search** the existing knowledge base using the error signature:

```bash
# 1. Search fixes by signature keyword
ls .agents/notes/fixes/*<keyword>* 2>/dev/null

# 2. Search errors by signature keyword
ls .agents/notes/errors/*<keyword>* 2>/dev/null

# 3. Search lessons by signature keyword
ls .agents/notes/lessons/*<keyword>* 2>/dev/null

# 4. Search skill files for related content
grep -rl "<keyword>" .agents/skills/ 2>/dev/null
```

**If found**: 
- Read the existing note
- Apply the fix if it was already solved
- Update with new information if the problem evolved (add a new section with today's date)
- **Do NOT create a duplicate file**

**If not found**:
- Create a new file with the signature as its name
- The filename becomes the lookup key for future encounters

### Step 5: Update Project Skills

If the error/fix reveals a general pattern that applies to the project, update the relevant skill's SKILL.md:

- Add to "Common Pitfalls" section
- Update API version-specific notes
- Add new troubleshooting steps
- Update code patterns

## Example Walkthrough: Error Signature Lookup in Action

### Scenario: Encountering a Compiler Error

1. **Error occurs**: `error: 'setAdvertisementType' is not a member of 'NimBLEAdvertising'`
2. **Extract signature**: `setAdvertisementType` (the unknown identifier)
3. **Lookup**: `ls .agents/notes/fixes/*setAdvertisementType*`
4. **Cache miss** — first time seeing this → create new note
   - Filename: `2026-06-21_setAdvertisementType-not-found.md`
   - Signature key in filename: `setAdvertisementType-not-found`
5. **Solve it**: Discover that NimBLE 2.5.0 renamed it to `setConnectableMode(BLE_GAP_CONN_MODE_UND)`
   - Save the fix in the note with code example

### Scenario: Same Error, Next Time

1. **Error occurs again**: Same `setAdvertisementType` compiler error
2. **Extract signature**: `setAdvertisementType`
3. **Lookup**: `ls .agents/notes/fixes/*setAdvertisementType*`
4. **Cache hit** → file exists!
5. **Read fix**: Open the note, find the solution, apply it immediately
6. **No wasted time**: Problem solved in seconds instead of hours

### Scenario: Similar but Different Error

1. **Error occurs**: `error: 'setScanResponse' is not a member of 'NimBLEAdvertising'`
2. **Extract signature**: `setScanResponse`
3. **Lookup**: `ls .agents/notes/fixes/*setScanResponse*`
4. **Cache miss** — but related to previous error pattern (API rename in NimBLE 2.5.0)
5. **Check related**: `ls .agents/notes/fixes/*nimble*` → finds the NimBLE API lesson
6. **Use context**: The lesson already explains this is a NimBLE 2.5.0 rename pattern
7. **Create**: New fix for `setScanResponse`, cross-reference the existing lesson

## Quick Capture Templates

### Compilation Error Quick Note
```
### Compilation Error — [Date]
- **File**: [filename]
- **Error**: [error message]
- **Fix**: [what fixed it]
- **Prevention**: [how to avoid]
```

### BLE/Runtime Error Quick Note
```
### BLE Error — [Date]
- **Symptom**: [what happened]
- **Cause**: [root cause]
- **Fix**: [solution]
- **Verification**: [how to test]
```

### Task Completion Quick Note
```
### Task Complete — [Date]
- **Task**: [what was done]
- **Steps taken**: [key steps]
- **Lessons**: [what to remember]
- **Reusable patterns**: [patterns for future use]
```

## How This Skill Helps

1. **Builds tribal knowledge** — Never lose a solution to a hard problem
2. **Prevents repeated mistakes** — Check notes before re-encountering known issues
3. **Accelerates onboarding** — New sessions start with full context
4. **Creates searchable reference** — Find past solutions fast
5. **Updates project skills** — Knowledge flows into the right places

## File Naming Convention

The **signature** is the critical part — it's what you'll search by:

```
YYYY-MM-DD_<error-signature>.md
```

| Part | Purpose | Example |
|---|---|---|
| `YYYY-MM-DD` | Chronological order | `2026-06-21` |
| `<error-signature>` | **The search key** — descriptive, lowercase, hyphenated | `ble-advertising-not-visible` |
| `.md` | Markdown format | `.md` |

**Rule**: The signature should be short (2–6 words) and describe the problem/solution distinctly enough to be found by keyword search. Good signatures are self-explanatory:

- ✅ `ble-advertising-not-visible` — clear symptom
- ✅ `setAdvertisementType-not-found` — exact compiler error
- ✅ `nimble-2.5.0-api-changes` — broader topic
- ❌ `problem-with-ble` — too vague
- ❌ `fix-1` — not searchable
- ❌ `error` — useless as a lookup key

## Notes Integration

Always cross-reference with the main project skill:
- `esp32-nimble-hid-keyboard/SKILL.md` — Should contain the most current project knowledge
- If a fix changes fundamental project understanding, update the SKILL.md directly
- Notes files serve as detailed history; SKILL.md serves as the current reference

## Summary: The Knowledge Loop

```
Error Occurs
    │
    ▼
Extract Error Signature (e.g., "ble-advertising-not-visible")
    │
    ▼
Search .agents/notes/fixes/ for matching signature
    │
    ├── FOUND ──► Read fix, apply solution, optionally update note
    │
    └── NOT FOUND ──► Create new note with signature as filename
                          │
                          ▼
                      Solve error
                          │
                          ▼
                      Save solution in the note
                          │
                          ▼
                      Next time: Signature lookup hits → instant fix
```

This turns the `.agents/notes/` directory into a **searchable knowledge base** where:
- **Filename = problem signature** (the lookup key)
- **Content = solution** (the fix, code, prevention)
- **Search by keyword** = check if we've solved this before
