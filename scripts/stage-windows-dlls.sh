#!/usr/bin/env bash
set -euo pipefail

WINDOWS_EXE_DIR="${WINDOWS_EXE_DIR:-}"
if [[ -z "${WINDOWS_EXE_DIR}" ]]; then
    echo "WINDOWS_EXE_DIR is required."
    exit 1
fi

WINDOWS_MINGW_PREFIX="${WINDOWS_MINGW_PREFIX:-x86_64-w64-mingw32}"
if [[ -z "${WINDOWS_MINGW_BIN:-}" ]]; then
    if [[ -d "/mingw64/bin" ]]; then
        WINDOWS_MINGW_BIN="/mingw64/bin"
    else
        WINDOWS_MINGW_BIN="/usr/${WINDOWS_MINGW_PREFIX}/bin"
    fi
fi

WINDOWS_EXTRA_DLL_DIRS="${WINDOWS_EXTRA_DLL_DIRS:-}"

WINDOWS_DLL_DIRS=()
if [[ -n "${WINDOWS_EXTRA_DLL_DIRS}" ]]; then
    IFS=':' read -r -a EXTRA_DIRS <<< "${WINDOWS_EXTRA_DLL_DIRS}"
    WINDOWS_DLL_DIRS+=("${EXTRA_DIRS[@]}")
fi
WINDOWS_DLL_DIRS+=("${WINDOWS_MINGW_BIN}")

copy_dll() {
    local name="$1"
    local src=""
    for dir in "${WINDOWS_DLL_DIRS[@]}"; do
        if [[ -f "${dir}/${name}" ]]; then
            src="${dir}/${name}"
            break
        fi
    done
    if [[ -n "${src}" ]]; then
        cp -f "${src}" "${WINDOWS_EXE_DIR}/"
        echo "Copied ${name} from ${src}"
        return 0
    fi
    echo "Warning: ${name} not found in DLL search paths."
    return 0
}

WINDOWS_DLLS=(
    "libgcc_s_seh-1.dll"
    "libstdc++-6.dll"
    "libwinpthread-1.dll"
    "SDL2.dll"
    "libcurl-4.dll"
    "libbrotlidec.dll"
    "libbrotlicommon.dll"
    "libbrotlienc.dll"
    "libcrypto-3-x64.dll"
    "libidn2-0.dll"
    "libnghttp2-14.dll"
    "libpsl-5.dll"
    "libssh2.dll"
    "libssl-3-x64.dll"
    "libssp-0.dll"
    "zlib1.dll"
    "libzstd.dll"
    "libiconv-2.dll"
    "libunistring-5.dll"
)

for dll in "${WINDOWS_DLLS[@]}"; do
    copy_dll "${dll}"
done
