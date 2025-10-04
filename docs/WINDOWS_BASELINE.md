# Windows maintenance baseline

Status (2026-10-03): work package A started; its exit gate is **not passed**.
Changes have been inspected on macOS only. No Windows build, batch execution,
application launch, GUI/audio test, or heap instrumentation has been run.
The original source baseline is `b742488`; retain its binaries and symbols when
qualifying the patched checkout. Do not start recording refactoring before A passes.

## Candidate environment and commands

Use a Windows x64 machine with Visual Studio 2019 C++ tools (v142) and a Windows
10 SDK. These match the tracked toolset; the exact OS, compiler and SDK versions
must be recorded from the machine used. The SDK defaults to installed `10.0`;
pass its exact version for repeatable qualification. This is a candidate environment,
not a supported-platform claim. Parser regeneration is not part of this build.

### Single-command local execution

Copy this checkout, including the uncommitted maintenance changes and `tools`
directory, to the Windows machine. Install Python 3.9 or later and Git in addition
to the C++ tools. Open an **x64 Native Tools Command Prompt** with v142 active,
change to the checkout root, then run:

```bat
py -3 tools\qualify_windows.py --sdk %WindowsSDKVersion:\=%
```

The command takes the exact SDK version from the developer prompt, removing its
trailing backslash. Alternatively pass the installed version explicitly, for
example `--sdk 10.0.19041.0`. A different VS installation can be used if its prompt
activates the installed v142 toolset; the runner checks `VCToolsVersion` and refuses
to silently substitute v143. It requires an x64 target prompt, Git, `cl`, `dumpbin`,
and MSBuild in PATH.

The runner:

1. Audits source/library inputs and records tool paths, SDK, compiler environment,
   revision, working-tree status and tracked diff.
2. Executes batch copy regressions in a disposable directory containing spaces,
   from an unrelated working directory. Checks refreshed headers/libraries,
   missing sources, early failure in lists and invalid destinations.
3. Runs serial x64 Debug and Release rebuilds followed by incremental builds in
   a fresh output directory containing spaces. A build exceeding 30 minutes is
   stopped with its child processes; override using `--timeout SECONDS`.
4. Stages `auxlab64.exe`, `auxp64.dll`, `AUXLib.dll` and available PDBs outside the
   checkout, saves binary hashes, and runs `dumpbin /dependents` on each binary.
   Copies manual script inputs into a separate `script-inputs` directory.

Logs, MSBuild binlogs and `report.json` are saved under
`artifacts\qualification\run-*`. The final output names the staging directory
under the Windows temporary directory; retain it with the report before temporary
files are cleaned up. Each run uses new directories and preserves earlier results.
Untracked source files are listed by Git status but are not included in the tracked
diff; preserve the complete checkout used with your qualification record.

A nonzero exit means preparation, a regression, build, staging or dependency
inspection failed. Start with `report.json` and its referenced failing log. A zero
exit means **build and staging passed only**: the report leaves work package A and
all manual acceptance checks pending. After a successful run, launch each staged
`auxlab64.exe` from its own staging directory and complete the table below. No
application process is launched automatically, and this runner does not yet test
header-triggered recompilation or a second clean checkout.

The runner's failure/timeout/staging logic has portable unit checks. Its batch
regressions and actual MSBuild execution still require the Windows run; skipped
Windows tests on macOS do not qualify them.

### Manual commands

From an x64 Native Tools Command Prompt, at the repository root:

```bat
mkdir artifacts\qualification
ver > artifacts\qualification\environment.txt
where msbuild >> artifacts\qualification\environment.txt
msbuild -version >> artifacts\qualification\environment.txt
cl /Bv >> artifacts\qualification\environment.txt 2>&1
set WindowsSDKVersion >> artifacts\qualification\environment.txt
set VCToolsVersion >> artifacts\qualification\environment.txt
git rev-parse HEAD >> artifacts\qualification\environment.txt
git status --short >> artifacts\qualification\environment.txt
py -3 tools\audit_build_inputs.py > artifacts\qualification\inputs.json
```

Stop on audit failure. Replace `SDK_VERSION` below with the installed version
recorded above, without its trailing backslash. Run each command separately and
stop on a nonzero exit code:

```bat
msbuild auxlab.sln /t:Rebuild /m:1 /p:Configuration=Debug /p:Platform=x64 /p:WindowsTargetPlatformVersion=SDK_VERSION /bl:artifacts\qualification\debug.binlog
msbuild auxlab.sln /t:Rebuild /m:1 /p:Configuration=Release /p:Platform=x64 /p:WindowsTargetPlatformVersion=SDK_VERSION /bl:artifacts\qualification\release.binlog
msbuild auxlab.sln /t:Build /m:1 /p:Configuration=Release /p:Platform=x64 /p:WindowsTargetPlatformVersion=SDK_VERSION /bl:artifacts\qualification\incremental.binlog
```

Use serial builds for this first qualification: legacy project events share the
`include` staging directory. Parallel builds and simultaneous configurations are
not qualified. Output defaults to `artifacts\x64\Debug` and
`artifacts\x64\Release`. To relocate it, add e.g. `/p:BuildDir=D:\auxlab-build\`.
When quoting a path with spaces, use `/p:BuildDir="D:\AUXLAB Build\\"`.
Verify this override in the binlog and repeat from a checkout path containing spaces.

Header copying now overwrites old copies and propagates failures. Scripts resolve
the repository from their own location; call sites quote paths. Release no longer
deletes shared headers. Prebuilt library copies also overwrite, and the common
link search path prefers `lib\$(Platform)` over staged output, so AUXLib can resolve
its inputs before the application pre-link event. Do not infer parallel-build safety
from these changes.

## Dependency and artifact inventory

- The solution contains 11 projects. `libsamplerate.vcxproj` and
  `lame_bj.vcxproj` are not solution projects; do not add them by assumption.
- `tools/audit_build_inputs.py` checks literal compile/resource paths for those
  11 projects and hashes seven expected x64 archives. The initial audit found
  109 unique source/resource inputs and no missing paths. It is not a full
  MSBuild evaluator, transitive include/resource audit, or ABI check.
- Checked-in archives: FFTW 3.3.9 and 3.3.4, libsndfile 1.0.28 and 1.0.26,
  libsamplerate, libmp3lame, and mpglib. Older library names remain in graffy's
  project metadata; changing their versions is not part of this patch.
  Archive names alone do not establish compiler/runtime compatibility or provenance.
- Win32++ headers are under `include/wxx820`. Generated parser sources
  `sigproc/psycon.tab.c`, `sigproc/psycon.yy.c` and the qlparse generated sources
  are tracked. Existing bison/flex command notes are historical regeneration
  instructions, not required build steps.
- Stage `auxlab64.exe` and `auxp64.dll` from the same configuration. The runtime
  explicitly loads `auxp64.dll`; the old README's `auxp.dll` label is imprecise.
  Its resources embed `auxp/default_callback_audio_recording.aux` and
  `auxp/f2_channel_stereo_mono.aux`. Include standalone callback files when testing
  user-file lookup; preserve their exact content in the qualification record.
- Include `AUXLib.dll` in the candidate inventory because the solution builds it.
  Do not use it as a regression oracle until L06 and context-handle validation
  have been repaired and tested in work package B.
- `aux_ext64_pitchtime.dll` is an external extension, not supplied by the tracked
  solution. Leave it unconfigured for the core baseline; separately record its
  origin/version if extension testing is included. Help CHM content is also a
  separate packaging input; test and record how missing help behaves.
- Record generated per-computer INI/history locations, AUX search paths, writable
  directory requirements, and any needed Visual C++ runtime installation on the
  actual Windows machine. Verify with `dumpbin /dependents` on each staged binary.
- Preserve Debug PDBs from intermediate project directories with the binaries and
  crash dumps. Existing Release settings disable some debug information; Release
  dump symbol coverage is still an open qualification item.

## Baseline acceptance record

Record pass/fail, actual output, logs/dumps, configuration, and exact revision for
each row; currently all Windows rows are **pending**.

| Check | Expected evidence |
| --- | --- |
| Clean Debug and Release builds | Both exit 0; binary logs and dependency hashes saved |
| Incremental inputs | Change a copied header in a disposable checkout; verify staged bytes update and dependent compilation occurs; restore and rebuild |
| Copy failure | Missing source and unwritable destination return nonzero and stop the build |
| Paths with spaces / relocated output | Build succeeds and all artifacts use the requested location |
| Staged launch | Launch outside source/build directories; record missing dependencies and files |
| Console and UDF | Arithmetic, variables, UDF result and intentional error; app remains usable |
| Script characterization | Run `test_indexing.aux`, `test_filt.aux`, `test_timeextract_condition.aux`; preserve outputs and prerequisites; do not assume every existing script is self-checking |
| Workspace / plot | Inspect values, open and close a basic plot, refresh workspace |
| Debugger | Nested UDF step, continue, abort, error recovery |
| Persistence | Close/reopen; verify intended settings/history locations and content |
| Audio baseline | Record device identity, basic playback/stop and capture callback behavior; capture failures as baseline defects |

Use a disposable checkout for input/failure experiments. Existing scripts can
have UI/audio dependencies; establish expected results before converting them to
automated assertions in B. Do not fabricate successful baseline values.

## Next gate

Obtain the Windows build/launch evidence above, resolve concrete build failures,
and select the qualified environment. A remains open until a second clean checkout
can build and launch the staged application. Then implement B, beginning with AUXLib
allocation/context safety if that wrapper is selected. L01–L07 runtime repairs,
recording ownership changes, and release qualification remain outstanding.
