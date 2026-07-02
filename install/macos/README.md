# macOS Build Notes

macOS support is experimental. The repository now includes a helper script that builds RPFM and lays it out in a minimal `.app` bundle, but it still needs broader local testing and notarization before it can be considered a maintained release path.

## Prerequisites

- Rust toolchain (stable, >= 1.85)
- Qt 6
- CMake
- pkg-config
- GNU Make (`gmake`)

With Homebrew:

```bash
brew install rust cmake pkg-config make qt@6
```

If `qt@6` is not linked into your default PATH, the build script and `rpfm_ui/build.rs` check the common Homebrew prefixes automatically. You can also set `QMAKE6` or `QMAKE` explicitly.

By default the script sets `RPFM_NO_KDE=1`. This uses Qt-only compatibility shims for KLineEdit, KMessageWidget, KTextEditor, KShortcutsDialog and related widgets, because readily available Homebrew taps do not currently provide the KF6 stack in the same way RPFM expects on Linux/Windows.

## Building

From the repository root:

```bash
./install/macos/build_macos.sh
```

The script runs `cargo build --release --bin rpfm_server --bin rpfm_ui`, compiles the custom Qt extensions library through `rpfm_ui/build.rs`, and creates:

```text
target/macos/RPFM.app
```

## Download Releases

End users should download packaged macOS builds from:

```text
https://github.com/gyasigit/rpfm-macos/releases
```

Release assets should be named clearly, for example:

```text
RPFM-macos-arm64-v5.0.5.zip
RPFM-macos-x64-v5.0.5.zip
```

Each zip should contain `RPFM.app` and retain the bundled `Contents/Resources/LICENSE` file.

## Using RPFM with WH3MM on macOS

This app opens and edits `.pack` files. It does not manage WARHAMMER III mod load order or launch the game. For that part of the workflow, use the WH3MM macOS fork:

```text
https://github.com/gyasigit/WH3-Mod-Manager-macos/tree/macos-compat
```

After building `RPFM.app`, associate `.pack` files with it:

1. In Finder, select any `.pack` file.
2. Choose File > Get Info.
3. Under Open With, choose `RPFM.app`.
4. Use Change All if you want every `.pack` file to open with RPFM.

Once associated, WH3MM's "Open in RPFM" action can hand `.pack` files to this app through macOS.

You can still build manually:

```bash
RPFM_NO_KDE=1 \
QMAKE6=/opt/homebrew/opt/qt@6/bin/qmake6 \
GMAKE=/opt/homebrew/opt/make/libexec/gnubin/make \
cargo build --release --bin rpfm_server --bin rpfm_ui
```

Set `RPFM_NO_KDE=0` only if you have a local KDE Frameworks 6 setup with the required headers, libraries and designer plugins.

## Total War Game Detection

RPFM has a macOS Steam install type for Total War: WARHAMMER III. It detects the Feral macOS layout by the `Total War WARHAMMER III.app` bundle and uses `TotalWarhammer3Data/data` for PackFiles and `TotalWarhammer3Data/data/localisation` for language files.

## Packaging Considerations

- macOS applications are typically distributed as `.app` bundles inside `.dmg` disk images.
- Qt dependencies should be bundled using `macdeployqt6` or `macdeployqt`; the script calls either tool when available.
- The Qt-only build disables KDE's shortcut editor and uses `QPlainTextEdit` instead of KTextEditor, so syntax highlighting/editor preferences are reduced compared with Linux/Windows.
- Code signing and notarization are required for distribution outside the App Store.
- TODO: add Developer ID signing and Apple notarization for release artifacts.
- TODO: verify the build on Unix-like hosts, including macOS arm64/x64 and Linux.
- TODO: add CI or a release checklist that confirms `RPFM.app` launches, opens `.pack` files from Finder, and keeps `.pack` document associations.
- No CI runner is currently configured for macOS builds.
