# INQNET TDC GUI

Qt-based user interface for the INQNET time-to-digital converter setup. Drives
a qutools quTAG and a Swabian Instruments Time Tagger (Ultra/X), computes QKD
histogram and logic-combination statistics, and logs to MySQL.

Builds on **Linux** and **Windows**. Neither platform needs Qt Creator — see
[Building without an IDE](#building-without-an-ide). Qt Creator remains
perfectly usable for day-to-day development if you prefer it.

---

## Table of contents

- [Prerequisites](#prerequisites)
- [Building without an IDE](#building-without-an-ide)
- [Building in Qt Creator](#building-in-qt-creator)
- [Deploying to a machine with no Qt installed](#deploying-to-a-machine-with-no-qt-installed)
- [Vendor SDKs](#vendor-sdks)
- [Runtime configuration](#runtime-configuration)
- [MySQL](#mysql)
- [Repository layout](#repository-layout)
- [Licensing and redistribution](#licensing-and-redistribution)
- [Troubleshooting](#troubleshooting)

---

## Prerequisites

### Both platforms

| Component | Notes |
| --- | --- |
| Qt 6.x | Must include the **SerialPort** module (`ovdl.cpp` needs it). Not installed by default. |
| qutools quTAG SDK | `tdcbase` — ships in this repo under `lib/`. |
| Swabian Time Tagger SDK | Installed separately, see [Vendor SDKs](#vendor-sdks). |
| MySQL server | Only to actually log data; the app builds and runs without it. |

Install Qt with the [Qt Online Installer](https://www.qt.io/download-qt-installer)
(a free Qt account is required). In the component tree select your Qt 6.x
version, the desktop kit for your platform, **and Qt Serial Port** under
*Additional Libraries*. Without SerialPort you get
`Project ERROR: Unknown module(s) in QT: serialport` at configure time.

### Windows

**Use the MSVC kit, not MinGW.** This is not a preference. The Swabian SDK's
C++ API passes STL types (`std::vector`, `std::function`, `std::string`) across
the DLL boundary, and its Windows import library exports MSVC-mangled symbols
that MinGW's linker cannot resolve at all. Even if it linked, mixing
libstdc++'s ABI with MSVC's STL across a DLL boundary is silent memory
corruption, not a link error.

- **Visual Studio 2022** with the *Desktop development with C++* workload
  (Community edition is fine).
- Qt kit: **`msvc2022_64`** — e.g. `C:\Qt\6.11.1\msvc2022_64`.

To add the MSVC kit and SerialPort to an existing Qt install:

```powershell
C:\Qt\MaintenanceTool.exe install `
  qt.qt6.6111.win64_msvc2022_64 `
  qt.qt6.6111.addons.qtserialport
```

(Substitute your version: `6111` = 6.11.1. Close Qt Creator first.)

### Linux (Ubuntu 22.04 or newer)

```bash
sudo apt-get update
sudo apt-get install git build-essential cmake perl ninja-build \
  default-libmysqlclient-dev libmysql++-dev \
  libxcb-xinerama0 libxcb-cursor0 '^libxcb.*-dev' libx11-xcb-dev \
  libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev \
  libxkbcommon-x11-dev libpciaccess-dev libfontconfig1-dev libfreetype6-dev \
  libx11-dev libxext-dev libxfixes-dev libgtk-3-dev libglib2.0-dev \
  mesa-common-dev libgl1-mesa-dev
```

The Qt Online Installer places Qt in `~/Qt/<version>/gcc_64` by default.

---

## Building without an IDE

Both scripts create their build tree under `build/` (gitignored), check
prerequisites up front with actionable errors, and create
`runtime_data/databaseInfo.json` from the template if it's missing.

### Windows

From any PowerShell prompt — the script locates Visual Studio via `vswhere`
and imports the MSVC environment itself, so an *x64 Native Tools* prompt is
**not** required:

```powershell
.\scripts\build-windows.ps1                  # release
.\scripts\build-windows.ps1 -Config Debug
.\scripts\build-windows.ps1 -Deploy          # build, then bundle into dist\
```

Useful overrides:

```powershell
.\scripts\build-windows.ps1 `
  -QtDir 'C:\Qt\6.11.1\msvc2022_64' `
  -TimeTaggerDir 'C:\Program Files\Swabian Instruments\Time Tagger'
```

Output: `build\windows-release\release\PROGRAM.exe`. It uses `jom` for a
parallel build when present (it ships with Qt Creator) and falls back to
`nmake`.

### Linux

```bash
chmod +x scripts/*.sh          # first time only
./scripts/build-linux.sh                          # release
./scripts/build-linux.sh --config debug
./scripts/build-linux.sh --qmake ~/Qt/6.11.1/gcc_64/bin/qmake
./scripts/build-linux.sh -j 8
```

Output: `build/linux-release/PROGRAM`.

### Doing it by hand

The scripts are thin wrappers; the underlying commands are just:

```bash
# Linux
mkdir -p build/linux-release && cd build/linux-release
~/Qt/6.11.1/gcc_64/bin/qmake ../../source/INQNET_GUI.pro CONFIG+=release
make -j$(nproc)
```

```powershell
# Windows, from an x64 Native Tools Command Prompt for VS 2022
mkdir build\windows-release && cd build\windows-release
C:\Qt\6.11.1\msvc2022_64\bin\qmake.exe ..\..\source\INQNET_GUI.pro -spec win32-msvc CONFIG+=release
jom -f Makefile.Release
```

---

## Building in Qt Creator

1. Open `source/INQNET_GUI.pro`.
2. Pick a kit — on Windows this **must** be an MSVC 2022 64-bit kit.
3. Leave the Debug/Release build directories at their defaults.
4. Build and Run.

Two Qt Creator specifics worth knowing:

- After changing `INQNET_GUI.pro`, use **Build → Run qmake** before building.
  A plain build does not pick up changed linker flags, and you will get a
  stack-overflow crash on startup if `/STACK` is missed.
- Running from Qt Creator works because its default working directory is the
  build root, where the `.pro`'s `copydata` step puts the config files. The
  executable itself lands in a `release/` subdirectory, so launching it
  directly from there won't find them — use the deploy script for that.

---

## Deploying to a machine with no Qt installed

The deploy scripts produce a self-contained `dist/INQNET_GUI/` directory. The
target machine needs **no Qt, no Qt Creator, and no Visual Studio**.

### Windows

```powershell
.\scripts\build-windows.ps1 -Deploy
# or, against an existing build:
.\scripts\deploy-windows.ps1 -ExePath .\build\windows-release\release\PROGRAM.exe
```

Produces roughly **40 MB / 45 files**. It contains:

- `PROGRAM.exe`
- the Qt runtime and plugins, via `windeployqt`
- the MSVC runtime DLLs, copied **app-locally** so nobody has to run
  `vc_redist.exe`
- the vendor DLLs `windeployqt` knows nothing about: `tdcbase.dll`,
  `FTD3XX.dll`, `libusb0.dll`, `TimeTagger.dll`, `okFrontPanel.dll`
- the runtime config files

Zip that folder, hand it over, run `PROGRAM.exe` from inside it.

The script deliberately excludes about 40 MB that this widgets-only app never
touches: the DirectX shader compilers (`dxcompiler.dll`, `dxil.dll`,
`d3dcompiler_47.dll`), the Mesa software-OpenGL fallback, and the 24.5 MB
`vc_redist.x64.exe` installer.

### Linux

```bash
./scripts/build-linux.sh
./scripts/deploy-linux.sh
```

Produces `dist/INQNET_GUI/` with `lib/`, `plugins/`, a `qt.conf`, and an
`INQNET_GUI` launcher that sets `LD_LIBRARY_PATH` and `cd`s into the bundle.
**Run the launcher, not `PROGRAM` directly.**

Base system libraries (glibc, libstdc++, X11, OpenGL, glib) are intentionally
*not* bundled — they come from the host. That keeps the bundle working across
reasonably similar distributions; it is not a substitute for an AppImage or
Flatpak if you need to target genuinely old or unknown systems. For that,
point [`linuxdeployqt`](https://github.com/probonopd/linuxdeployqt) at
`build/linux-release/PROGRAM` instead.

> The Windows path above is verified end-to-end: the resulting bundle launches
> with `PATH` reduced to `C:\Windows\system32` and exits cleanly. The Linux
> deploy script follows the standard bundling pattern but has **not** been
> run on a Linux machine yet — expect to iterate on the `ldd` filter list.

---

## Vendor SDKs

### qutools quTAG (`tdcbase`)

Ships in this repo:

- Linux: `lib/libtdcbase.so`, `lib/libftd3xx.so`
- Windows: `lib/DLL_64bit/` — `tdcbase.lib`, `tdcbase.dll`, plus `FTD3XX.dll`
  and `libusb0.dll`, which are `tdcbase`'s own runtime dependencies

Nothing in this project calls FTDI D3XX directly, so `ftd3xx` is not linked —
but `FTD3XX.dll` must still be present at runtime.

### Swabian Instruments Time Tagger

Install the vendor's package; it is **not** redistributed here in full.
Default locations:

- Windows: `C:\Program Files\Swabian Instruments\Time Tagger`
  (headers in `driver\include`, libraries in `driver\x64`)
- Linux: headers under `/usr/include`, libraries under `/usr/lib`

Override with `-TimeTaggerDir` (Windows) or `--timetagger-inc` (Linux), or
pass `TIMETAGGER_DIR=` / `TIMETAGGER_INC=` straight to `qmake`.

**On the vendored headers:** `source/` contains copies of `TimeTagger.h` and
`Iterators.h`. `TimeTagger.h` there is **2.16.2** and stale; the `.pro`
deliberately puts the installed SDK's include directory *ahead* of `source/`
so the SDK's own headers win and match the library you link against. Recent
SDKs dropped the top-level `Iterators.h` aggregator and moved measurement
headers into `measurements/`, which is why the SDK include path is mandatory
rather than optional — without it the build fails on
`measurements/ChannelGate.h`. Deleting the two vendored copies outright would
be cleaner and is worth doing once both platforms are confirmed working.

`TimeTagger.h` carries `#pragma comment(lib, "TimeTagger"/"TimeTaggerD")`, so
on MSVC the correct import library is selected automatically per
configuration. Don't add `-lTimeTagger` to `LIBS` — that forces the release
library into a Debug link alongside `TimeTaggerD`.

---

## Runtime configuration

`runtime_data/` holds the files the app reads at startup:

| File | Purpose |
| --- | --- |
| `databaseInfo.json` | MySQL credentials. **Gitignored.** Copy from `databaseInfo.json.example`. |
| `exfofilters.json` | EXFO tunable-filter addresses and settings. |
| `LastSeasonVariables.conf` | Saved GUI parameters, rewritten on exit. |
| `databaseLOG_Rates.txt`, `databaseLOG_logic.txt` | Log files. |

The app opens these by **bare relative path** (e.g.
`QFile("databaseInfo.json")`), so they resolve against the *working
directory*, not the executable's location. The `.pro`'s `copydata` step copies
them into the build output, and the deploy scripts place them beside the
executable.

---

## Fonts

The UI font is **bundled into the executable** as a Qt resource
(`source/fonts/DejaVuSansMono.ttf` and its Bold variant, declared in
`resources.qrc`) and registered at startup in `main.cpp`. Both platforms
therefore render the same face whether or not DejaVu is installed
system-wide.

This replaced a request for the family `"DejaVuSerif-Bold"`, which is not a
family name at all — `DejaVu Serif` is the family and `Bold` is a weight — so
it matched nothing on either platform. Only the `QFont::Monospace` style hint
took effect, and each platform then fell back to its own monospace default:
DejaVu Sans Mono on Ubuntu, **Courier New** on Windows. That was the entire
reason the two builds looked different.

The point size is pinned explicitly to 12. The old code left it unset, which
happened to resolve to `QFont`'s built-in 12pt default only because
`setFont()` ran before the `QApplication` existed; registering fonts requires
a live application instance, and moving the call after construction would
otherwise have let the platform default (9pt on Windows) shrink every widget.

To change the font, edit the family name in `main.cpp` and swap the `.ttf`
files in `source/fonts/` plus their entries in `resources.qrc`. A monospace
face is recommended: the numeric readouts and LCD displays assume digits
occupy a fixed width.

DejaVu is permissively licensed and free to redistribute — see
`source/fonts/LICENSE-DejaVu.txt`. This is unlike the vendor DLLs; see
[Licensing and redistribution](#licensing-and-redistribution).

---

## MySQL

> Qt does not ship the MySQL driver plugin (`qsqlmysql`). Until it is built
> against MySQL's client library, database connections fail at runtime even
> though the app builds and runs fine. See
> [Qt's SQL driver docs](https://doc.qt.io/qt-6/sql-driver.html#qmysql).

Server setup:

```sql
CREATE DATABASE INQNET_GUI;
CREATE USER 'GUI3'@'localhost' IDENTIFIED WITH mysql_native_password BY 'yourpassword';
GRANT ALL PRIVILEGES ON *.* TO 'GUI3'@'localhost';
FLUSH PRIVILEGES;
```

Record the user and password in `runtime_data/databaseInfo.json`. Replace
`localhost` with an IP if the database and GUI are on different machines.

On Ubuntu, `sudo apt-get install mysql-server` then `sudo mysql_secure_installation`.
If you cannot set a root password that way:

```sql
ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password BY 'newpassword';
```

Schema: a fixed, normalised layout — `runs(run_id, started_at)` plus
`tab1_readings`, `tab2_readings`, `tab2_filters`, each row-per-reading with an
indexed `run_id` foreign key. See `source/dbcontrol.{h,cpp}`.

---

## Repository layout

```
source/        All project source, headers, .ui forms, and INQNET_GUI.pro
source/fonts/  UI font, compiled into the binary as a Qt resource
lib/           Vendor quTAG binaries (Linux .so, Windows DLL_64bit/)
runtime_data/  Config and calibration files the app reads at startup
scripts/       Build and deploy scripts for both platforms
build/         Build output (gitignored)
dist/          Deploy bundles (gitignored)
```

Build artifacts, generated `moc_*`/`ui_*.h`/`qrc_*` files, `build/`, `dist/`
and vendor `*.pdb` symbols are all gitignored.

---

## Licensing and redistribution

**Read this before sharing a deploy bundle.** The vendor libraries in a bundle
are proprietary and not covered by whatever licence this project uses:

- `TimeTagger.h` states outright: *"Unauthorized copying of this file is
  strictly prohibited."*
- `tdcbase`, `FTD3XX`, `libusb0`, `okFrontPanel` each carry their own terms.

Redistributing them — including inside a zip handed to a colleague at another
institution, or attached to a public GitHub release — may breach those terms.
Options, roughly in order of safety:

1. Distribute the bundle **internally only**, to people already licensed for
   the hardware.
2. Ship the bundle **without** vendor DLLs and have each user install the
   vendor SDKs themselves. They need the drivers anyway to talk to hardware.
3. Get written redistribution permission from Swabian and qutools.

Note also that a public repository containing the vendor binaries already
redistributes them, independently of any release bundle.

**Credentials:** `runtime_data/databaseInfo.json` is gitignored, but a MySQL
password was committed to this project's earlier history in the upstream
`FermilabQuantumNetwork/MultiQB_GUI` repository. Rotate it on the server if
that hasn't been done; removing it going forward does not undo the leak.

---

## Troubleshooting

| Symptom | Cause and fix |
| --- | --- |
| `Unknown module(s) in QT: serialport` | SerialPort not installed. Add it with the Qt Maintenance Tool. |
| `Cannot open include file: 'measurements/ChannelGate.h'` | Time Tagger SDK include path not found. Pass `-TimeTaggerDir` / `--timetagger-inc`. |
| Unresolved `TimeTagger` symbols with mangled `_ZN...` names | You are building with MinGW. Switch to an MSVC kit. |
| `cannot open file 'TimeTaggerD.lib'` | Debug build, and the SDK's debug library is missing. Build Release, or install the full SDK. |
| Exits immediately, `0xC0000135` | Missing DLL. Run the deploy script. |
| Exits immediately, `0xC00000FD` (stack overflow) | `/STACK:8388608` not applied. In Qt Creator: **Build → Run qmake**, then rebuild. `MainWindow` is stack-allocated and large; Windows' default 1 MB stack is not enough. |
| Crash on exit, `0xC0000005` | Fixed. If it reappears, look for uninitialised pointers dereferenced in a destructor. |
| `could not find or load the Qt platform plugin` | Linux bundle missing `qt.conf` or `plugins/platforms/`. Re-run `deploy-linux.sh`. |
| Database never connects | `qsqlmysql` driver plugin not built — see [MySQL](#mysql). |
| Serial/delay-line device not found on Windows | `OVDL::connectOVDLmw()` scans `/dev/` for `ttyUSB*`, which is Linux-only. Needs porting to `QSerialPortInfo::availablePorts()`. |
| Font looks wrong / falls back to Courier New | The bundled font failed to load. `main.cpp` warns on stderr. Check `resources.qrc` still lists the `.ttf` files and that `rcc` ran. See [Fonts](#fonts). |
| `RCC Parse Error ... Expected '>'` | An XML comment in `resources.qrc` contains a `--` sequence, which XML forbids inside comments. |

### Memory use

The app reserves roughly **200 MB** at startup, mostly because
`MainWindow::setupQKDPlots()` eagerly allocates
`NUM_QKD_CHANNELS × MAX_WIN × MAX_QUBITS*2` = **56,000**
`QCPItemStraightLine` objects, nearly all of them permanently invisible. This
is expected in the current design, not a leak. Allocating those lines lazily —
only up to `in_QKD_numb × in_QKD_pxq`, which is what is ever displayed — would
cut startup memory dramatically and is the obvious first optimisation if it
ever matters.
