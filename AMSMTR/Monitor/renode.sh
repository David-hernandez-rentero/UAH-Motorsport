#!/usr/bin/env bash
cd "$(dirname "$0")"
set -euxo pipefail

../simulate.sh --clean | tee >(LC_ALL=C tr -c '\11\12\40-\176' '�' >/dev/tty) | cargo run --release
