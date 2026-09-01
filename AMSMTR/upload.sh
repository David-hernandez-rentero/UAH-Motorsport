#!/usr/bin/env bash
cd "$(dirname "$0")"
set -euxo pipefail

./build.sh "$@"
if [[ " $* " == *" --debug "* ]]; then
    TARGET=Debug
else
    TARGET=Release
fi

STM32_Programmer_CLI -c port=SWD -e all -w "G474Re-BMS/build/$TARGET/G474Re-BMS.elf" -v -rst -g
STM32_Programmer_CLI --list | awk '
/UART Interface/ {uart=1}
/=====/ && uart && !/UART Interface/ {uart=0}
uart && /NUCLEO-G474RE/ {found=1}
uart && found && /Port:/ {print; exit}
'
