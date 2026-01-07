#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
ASSETS_DIR="${ROOT_DIR}/assets"
APPIMAGETOOL_BIN="${ASSETS_DIR}/appimagetool-x86_64.AppImage"
NONSTEAM_BIN="${ASSETS_DIR}/nonsteam"

BUILD_LINUX="${BUILD_LINUX:-${ROOT_DIR}/build-linux}"
BUILD_APPIMAGE="${BUILD_APPIMAGE:-${ROOT_DIR}/build-appimage}"
BUILD_WINDOWS="${BUILD_WINDOWS:-${ROOT_DIR}/build-windows}"

LINUX_CMAKE_ARGS="${LINUX_CMAKE_ARGS:-}"
APPIMAGE_CMAKE_ARGS="${APPIMAGE_CMAKE_ARGS:-}"
WINDOWS_CMAKE_ARGS="${WINDOWS_CMAKE_ARGS:-}"
BUILD_ARGS="${BUILD_ARGS:-}"
WINDOWS_MINGW_PREFIX="${WINDOWS_MINGW_PREFIX:-x86_64-w64-mingw32}"
WINDOWS_MINGW_BIN="${WINDOWS_MINGW_BIN:-/usr/${WINDOWS_MINGW_PREFIX}/bin}"
WINDOWS_EXTRA_DLL_DIRS="${WINDOWS_EXTRA_DLL_DIRS:-}"

echo "==> Building Linux (${BUILD_LINUX})"
cmake -S "${ROOT_DIR}" -B "${BUILD_LINUX}" ${LINUX_CMAKE_ARGS}
cmake --build "${BUILD_LINUX}" ${BUILD_ARGS}

echo "==> Building AppImage (${BUILD_APPIMAGE})"
cmake -S "${ROOT_DIR}" -B "${BUILD_APPIMAGE}" -DBUILD_APPIMAGE=ON ${APPIMAGE_CMAKE_ARGS} \
    -DAPPIMAGETOOL="${APPIMAGETOOL_BIN}" -DNONSTEAM_TOOL="${NONSTEAM_BIN}"
cmake --build "${BUILD_APPIMAGE}" ${BUILD_ARGS}

WINDOWS_TOOLCHAIN="${WINDOWS_TOOLCHAIN:-${ROOT_DIR}/toolchains/mingw64.cmake}"
echo "==> Building Windows (${BUILD_WINDOWS})"
cmake -S "${ROOT_DIR}" -B "${BUILD_WINDOWS}" -DCMAKE_TOOLCHAIN_FILE="${WINDOWS_TOOLCHAIN}" ${WINDOWS_CMAKE_ARGS}
cmake --build "${BUILD_WINDOWS}" ${BUILD_ARGS}

echo "==> Staging Windows runtime DLLs"
WINDOWS_EXE="${BUILD_WINDOWS}/GamepadCommander.exe"
if [[ ! -f "${WINDOWS_EXE}" ]]; then
    WINDOWS_EXE="$(find "${BUILD_WINDOWS}" -maxdepth 4 -type f -name "GamepadCommander.exe" | head -n 1 || true)"
fi

if [[ -z "${WINDOWS_EXE}" ]]; then
    echo "Warning: GamepadCommander.exe not found; skipping DLL staging."
    exit 0
fi

WINDOWS_EXE_DIR="$(dirname "${WINDOWS_EXE}")"
WINDOWS_EXE_DIR="${WINDOWS_EXE_DIR}" \
WINDOWS_MINGW_PREFIX="${WINDOWS_MINGW_PREFIX}" \
WINDOWS_MINGW_BIN="${WINDOWS_MINGW_BIN}" \
WINDOWS_EXTRA_DLL_DIRS="${WINDOWS_EXTRA_DLL_DIRS}" \
"${ROOT_DIR}/scripts/stage-windows-dlls.sh"
