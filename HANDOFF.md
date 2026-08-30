# Handoff notes

Continuity notes for picking this project back up on a different machine
(specifically: moving to Windows). Read this alongside `README.md` (build/
setup instructions) — this file is about *what changed recently, what's
verified vs. not, and what to do next*.

## Where this repo came from

This is a personal, private continuation of
`FermilabQuantumNetwork/MultiQB_GUI` (org repo, remote `origin` in the
original checkout). The work below was done on a branch called
`code-layout-refactor` there, then squashed into a single initial commit
here in `EnhINQ_GUI` — so this repo's `main` has no commit-by-commit history
of its own. If you ever want to see *how* a specific change was made
step-by-step (not just the end result), the original repo's
`code-layout-refactor` branch still has it, as these commits (newest first):

```
dbdf068 Stop tracking runtime_data/databaseInfo.json; add .example template
80e2fda Fix data-saving gates to check the run, not just the connection
ab2200d Add SHOW_LOGO compile-time toggle for tab1's logo
e1bbd7c Replace per-run MySQL tables with a normalized runs/readings schema
e29877a Make the .pro file and remaining POSIX-only code Windows-buildable
d80882d Pop the Parameters tab out into its own window
e5751d3 Clean up dead commented-out code and add structural comments
38c5da2 Build the 4 QKD plot panels in code instead of mainwindow.ui
d82fac8 Move gui_param.ui to code; parameterize its QKD channel signals
4bac76f Collapse per-channel A/B/C/D duplication into channel arrays
```

## What changed, and why

1. **Layout moved from Qt Designer to code, for the 4 QKD histogram channels
   (A/B/C/D).** The tab1 plot panels, and the `gui_param` window's per-channel
   controls, used to be 4 near-identical blocks hand-duplicated in `.ui` XML.
   They're now built by a loop over `NUM_QKD_CHANNELS` (`mainwindow.h`), so
   adding a channel is bumping that constant instead of copy-pasting a 5th
   Designer widget. `qkd_param`'s window itself (the "Lines" popup) was
   deliberately left alone throughout — a separate window, not touched.
   Decided **against** making the channel count a runtime setting: it's
   unrelated to device hardware channel counts (`NQUTAGCHANNELS`/
   `NTTUCHANNELS`, both 5), and `DBControl`'s per-tab1 signals are fixed
   4-argument, so runtime flexibility would mean a DB-layer rewrite too.
2. **Dead code cleanup** across the project's own headers/sources (not the
   vendor SDK files: `qcustomplot.*`, `tdcbase.h`/`TimeTagger.h`,
   `Iterators.h`, `CustomStartStop.*`/`CustomDelayedChannel.*` — the last two
   are unmodified Swabian SDK example files). Removed commented-out
   superseded implementations, added purpose comments to each class.
3. **Windows build support**: `.pro` file's `LIBS`/`DEFINES`/include paths
   split into `unix{}`/`win32{}` scopes (critical detail: qutools' tdcdecl.h
   picks its DLL-import decoration off `#ifdef unix`, so those defines must
   stay unix-only). Removed `#include <unistd.h>` / dead `usleep`-based
   `SLEEP` macro from `qutag_adq`/`qutag_anl` (both dead code, and
   `unistd.h` doesn't exist on Windows anyway).
4. **MySQL schema redesigned** from one-CREATE-TABLE-per-run
   (`TAB1_<timestamp>`, `TAB2_<timestamp>`, wide per-channel columns) to a
   fixed, normalized schema: `runs(run_id, started_at)` plus
   `tab1_readings`/`tab2_readings`/`tab2_filters`, each row-per-reading with
   an indexed `run_id` foreign key. See `dbcontrol.h`/`dbcontrol.cpp`.
5. **Fixed a real bug**: disconnecting from the DB never ended the current
   run (`dbrunning` and `DBControl::connection_succesfull`/`current_run_id`
   were never reset), so a reconnect without an explicit "Create Tables"
   click would silently keep tagging new readings with a stale, pre-disconnect
   `run_id`. Now `disconnectFromServer()` and `turnONDB(0)` both properly end
   the run, and the data-saving gates check `dbrunning` (the run state)
   instead of the raw connection flag.
6. **`SHOW_LOGO` compile-time toggle** added in `mainwindow.h` (default
   `0`/off) for the logo in tab1's top-left corner (`ui->label_8`).
7. **Security**: `runtime_data/databaseInfo.json` (real MySQL credentials)
   is now gitignored and untracked; `databaseInfo.json.example` is the
   committed template. **The database password that used to be in this repo
   is still sitting in the original org repo's git history** (both an old
   commit of this file and a dead comment in `dbcontrol.cpp`) — rotate it on
   the actual MySQL server if that hasn't happened yet. Squashing this repo's
   history doesn't retroactively fix that; it just avoids repeating the leak
   going forward.

## What's verified, and what genuinely isn't

Everything above compiles cleanly (`qmake` + `g++`, full clean rebuilds, no
new warnings) in the Linux dev sandbox this work was done in. That sandbox
has **no display** and is **missing the proprietary vendor libraries**
(`libTimeTagger.so`, the real `tdcbase`/`ftd3xx` binaries), so none of this
has been:

- **Visually confirmed.** Item 1 above (the tab1 plot panels, `gui_param`
  rebuild, and the "Parameters" tab popping into its own window) rebuilds
  real widget layout from `.ui` XML by hand — mechanically careful, but
  never actually seen on screen. Worth an eyeball pass before trusting it.
- **Linked or run at all.** Nothing has executed against real hardware or a
  live MySQL server. The DB schema change in particular (item 4) has never
  been exercised against an actual database — the SQL is correct as written,
  but "correct as written" and "tested" are different claims.
- **Built on Windows**, obviously — there's no Windows toolchain in this
  sandbox. The `.pro` changes are reasoned through carefully (see the
  qutools `unix` macro detail above) but this is the first time any of it
  will hit an actual Windows compiler.

## Next steps on Windows

1. **Compiler: MSVC (Visual Studio 2022, "Desktop development with C++"
   workload) — not MinGW.** The vendor SDKs (Swabian Time Tagger, qutools
   tdcbase) pass STL types (`std::vector`, `std::function`, `std::string`)
   across the DLL boundary in their C++ APIs. MinGW's libstdc++ ABI is not
   compatible with MSVC's STL ABI — mixing them with a vendor DLL built for
   MSVC isn't a link error, it's silent memory corruption. Match whatever
   MSVC version each vendor's Windows SDK docs actually specify if they pin
   one.
2. Install the matching Qt kit from the Qt Online Installer (e.g. "Qt 6.x
   MSVC2022 64-bit" — **not** the MinGW kit).
3. Clone this repo.
4. Get the Windows builds of the three vendor SDKs (FTDI D3XX, qutools
   quTAG, Swabian Time Tagger) and drop the `.lib`/`.dll` files into `lib/`
   — see the comment block above the `win32{}` scope in
   `source/INQNET_GUI.pro` for the expected names; adjust the `LIBS` line if
   the actual filenames differ.
5. Copy `runtime_data/databaseInfo.json.example` to
   `runtime_data/databaseInfo.json` and fill in real credentials (this file
   is gitignored — it won't get committed).
6. Open `source/INQNET_GUI.pro` in Qt Creator (or build from an "x64 Native
   Tools Command Prompt for VS2022" with `qmake && nmake`/`jom`).
7. Known non-blocking gap, not fixed: `OVDL::connectOVDLmw()`
   (`mainwindow.cpp`) browses `/dev/` for `ttyUSB*` files — a Linux
   convention. Windows serial ports are named `COM1`, `COM3`, etc. and
   aren't files; this should move to
   `QSerialPortInfo::availablePorts()` at some point, but wasn't in scope
   for the Windows-buildability pass.
