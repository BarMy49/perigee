# Perigee

<img src="app/res/io.github.BarMy49.Perigee.svg" alt="The Moon at perigee around Earth" width="96" height="96">

Perigee is a low-latency remote desktop and game-streaming client for Linux
and Windows, based on [Moonlight Qt](https://github.com/moonlight-stream/moonlight-qt).
Connect to Apollo or Sunshine for remote work or play.

## Download

Get [Perigee 7.0.2](https://github.com/BarMy49/perigee/releases/tag/v7.0.2), or
see [all releases](https://github.com/BarMy49/perigee/releases). Releases use
plain version numbers such as v7.0.2. Choose the package for your system.

| Package | Use it on |
| --- | --- |
| [Fedora 45 RPM, x86_64](https://github.com/BarMy49/perigee/releases/download/v7.0.2/perigee-7.0.2-1.fc45.x86_64.rpm) | Fedora 45 on Intel or AMD. Supports native Wayland. |
| [Fedora 44 RPM, x86_64](https://github.com/BarMy49/perigee/releases/download/v7.0.2/perigee-7.0.2-1.fc44.x86_64.rpm) | Fedora 44 on Intel or AMD, with its matching system libraries. |
| [Fedora 45 RPM, ARM64](https://github.com/BarMy49/perigee/releases/download/v7.0.2/perigee-7.0.2-1.fc45.aarch64.rpm) | Fedora 45 on ARM64. |
| [Windows installer](https://github.com/BarMy49/perigee/releases/download/v7.0.2/PerigeeSetup-7.0.2.exe) | Windows 10 or 11, x64 or ARM64. |
| [Windows portable, x64](https://github.com/BarMy49/perigee/releases/download/v7.0.2/PerigeePortable-x64-7.0.2.zip) | Windows 10 or 11 on x64, without installing. |
| [Windows portable, ARM64](https://github.com/BarMy49/perigee/releases/download/v7.0.2/PerigeePortable-arm64-7.0.2.zip) | Windows 10 or 11 on ARM64, without installing. |

On Fedora, install the RPM with `sudo dnf install ./PACKAGE.rpm`. On Windows,
run the installer, or unzip the portable package and start `Perigee.exe`. The
portable package keeps everything in `perigee-data/` next to `Perigee.exe`.

Every package is built in GitHub Actions from the commit the release is tagged
on. The release also includes matching Fedora source RPMs and
[SHA256SUMS](https://github.com/BarMy49/perigee/releases/download/v7.0.2/SHA256SUMS). Check your downloads with
`sha256sum --check --ignore-missing SHA256SUMS`.

The packages are unsigned, so Windows may show a SmartScreen warning. The RPMs
pass install, startup and removal checks in their Fedora build containers, and
the Windows packages pass CI build and identity checks. Live streaming checks on
Windows and ARM64 are still pending. Flatpak and AppImage builds are not made
for 7.x yet; the 6.2.0 ones remain on the
[6.2.0-perigee.1 release](https://github.com/BarMy49/perigee/releases/tag/v6.2.0-perigee.1).

Windows builds use their own settings
(`HKCU\Software\io.github.BarMy49\Perigee`) and never share pairing with an
installed Moonlight. Moonlight import is Linux-only.

## Features

- **GNOME-style look.** A libadwaita-inspired design with a flat header bar,
  rounded cards and preference groups, sentence-case labels, and Adwaita Sans
  or Cantarell when installed. Each computer card shows whether it is Online,
  Offline or Unpaired. Settings adapt to the window width and remember your
  light or dark theme.
- **Desktop browsing.** Save favorites per host, navigate the host and app grids
  with the keyboard, and open tile menus with Shift+F10.
- **Session profiles.** Switch between Programming, Gaming and **Current
  settings** from the floating profile button on the computer and app pages, or
  in Settings. Choosing a profile loads its settings, and while it is selected,
  settings you change are saved to it. Streams started from the command line use
  the same settings, and command-line options still override them.
- **In-stream controls.** Edit optional shortcuts in **Settings → Keyboard
  shortcuts**. Open the keyboard-only control panel with **Ctrl+Alt+Shift+O**
  for available actions such as fullscreen, capture, clipboard and statistics.
  **Ctrl+Alt+Shift+Z** always releases capture.
- **Text clipboard.** Opt in per host from the PC tile menu. With clipboard
  permission, **Ctrl+Alt+Shift+J** sends text and **Ctrl+Alt+Shift+K** receives
  it by default. GNOME native Wayland also supports focused automatic sync.
  Other desktops and XWayland use manual transfer.
- **Apollo launch options.** Request a virtual display for a new launch when
  the host grants permission and reports its driver ready. App launch by UUID
  is optional and requires authenticated host support. Open a configured HTTPS
  admin page from the host menu.
- **SFTP handoff.** Open a configured host destination in your desktop's SFTP
  handler.
- **Moonlight coexistence.** Perigee has separate settings, cache and pairing
  identity. Import Moonlight preferences explicitly, with pairing credentials
  included only if requested. Portable mode uses `perigee-portable.dat` and
  `perigee-data/`. See [identity and import](docs/BRANDING.md).
- **Streaming support.** Hardware decoding, available codecs, audio and gamepad
  support depend on the host, client GPU, drivers and package.

## Build and develop

Install the dependencies in the [Fedora build guide](docs/FEDORA_BUILD.md).
From the source checkout, initialize the pinned submodules and build out of tree.

```bash
git submodule update --init --recursive
source_root=$(pwd -P)
mkdir -p ../perigee-build/debug
cd ../perigee-build/debug
qmake6 "$source_root/moonlight-qt.pro"
make -j1 debug
./app/perigee
```

- On Windows, run `powershell ./setup-deps.ps1`, then
  `scripts\build-arch.bat Release x64` from a shell with Qt's MSVC `bin` on `PATH`
- [Focused Linux tests](tests/README.md)
- Package recipes for [RPM](packaging/rpm/README.md),
  [Flatpak](packaging/flatpak/README.md) and [AppImage](packaging/appimage/README.md).
  Releases are built by the [Fedora RPM](.github/workflows/release-fedora-rpm.yml)
  and [Windows](.github/workflows/release-windows.yml) workflows.
- [Architecture](docs/ARCHITECTURE.md), [Apollo protocol](docs/APOLLO_PROTOCOL.md)
  and [upstream maintenance](docs/UPSTREAM_MAINTENANCE.md)

## License and attribution

Perigee derives from Moonlight Qt at `ac296864ace96f51ad75f78b90644bdbca76c332`.
See the [GPL license](LICENSE), [asset provenance](ASSET_NOTICES.md) for the
Earth and Moon icon, and [repository analysis](docs/REPOSITORY_ANALYSIS.md) for
dependency pins and source provenance.
