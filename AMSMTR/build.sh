#!/usr/bin/env bash
cd "$(dirname "$0")"
set -euxo pipefail

cd G474Re-BMS/

if [[ " $* " == *" --clean "* || " $* " == *" --clear "* ]]; then
    rm -rf build/
    exit
fi

function build() {
    PRESET=$1

    if [[ ! -d build/$PRESET ]]; then
        cmake --preset "$PRESET" || (
            ERR=$?
            # Remove entire build/, not only build/$PRESET
            rm -rf build/
            exit $ERR
        )
    fi

    cmake --build -j8 --preset "$PRESET"
}

build Debug
build Release

if command -v clang-format; then
    find Core/ -type f -exec clang-format -i {} +
fi
if command -v clang-tidy; then
    find Core/Src/{main.c,bms/} -type f -exec clang-tidy -quiet -p build/Debug/ {} +
fi
