# Changelog

Notable changes in each Perigee release. Full notes and downloads are on the
[releases page](https://github.com/BarMy49/perigee/releases), and the release
text lives in [packaging/release-notes](packaging/release-notes).

## 7.0.2 (2026-11-28)

Catches up with upstream [Moonlight Qt](https://github.com/moonlight-stream/moonlight-qt)
master up to `a85f933e`, the 50 upstream changes made since Perigee forked.

- Streaming library (moonlight-common-c) updated with about 55 stability fixes:
  race fixes in video, input and timing, sturdier STUN and RTSP handling, and
  correct setup when starting a second stream.
- Video renderer fixes for VAAPI, Vulkan, CUDA and D3D11, plus better V-Sync
  waiting and dropped-frame counting.
- Audio output restarts on its own if it fails during a stream.
- Gamepad and haptics fixes.
- Pairing key handling fix, and a command-line PIN must have 4 digits.
- Windows builds use Qt 6.12.1, upstream's newer prebuilt libraries and the
  latest MSVC runtime.
- libplacebo bumped to v7.387.0 in the AppImage and Flatpak recipes.

## 7.0.1 (2026-11-27)

- Choosing a session profile loads its settings, and while it is selected,
  setting changes are saved to it. **Current settings** keeps your values
  without a profile.
- Streams started from the command line begin from your current settings,
  including the selected profile. Command-line options still override them.
- Saved profiles keep the settings they last streamed with on first start.

## 7.0.0 (2026-11-27)

Renumbered from 6.2.0-perigee.2 to plain version numbers.

- Sentence-case buttons, menus and settings, larger settings descriptions and
  more room between preference groups.
- Computer cards show Online, Offline or Unpaired and are more compact. An
  empty list shows a hint and an "Add PC manually" button.
- Windows and Fedora ARM64 packages are built in GitHub Actions.
