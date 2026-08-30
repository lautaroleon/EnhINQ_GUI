Qt based user interface for INQNET TDC

## Setting up on native Ubuntu

1. Install Ubuntu 22.04.1 (this version manages dual boot with Windows automatically).

2. Fix the date and time of the computer, then run `sudo apt-get update`.

3. Install build dependencies:

   ```
   sudo apt-get install git build-essential cmake perl ninja-build default-libmysqlclient-dev libxcb-xinerama0  libmysql++-dev libxcb-cursor0 '^libxcb.*-dev' libx11-xcb-dev libglu1-mesa-dev libxrender-dev libxi-dev libxkbcommon-dev libxkbcommon-x11-dev libpciaccess-dev libfontconfig1-dev   libfreetype6-dev    libx11-dev    libx11-xcb-dev    libxext-dev    libxfixes-dev    libxi-dev    libxrender-dev    libxcb1-dev    libxcb-cursor-dev    libxcb-glx0-dev    libxcb-keysyms1-dev    libxcb-image0-dev    libxcb-shm0-dev    libxcb-icccm4-dev    libxcb-sync-dev    libxcb-xfixes0-dev    libxcb-shape0-dev    libxcb-randr0-dev    libxcb-render-util0-dev    libxcb-util-dev    libxcb-xinerama0-dev    libxcb-xkb-dev    libxkbcommon-dev    libxkbcommon-x11-dev libgtk-3-dev libglib2.0-dev mesa-common-dev libgl1-mesa-dev libglu1-mesa-dev
   ```

4. Install Qt Creator (`qt-creator-opensource-linux-x86_64-11.0.0.run`, or a newer 11.x release).

5. Install Qt 6 itself using the **Qt Online Installer** (recommended — faster and simpler than building `qt-everywhere` from source, and this is the method that's been verified to work for this project):

   ```
   wget https://download.qt.io/official_releases/online_installers/qt-online-installer-linux-x64-online.run
   chmod +x qt-online-installer-linux-x64-online.run
   ./qt-online-installer-linux-x64-online.run
   ```

   Log in with a (free) Qt account, and in the component tree select your Qt 6.x version → Desktop gcc 64-bit, plus the **Qt SerialPort** module under Additional Libraries (this project needs it — without it you'll get `Unknown module(s) in QT: serialport` when configuring). This installs into `~/Qt/<version>/gcc_64` by default.

   *(Building `qt-everywhere-src` from source is still possible if you need a customized Qt build, but is far slower and isn't necessary for normal development on this project.)*

6. Open Qt Creator, open the `.pro` file of this project (`source/INQNET_GUI.pro`). In the kit picker / **Edit → Preferences → Kits**, make sure a kit exists pointing at the Qt version you just installed (`~/Qt/<version>/gcc_64/bin/qmake`) with a detected compiler and `gdb`.

7. Leave the **Debug** / **Release** / **Profile** build directories at their defaults — do not redirect them to the repo's data folder. The `.pro` file already has a build step (`copydata`) that automatically copies the runtime config/calibration files from `runtime_data/` into whichever build output folder is currently active, so the app finds them regardless of which configuration you build.

8. You should be ready to go — press Play (Release mode, or whichever configuration you're working in).

## Setting up on Windows via WSL2

This project can also be developed from a Windows workstation using WSL2 (Ubuntu), instead of a native/dual-boot install. The build itself is identical to native Ubuntu once WSL is set up — the differences are all in the initial environment setup:

1. Install WSL2 with an Ubuntu distro (Windows 11, or Windows 10 with the Store version of WSL). GUI apps display automatically via WSLg — no separate X server needed.

2. Install Qt Creator by extracting/installing it under your WSL home directory, then add it to your `PATH` so you can launch it from any terminal:

   ```
   echo 'export PATH="$PATH:/path/to/qtcreator-11.0.0/bin"' >> ~/.bashrc
   source ~/.bashrc
   ```

3. Generate an SSH key inside WSL (it has its own separate `~/.ssh`, distinct from any key you may already have on the Windows side) and add the public key to your GitHub account before cloning:

   ```
   ssh-keygen -t ed25519 -C "your_email@example.com"
   eval "$(ssh-agent -s)"
   ssh-add ~/.ssh/id_ed25519
   cat ~/.ssh/id_ed25519.pub   # paste this into GitHub → Settings → SSH and GPG keys
   git clone git@github.com:FermilabQuantumNetwork/MultiQB_GUI.git
   ```

4. Install the build toolchain and Qt 6 exactly as in the native Ubuntu steps above (apt dependencies, Qt Creator, Qt Online Installer + SerialPort module, kit setup).

5. Hardware note: if the time tagger hardware is connected to the Windows host rather than natively to Linux, WSL2 doesn't share Windows' USB devices by default. Use [`usbipd-win`](https://github.com/dorssel/usbipd-win) to pass a specific USB device through to WSL:

   ```
   usbipd list                       # on Windows, find the device's BUSID
   usbipd bind --busid <BUSID>       # on Windows, admin required
   usbipd attach --wsl --busid <BUSID>
   ```

   While attached, the device is exclusively owned by WSL (Windows can't use it at the same time). This works well for USB instruments that talk over standard USB/serial; it won't help if a given manufacturer's SDK requires its own Windows-only kernel driver rather than plain userspace USB access.

6. You may see a harmless startup warning from Qt Creator: `QStandardPaths: wrong permissions on runtime directory /mnt/wslg/runtime-dir, 0777 instead of 0700`. This is a known, long-standing WSLg quirk (see [microsoft/WSL#10896](https://github.com/microsoft/WSL/issues/10896)) and can be safely ignored — it doesn't affect the app.

## Repository layout notes

- `source/` — all project source, headers, `.ui` forms, and the `.pro` file.
- `lib/` — vendor-provided closed-source hardware libraries (`.so` files) linked at build time.
- `runtime_data/` — runtime configuration and calibration files (`LastSeasonVariables.conf`, `databaseInfo.json`, `exfofilters.json`, log files) that the compiled app expects to find alongside its executable. These are copied automatically into the active build output folder by the `.pro` file's `copydata` step — you should not need to move or copy them manually.
  - `databaseInfo.json` holds your real MySQL credentials, so it's gitignored and not tracked. Copy `databaseInfo.json.example` to `databaseInfo.json` and fill in your own server/user/password (see the MySQL server instructions below) before building.
- Build artifacts (object files, `Makefile`, generated `moc_*`/`ui_*.h`/`qrc_*` files, and the `debug/`/`release/`/`profile/` output folders) are intentionally not tracked in git — see `.gitignore`.

------------------------------------
Instructions for the MySQL server:

1. `sudo apt install mysql-server-core-8.0 mysql-client-core-8.0` `sudo apt-get install mysql-server`

2. `sudo mysql_secure_installation`
   in other distros usually in this step I can set the root password but in this case I have to do this instead:
3. `sudo mysql`

4. `ALTER USER 'root'@'localhost' IDENTIFIED WITH mysql_native_password by 'newpassword';`

now the database for the GUI:

5. `create database INQNET_GUI;`

6. `create user 'GUI3'@'localhost' identified with mysql_native_password by 'newpassword';`
   here you can change GUI3 and newpassword but remember to record the changes in `runtime_data/databaseInfo.json` (copy it from `databaseInfo.json.example` if it doesn't exist yet — it's gitignored since it holds real credentials). `localhost` can be changed to an IP on your local network if your DB and GUI are on different computers.

7. `grant all privileges on *.* to 'GUI3'@'localhost';`

8. `FLUSH PRIVILEGES;`
