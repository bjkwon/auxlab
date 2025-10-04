# Legacy AUXLAB maintenance scope

Review date: 2026-10-03. Source baseline: `b742488` (1.71 matrix transposition update).

## Decision

Keep legacy AUXLAB as a compatibility application with a bounded stabilization effort. Preserve its existing Win32 GUI, legacy language behavior, and WinMM audio implementation for this effort. Make ownership and error handling reliable in the paths we support. Do not make complete GUI/engine separation a prerequisite for fixing bugs.

Replacing `sigproc` with `aux_engine`, or maintaining feature parity with auxlab2, would be a separate migration project. The current coupling makes that materially larger than a library substitution.

This document defines proposed implementation scope, not completed repairs or a Windows support guarantee. The review used source inspection on macOS; no Windows build, GUI execution, audio measurement, or sanitizer run was performed. Existing bug notes are leads, not proof that their reports still reproduce.

## What was reviewed

- Visual Studio solution/projects, property sheets, header/library copying, README and release/bug notes.
- Interpreter/application callbacks in `sigproc/sigproc.h`, `AstSig.cpp`, and builtin registration/audio dispatch.
- Console/debugger integration in `xcom/xcom.cpp` and recording callback execution in `xcom/showvar.cpp`.
- Recording status worker, WinMM capture/playback, graphics object inheritance/deletion, and the AUXLib embedding wrapper.
- Existing root-level AUX test scripts. No integrated automated test runner or tracked GitHub workflow was found in the inspected checkout.

This is a targeted architecture and maintenance-risk review, not an exhaustive correctness or security audit.

## Coupling map

| Boundary | Evidence | Maintenance consequence |
| --- | --- | --- |
| Interpreter to UI | `sigproc/sigproc.h:185` defines `CFuncPointers` with debugger, workspace, graphics property, and repaint callbacks; `CUDF` contains `map<HWND, RECT>` at line 214. `xcom/xcom.cpp:1690` installs application handlers. | There is an existing callback seam, but runtime state still contains window state. Use the seam for tests and local repairs; moving files alone will not separate the engine. |
| UI to interpreter | `xcom/xcom.cpp:1193` implements breakpoint handling by changing interpreter debug state and executing user input in the paused context. `showvar.cpp` executes recording UDFs and updates parent workspace values. | GUI changes can affect execution, scope, and callback lifetime. Tests must cover both results and UI state. |
| Graphics to runtime values | `graffy/graffy.h:83` makes `CGobj` inherit `CVar`. `graffy/graffy.cpp` uses `xscope`, converts values to pointers, and calls `erase_GO` during deletion. `sigproc/AstSig.cpp:1785` includes `graffy2.h`. | Handle identity, aliases, deletion, and window lifetime are intertwined. A modern engine replacement needs a graphics adapter and compatibility decisions. |
| Audio to UI/runtime | `wavplay/wavplay.h` exposes `CSignals`, `CVar`, `AstNode*`, HWNDs, and thread IDs. `wavplay/record.cpp` uses an application-owned initialization event; `showvar.cpp` evaluates callbacks and repaints graphics. | A recording fix commonly spans device transport, callback execution, runtime handles, and GUI teardown. |
| DLL to application internals | `AUXLib/AUXLib.cpp:33` supplies UI-related global symbols described as unused but required. It owns separate global interpreter/result registries. | AUXLib is a useful possible test entry point, but is not evidence of a fully isolated engine. Validate the wrapper before trusting it as a test oracle. |
| Build to checkout state | `auxlab.props` fixes a local output path and old SDK version. Header/library copy scripts copy only when destination files do not exist. | Incremental builds can retain stale copied inputs. Reproducibility comes before broad source changes. |

The central problem is shared ownership and execution state across modules, not simply Win32 API usage. The smallest valuable separation is between device transport, callback execution, and UI notification within audio maintenance.

## Initial defect and risk register

“Source-confirmed” means the problematic code path is present. User-visible frequency and exact symptoms still need Windows reproduction. Priorities refer to this maintenance project.

| ID | Priority / evidence | Finding and consequence | Required regression |
| --- | --- | --- | --- |
| L01 | P1, source-confirmed | `wavplay/record.cpp:181` calls `waveInOpen` with device 0, although `Capture` checks capabilities for `DevID` and the worker stores that ID. A nonzero selection can open the wrong device or fail against another device's format. | Select two distinct inputs; verify actual captured source and reported identity, including an unsupported format. |
| L02 | P1, source-confirmed unsafe ownership | `record.cpp:271` posts the address of reusable `send2OnSoundEven`; its PCM buffer is requeued immediately. `showvar.cpp:2130` reads the payload later. No per-message ownership or consumer acknowledgement protects the payload and samples. | Deliberately slow callbacks, numbered input blocks, and stop with pending messages; verify order, sample integrity, bounded memory, and no stale accesses. |
| L03 | P1, source-confirmed unsafe queue | `showvar.cpp:2106–2122` enqueues a pointer to a loop-local status object. `audio_capture_status.cpp:299–310` waits without a predicate and accesses/pops the shared queue after unlocking. The detached worker has an unconditional infinite loop. | Show/hide/close the status window during repeated capture; test spurious/early notifications and shutdown with an empty/nonempty queue. |
| L04 | P1, source-confirmed failure path | In `record.cpp:377–390`, `Capture` owns a joinable `std::thread`; the `MM_ERROR` branch throws before join/detach. Stack unwinding destroys the joinable thread and invokes termination rather than the intended error return. | Inject `waveInOpen` failure after capability checking; app must survive, report the error, and permit another attempt. |
| L05 | P1, source-supported hang risk | `record.cpp:242` waits indefinitely for initialization. `showvar.cpp:2082` signals readiness only after initial callback success; error handlers post a stop message, which the waiting capture worker cannot process. | Missing callback and callback-0 exception must cancel initialization and release all workers/resources without hanging. |
| L06 | P1 if AUXLib is shipped or used by tests, source-confirmed | `AUXLib/AUXLib.cpp:97–106` and `140–149` allocate with `calloc` and release with `delete[]`. Compute exceptions also bypass those normal cleanup sites. | Repeated successful, parse-error, and evaluation-error calls under heap instrumentation; use matching allocation and exception-safe ownership. |
| L07 | P1 investigation | `record.cpp:162–165` divides by `inBufferLen` and then `nBlocks`; short durations or a block size rounding to zero can make divisors zero. `Capture` itself does not validate these dimensions. Trace upstream restrictions before establishing script reachability. | Duration below one block, below one sample, zero/negative block size, nonfinite values, and large values; reject or handle safely before worker launch. |
| L08 | P2, source-confirmed build risk | `copy_headers.bat` and `copy_lib_files.bat` skip existing outputs; `auxlab.props` hardcodes `c:\devploy\auxlab\` and SDK `10.0.16299.0`. | Clean and incremental builds must use the same current source inputs and documented dependency set. |
| L09 | P2 investigation | Graphics objects are runtime values with pointer-based lookup and deletion through scopes; debugger/UI retains interpreter pointers. This establishes exposure to lifetime errors, not a specific reproduced bug. | Aliased handle deletion, close during callback, debugger abort/close, and workspace refresh after deletion. |

Recording format negotiation, sample-width conversion, final partial buffers, global recorder state, and playback cleanup also require tracing during implementation. Do not promise 24-bit capture, concurrent recording, or timing guarantees merely because an API parameter exists.

## Supported scope for the first maintenance release

- One documented Windows x64 environment, selected and recorded during build qualification. Retain the existing MSVC project structure initially. Do not promise all historical Windows versions or x86 compatibility without testing them.
- Existing console evaluation, UDF execution, workspace inspection, basic plotting, debugger stepping/continue/abort, and configuration/history persistence.
- Existing mono/stereo playback, queue/repeat, pause/resume/stop, and WinMM recording with one active recording session. Document and enforce this recording limit if necessary.
- Existing callback syntax and output attachment behavior. Callback errors must end a session coherently and leave the application usable.
- AUXLib and private/extension DLL workflows only to the extent explicitly included in the release inventory. Preserve bundled components; unsupported external plugins must be identified rather than silently assumed compatible.
- Targeted runtime correctness fixes for reproduced failures in supported workflows. Evaluate relevant `aux_engine` fixes by behavior and ownership assumptions; do not bulk-copy changed engine code.

## Ordered work packages and completion gates

### A. Establish a trustworthy baseline

1. Reproduce x64 Debug and Release builds from a clean checkout on Windows. Record compiler, SDK, dependencies, artifacts, and exact commands.
2. Make output paths configurable and copied headers/libraries refresh deterministically. Audit missing prebuilt inputs and parser-generation requirements without regenerating parser sources by default.
3. Record which DLLs, callback AUX files, settings, and external extensions are necessary for the supported application.
4. Run a baseline smoke checklist and preserve expected results for representative legacy scripts. Capture crash dumps/logs with useful symbols.

Exit: another clean Windows checkout can build and launch the staged application, including outside the source/build directories. If this cannot be achieved within a small initial timebox, re-scope before audio refactoring.

### B. Add the minimum regression infrastructure

1. Validate a small Windows test executable using AUXLib or direct legacy runtime linking with explicit application callback stubs. Choose the least invasive viable path after the build is working.
2. If AUXLib is used, fix L06 first. Test invalid/deleted context handles and success/error cleanup before using it to characterize interpreter behavior.
3. Run the existing indexing/filter/time-extraction scripts with machine-readable pass/fail and bounded execution time. Add focused regression cases for every repaired bug.
4. Add controllable audio failure injection at a narrow device-call boundary or a small fake transport. Real devices remain required for audio acceptance.

Exit: repeatable tests distinguish crashes, hangs, incorrect values, and expected errors; test failures produce a failing process exit code.

### C. Stabilize recording ownership and lifecycle

1. Repair L01, L04, L05, and invalid dimension handling as focused changes with regressions.
2. Replace posted pointers to reused metadata/PCM with owned blocks. Set an explicit bounded queue and overflow policy; a slow callback must never silently consume overwritten samples. Prefer a clear overrun error over silent loss for the initial release.
3. Replace the status queue's borrowed stack pointers with owned/value messages and synchronized access. Define worker termination and safe UI notification after window closure.
4. Give a session an explicit lifecycle: initializing, active, stopping, finished/failed. Ensure initialization failure, callback failure, stop, and app shutdown each release resources exactly once.
5. Keep AST/workspace references in the callback/application layer where practical. Device transport should pass audio data, session identity, and status. Extract only what is needed for ownership and testing; do not redesign all runtime handles.
6. Audit existing callback execution versus console/debugger access to shared runtime state. Serialize conflicting operations or reject them clearly; do not assume the legacy runtime supports concurrent evaluation.

Exit: all recording regressions pass on real Windows devices, including repeated sessions, slow/throwing callbacks, window closure, device errors, and application exit. No worker continues to reference a destroyed window/context/session.

### D. Apply selected playback, graphics, debugger, and runtime fixes

- Check playback queue/repeat/pause/resume/stop and early shutdown; repair reproduced cleanup and handle-state failures.
- Characterize graph alias deletion, figure closure during recording, and workspace refresh. Fix identified lifetime bugs locally while preserving legacy handle behavior.
- Verify nested UDF stepping, continue, abort, and error recovery; fix reproduced blockers rather than rebuilding the debugger.
- Revisit `known_bugs.txt` and release-note reports (including string concatenation corruption and `[x -y]` parsing). Admit only reproduced, bounded fixes with explicit compatibility expectations.

Exit: a finite, prioritized issue list is closed or documented with workarounds. Cosmetic/UI modernization and speculative cleanup do not extend the release indefinitely.

### E. Qualify and package

- Run the selected script suite against both the preserved baseline and patched build. Explain intentional behavior changes; legacy output alone is not an oracle for a known bug.
- Verify the staged x64 release on the chosen Windows environment with real audio input/output. Use two input devices for device-selection testing where available.
- Exercise at least 100 start/stop cycles and a 30-minute capture/playback soak with callback/GUI activity; inspect worker, handle, and memory growth for an ongoing leak trend. These are initial test targets, not proof of exhaustive reliability.
- Where toolchain support permits, use memory instrumentation on the repaired paths; supplement with Windows heap/handle diagnostics as needed.
- Publish a dependency/build record, tested workflow matrix, known limitations, regression results, and rollback artifact with the maintenance release.

Exit: release claims match measured coverage. A successful compile alone does not qualify audio or GUI behavior.

## Exclusions

- Complete GUI/core separation or replacing legacy `sigproc` with `aux_engine`.
- WASAPI/ASIO migration, new low-latency guarantees, or synchronized full-duplex audio features.
- New language semantics, full auxlab2 compatibility, or automatic backporting of every engine change.
- Graphics model replacement, debugger rewrite, broad smart-pointer conversion, formatting cleanup, or build-system migration to CMake.
- Concurrent multi-device recording, new sample formats, x86 qualification, and broad historical Windows support unless a concrete user requirement expands the scope.

If a critical defect cannot be repaired safely within these boundaries, explicitly narrow the supported workflow or assess migration to auxlab2. Do not silently turn maintenance into a second modernization project.

## Effort and decision checkpoints

Planning allowances for one developer familiar with AUX, with a working Windows machine and audio devices; these are rough engineering estimates, not commitments:

| Work | Initial allowance |
| --- | --- |
| A: build and baseline | 2–5 engineering days |
| B: regression infrastructure | 3–5 days |
| C: recording stabilization | 5–10 days |
| D: selected adjacent fixes | 3–7 days, capped by an explicit issue list |
| E: release qualification | 2–4 days |

Total initial envelope: approximately 15–31 engineering days. Dependency recovery, undiscovered runtime ownership defects, or external DLL compatibility can exceed it. Re-estimate after A/B and again after C; do not promise the full envelope before the first Windows reproduction.

First checkpoint: after at most five engineering days on A, decide whether reproducible builds and representative workflows justify continuing. Second checkpoint: after recording stabilization, decide whether actual legacy users still require the maintenance release or would be better served by closing equivalent gaps in auxlab2.

The immediate implementation batch is A plus B and the focused device/error/allocation regressions, followed by C. Full engine unification is deferred and would require its own language, graphics-handle, debugger, plugin ABI, and callback compatibility inventory.
