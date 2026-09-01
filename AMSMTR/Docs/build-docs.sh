#!/usr/bin/env bash
cd "$(dirname "$0")"
set -euxo pipefail

LUALATEX='lualatex "$1" </dev/null'
INKSCAPE='inkscape "${1%.tex}.pdf" -b white -n 1 -o "${1%.tex}.svg"'
FIX_BACKGROUND='sed -z -i "s@<svg[^>]*>@\\0<rect width=\"100%\" height=\"100%\" fill=\"white\"/>@" "${1%.tex}.svg"'
find . -name "*.tex" -exec sh -c \
    "$LUALATEX && $INKSCAPE && $FIX_BACKGROUND" _ {} \;
