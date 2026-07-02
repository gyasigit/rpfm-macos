#!/usr/bin/env bash
#
# macOS build script for RPFM.
#
# Builds the UI and server, then lays them out in a minimal .app bundle.
# Run from anywhere:
#   ./install/macos/build_macos.sh

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
APP_NAME="RPFM"
APP_BUNDLE="$REPO_ROOT/target/macos/${APP_NAME}.app"
CONTENTS_DIR="$APP_BUNDLE/Contents"
MACOS_DIR="$CONTENTS_DIR/MacOS"
RESOURCES_DIR="$CONTENTS_DIR/Resources"
FRAMEWORKS_DIR="$CONTENTS_DIR/Frameworks"

cd "$REPO_ROOT"

find_qmake6() {
    if [ -n "${QMAKE6:-}" ] && [ -x "$QMAKE6" ]; then
        printf '%s\n' "$QMAKE6"
        return 0
    fi

    if [ -n "${QMAKE:-}" ] && [ -x "$QMAKE" ]; then
        printf '%s\n' "$QMAKE"
        return 0
    fi

    for candidate in \
        /opt/homebrew/opt/qt@6/bin/qmake6 \
        /usr/local/opt/qt@6/bin/qmake6 \
        /opt/homebrew/bin/qmake6 \
        /usr/local/bin/qmake6
    do
        if [ -x "$candidate" ]; then
            printf '%s\n' "$candidate"
            return 0
        fi
    done

    if command -v qmake6 >/dev/null 2>&1; then
        command -v qmake6
        return 0
    fi

    return 1
}

require_command() {
    local command_name="$1"
    local install_hint="$2"

    if ! command -v "$command_name" >/dev/null 2>&1; then
        echo "Error: missing ${command_name}. ${install_hint}" >&2
        exit 1
    fi
}

copy_ui_templates() {
    rm -rf "$RESOURCES_DIR/ui"
    mkdir -p "$RESOURCES_DIR/ui"

    if [ "${RPFM_NO_KDE:-}" = "1" ]; then
        for template in rpfm_ui/ui_templates/*.ui; do
            perl -pe '
                s/KLineEdit/QLineEdit/g;
                s/KComboBox/QComboBox/g;
                s/KTextEdit/QTextEdit/g;
                s/KMessageWidget/QLabel/g;
                s/KColorCombo/QComboBox/g;
                s/KCollapsibleGroupBox/QGroupBox/g;
            ' "$template" > "$RESOURCES_DIR/ui/$(basename "$template")"
        done
    else
        cp -R rpfm_ui/ui_templates/* "$RESOURCES_DIR/ui/"
    fi
}

deploy_with_macdeployqt() {
    local deploy_tool="$1"
    local deploy_log="$REPO_ROOT/target/macos/macdeployqt.log"

    if "$deploy_tool" "$APP_BUNDLE" -always-overwrite -libpath="$QT_LIB_DIR" -libpath="$FRAMEWORKS_DIR" -executable="$MACOS_DIR/rpfm_server" -codesign=- >"$deploy_log" 2>&1; then
        if grep -q '^ERROR:' "$deploy_log"; then
            echo "Warning: macdeployqt reported resolver warnings. See: $deploy_log" >&2
        fi
    else
        cat "$deploy_log" >&2
        exit 1
    fi
}

patch_ritual_macos_bindings() {
    local qt_gui_c_lib="$REPO_ROOT/3rdparty/src/ritual/output/out/qt_gui/c_lib"

    if [ ! -d "$qt_gui_c_lib" ]; then
        echo "Error: Ritual Qt bindings are missing. Run:" >&2
        echo "  git submodule update --init --recursive 3rdparty/src/ritual" >&2
        exit 1
    fi

    # The vendored Ritual QtGui bindings were generated from a Linux target. Guard
    # Linux-only EGL/Wayland native interfaces on macOS and use GLint64 casts for
    # OpenGL 64-bit integer functions, where long is not the same width as GLint64.
    perl -0pi -e 's/#ifndef _WIN32\nRITUAL_EXPORT QOpenGLContext \* ctr_qt_gui_ffi_QNativeInterface_QEGLContext_fromNative/#if !defined(_WIN32) \&\& !defined(__APPLE__)\nRITUAL_EXPORT QOpenGLContext * ctr_qt_gui_ffi_QNativeInterface_QEGLContext_fromNative/g' \
        "$qt_gui_c_lib/file3.cpp"
    perl -0pi -e 's/#ifdef _WIN32\nRITUAL_EXPORT void ctr_qt_gui_ffi_QOpenGLExtraFunctions_glGetInteger64v/#if defined(_WIN32) || defined(__APPLE__)\nRITUAL_EXPORT void ctr_qt_gui_ffi_QOpenGLExtraFunctions_glGetInteger64v/g' \
        "$qt_gui_c_lib/file3.cpp"
    perl -0pi -e 's/#ifndef _WIN32\nRITUAL_EXPORT QOpenGLContext \* ctr_qt_gui_ffi_QNativeInterface_QEGLContext_fromNative1/#if !defined(_WIN32) \&\& !defined(__APPLE__)\nRITUAL_EXPORT QOpenGLContext * ctr_qt_gui_ffi_QNativeInterface_QEGLContext_fromNative1/g' \
        "$qt_gui_c_lib/file8.cpp"
    perl -0pi -e 's/#ifndef _WIN32\nRITUAL_EXPORT unsigned int ctr_qt_gui_ffi_QNativeInterface_QWaylandApplication_lastInputSerial/#if !defined(_WIN32) \&\& !defined(__APPLE__)\nRITUAL_EXPORT unsigned int ctr_qt_gui_ffi_QNativeInterface_QWaylandApplication_lastInputSerial/g' \
        "$qt_gui_c_lib/file1.cpp"
}

if command -v brew >/dev/null 2>&1; then
    BREW_PREFIX="$(brew --prefix)"
    QT_PREFIX="$(brew --prefix qt@6 2>/dev/null || true)"

    export PATH="$BREW_PREFIX/bin:$BREW_PREFIX/opt/make/libexec/gnubin:${QT_PREFIX:+$QT_PREFIX/bin:}$PATH"
    export CMAKE_PREFIX_PATH="${QT_PREFIX:+$QT_PREFIX:}${CMAKE_PREFIX_PATH:-}"
    export PKG_CONFIG_PATH="${QT_PREFIX:+$QT_PREFIX/lib/pkgconfig:}${PKG_CONFIG_PATH:-}"
else
    echo "Warning: Homebrew was not found. Continuing with the current PATH." >&2
fi

require_command cargo "Install Rust from https://rustup.rs/."
require_command rustc "Install Rust from https://rustup.rs/."
require_command cmake "Install it with: brew install cmake"
require_command pkg-config "Install it with: brew install pkg-config"

if ! QMAKE6_PATH="$(find_qmake6)"; then
    echo "Error: qmake6 was not found. Install Qt with: brew install qt@6" >&2
    exit 1
fi
export QMAKE6="$QMAKE6_PATH"
QT_LIB_DIR="$(cd "$(dirname "$QMAKE6")/../lib" && pwd)"
export RPFM_NO_KDE="${RPFM_NO_KDE:-1}"

if ! command -v gmake >/dev/null 2>&1; then
    echo "Error: GNU Make was not found. Install it with: brew install make" >&2
    exit 1
fi
export GMAKE="$(command -v gmake)"

echo "Using qmake6: $QMAKE6"
echo "Using GNU Make: $GMAKE"
echo "Using Qt lib dir: $QT_LIB_DIR"
if [ "$RPFM_NO_KDE" = "1" ]; then
    echo "Using Qt-only macOS build: KDE Frameworks are disabled."
fi

if [ -f "$REPO_ROOT/.env" ]; then
    set -a
    . "$REPO_ROOT/.env"
    set +a
fi

if [ -f "3rdparty/src/qt_rpfm_extensions/Makefile" ]; then
    "$GMAKE" -C 3rdparty/src/qt_rpfm_extensions clean || true
fi

patch_ritual_macos_bindings

cargo build --release --bin rpfm_server --bin rpfm_ui

rm -rf "$APP_BUNDLE"
mkdir -p "$MACOS_DIR" "$RESOURCES_DIR/icons" "$RESOURCES_DIR/locale" "$RESOURCES_DIR/ui" "$FRAMEWORKS_DIR"

cp target/release/rpfm_ui "$MACOS_DIR/$APP_NAME"
cp target/release/rpfm_server "$MACOS_DIR/rpfm_server"
cp -R icons/* "$RESOURCES_DIR/icons/"
cp -R locale/* "$RESOURCES_DIR/locale/"
copy_ui_templates
cp LICENSE "$RESOURCES_DIR/LICENSE"

cat > "$CONTENTS_DIR/Info.plist" <<PLIST
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "https://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
    <key>CFBundleExecutable</key>
    <string>${APP_NAME}</string>
    <key>CFBundleIdentifier</key>
    <string>com.frodo45127.rpfm</string>
    <key>CFBundleName</key>
    <string>Rusted PackFile Manager</string>
    <key>CFBundleDisplayName</key>
    <string>RPFM</string>
    <key>CFBundlePackageType</key>
    <string>APPL</string>
    <key>CFBundleShortVersionString</key>
    <string>$(grep '^version = ' Cargo.toml | head -1 | cut -d'"' -f2)</string>
    <key>CFBundleVersion</key>
    <string>$(grep '^version = ' Cargo.toml | head -1 | cut -d'"' -f2)</string>
    <key>CFBundleDocumentTypes</key>
    <array>
        <dict>
            <key>CFBundleTypeExtensions</key>
            <array>
                <string>pack</string>
            </array>
            <key>CFBundleTypeName</key>
            <string>Total War Pack File</string>
            <key>CFBundleTypeRole</key>
            <string>Editor</string>
            <key>LSHandlerRank</key>
            <string>Owner</string>
            <key>LSItemContentTypes</key>
            <array>
                <string>com.frodo45127.rpfm.pack</string>
            </array>
        </dict>
    </array>
    <key>UTExportedTypeDeclarations</key>
    <array>
        <dict>
            <key>UTTypeIdentifier</key>
            <string>com.frodo45127.rpfm.pack</string>
            <key>UTTypeDescription</key>
            <string>Total War Pack File</string>
            <key>UTTypeConformsTo</key>
            <array>
                <string>public.data</string>
            </array>
            <key>UTTypeTagSpecification</key>
            <dict>
                <key>public.filename-extension</key>
                <array>
                    <string>pack</string>
                </array>
            </dict>
        </dict>
    </array>
    <key>NSHighResolutionCapable</key>
    <true/>
</dict>
</plist>
PLIST

if command -v macdeployqt6 >/dev/null 2>&1; then
    deploy_with_macdeployqt macdeployqt6
elif command -v macdeployqt >/dev/null 2>&1; then
    deploy_with_macdeployqt macdeployqt
else
    echo "Warning: macdeployqt was not found. The .app bundle may require Qt libraries from your build environment." >&2
fi

if command -v codesign >/dev/null 2>&1; then
    codesign --force --deep --sign - "$APP_BUNDLE"
    codesign --verify --deep --strict "$APP_BUNDLE"
fi

echo "Done. App bundle created at: $APP_BUNDLE"
