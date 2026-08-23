# Sidereal Clock (C++ / Qt)

A native, fully re-implemented C++ port of the Sidereal Clock, built with
Qt Widgets and CMake so it compiles on both Linux and Windows from the
same source tree. Every part of the display — the analog dial(s), the
digital readout, the Polaris hour-angle position dial, and the settings
drawer — is drawn/built with real Qt widgets and `QPainter`, not a
wrapped web view.

## Features

Feature parity with the web version, plus one native-only bonus:

- Analog dial(s): Minimalist / Classic / Observatory / Skeleton styles, 12- or 24-hour face, continuous (non-ticking) sweep hands.
- One or two dials at once (sidereal, local, or both side by side).
- Digital readout: sidereal time (always 24h) and/or local time (12h/24h, with/without seconds), "Mechanical" (monospace) or "Engraved" (serif) numerals.
- Layout: analog only / digital only / both, with the digital block beneath, left, or right of the dial(s).
- The Polaris hour-angle position dial: a small instrument panel (top-left, out of the way of the main dial) showing Polaris riding the rim around a fixed North-Celestial-Pole crosshair, turning counter-clockwise as hour angle advances. The 0/24 mark can be set to top or bottom (the usual reticle conventions), and the dial and the HH:MM readout can each be shown or hidden independently.
- Location (lat/lon/name), any IANA time zone, live or custom clock source (with play/pause), a manual sync correction, and settings that persist between runs (via `QSettings` — the Windows registry or an INI file on Linux, automatically).
- Night mode (red-on-black), manual or automatic after astronomical dusk.
- **Bonus over the web version: a real NTP query.** A browser can't open a
  raw UDP socket, so the web version's "sync" was always manual (you type
  in a trusted time yourself). This native build can — Time & Sync →
  "Query this server now" sends an actual SNTP request over UDP to the
  server you pick and applies the resulting offset automatically.

## What's different from the web version

- **No built-in geolocation.** A browser can ask for the user's position;
  a desktop app has no standard cross-platform equivalent without adding
  a whole extra dependency, so this build sets location by hand only
  (name + latitude/longitude).
- The visual styling is a close match to the web version's palette and
  layout, redrawn with `QPainter` and Qt style sheets rather than
  CSS/SVG — small differences in spacing/fonts are expected, but the
  math (sidereal time, twilight, Polaris hour angle) is the exact same
  code, ported line-for-line and re-verified.

## Project layout

```
CMakeLists.txt
src/
  main.cpp            entry point
  MainWindow.*         top-level window, toolbar, footer, the tick loop
  DialWidget.*         the sidereal/local analog dial (reused for both)
  PolarisWidget.*       the Polaris hour-angle dial
  PlaqueWidget.*        the digital readout block
  SettingsPanel.*       the four-tab settings drawer
  AppState.*            all persisted settings (QSettings-backed)
  Astro.*               GMST / sun position / Polaris precession math
  SntpClient.*           the real UDP/SNTP client
  Theme.h                day/night color palettes
```

## Building on Linux

You need Qt 6 (Widgets + Network) and CMake. On Debian/Ubuntu:

```bash
sudo apt install build-essential cmake qt6-base-dev qt6-base-dev-tools
```

Then, from the project root:

```bash
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Release
cmake --build . -j$(nproc)
./SiderealClock
```

The binary dynamically links Qt6's shared libraries, so it will run on
any machine with the matching Qt6 runtime libraries installed (which the
`qt6-base-dev` package above already provides on the build machine — on
another machine you'd want the runtime-only packages, e.g. `libqt6widgets6`
and `libqt6network6`, or just rebuild from source there).

## Building on Windows

1. Install Qt 6 (Widgets + Network components) via the
   [Qt online installer](https://www.qt.io/download-qt-installer) — pick
   either the MSVC or the MinGW kit to match your compiler.
2. Install [CMake](https://cmake.org/download/) and either Visual Studio
   (2019+, with the "Desktop development with C++" workload) or MinGW-w64.
3. From a "Qt" or "Developer" command prompt (so `Qt6_DIR` resolves), in
   the project root:

   ```bat
   mkdir build
   cd build
   cmake .. -DCMAKE_PREFIX_PATH="C:\Qt\6.x.x\msvc2019_64" -G "Visual Studio 17 2022" -A x64
   cmake --build . --config Release
   ```

   (swap the `-G`/prefix path for your actual Qt install and MinGW if
   that's the kit you chose — Qt Creator can also just open
   `CMakeLists.txt` directly and handle all of this for you.)
4. The result is `Release\SiderealClock.exe`. To run it outside of Qt
   Creator/the build tree, either run `windeployqt SiderealClock.exe` in
   the output folder (bundles the needed Qt DLLs alongside it) or make
   sure Qt's `bin` directory is on `PATH`.

## Settings storage

Settings persist via `QSettings` under organization `jkobierczynski`,
application `SiderealClock` — an INI-style file under
`~/.config/jkobierczynski/` on Linux, or the registry under
`HKEY_CURRENT_USER\Software\jkobierczynski\SiderealClock` on Windows.

## Accuracy notes

Sidereal time (GMST/LST), the sun-altitude/twilight indicator, and
Polaris's precessed position all use the exact same formulas as the web
version — ported to C++ line-for-line, and Polaris's position was
re-verified against Astropy's FK5-equinox-of-date transform (agrees to
well under an arcsecond). See the web version's README for the fuller
accuracy notes; they apply here unchanged.

## License

MIT — see [LICENSE](LICENSE).
