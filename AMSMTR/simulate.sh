#!/usr/bin/env bash
cd "$(dirname "$0")"
set -euo pipefail

if [[ " $* " == *" --clean "* || " $* " == *" --clear "* ]]; then
    rm -f simulation/simulation.log*
fi

function download_repl() {
    URL=https://zephyr-dashboard.renode.io/zephyr_sim
    URL=$URL/$(curl -fsSL $URL/latest)
    URL=$URL/$(curl -fsSL $URL/latest)
    curl -fsSL $URL/nucleo_g474re/hello_world/hello_world.repl -o simulation/nucleo_g474re.repl
}
if [[ " $* " == *" --generate-patches "* ]]; then
    if [[ -f simulation/nucleo_g474re.repl ]]; then
        mv simulation/nucleo_g474re.repl simulation/latest.repl
    fi

    download_repl
    rm -f "simulation/patches/99_xxx.patch"

    function save() {
        (
            git diff --no-index simulation/prev.repl simulation/nucleo_g474re.repl \
                || true
        ) | \
        (
            echo "--- a/simulation/nucleo_g474re.repl"
            echo "+++ b/simulation/nucleo_g474re.repl"
            tail -n +5
        ) > "$1"
    }
    find simulation/patches/ -name "*.patch" | sort | xargs -L1 /usr/bin/env bash -c '
        eval "$0"
        echo -n "$1 ... "
        cp simulation/nucleo_g474re.repl simulation/prev.repl
        git apply "$1" || exit 255
        save "$1"
        echo done
    ' "$(declare -f save)"
    if [[ -f simulation/latest.repl ]]; then
        mv simulation/nucleo_g474re.repl simulation/prev.repl
        mv simulation/latest.repl simulation/nucleo_g474re.repl
        save simulation/patches/99_xxx.patch
        if (( "$(wc -l < simulation/patches/99_xxx.patch)" == 2 )); then
            rm simulation/patches/99_xxx.patch
        fi
    fi
    rm -f simulation/prev.repl
    exit
fi
if [[ " $* " == *" --nuke "* || ! -d simulation/renode/ ]]; then
    case "$(uname -s)" in
        Linux*)
            RENODE_URL=https://builds.renode.io/renode-latest.linux-portable.tar.gz
            EXTRACT_CMD="tar -xz -C simulation/renode/ --strip-components=1"
            ;;
        MINGW*|CYGWIN*)
            RENODE_URL=https://builds.renode.io/renode-latest.windows-portable.zip
            EXTRACT_CMD="
                cat > simulation/renode.zip
                unzip simulation/renode.zip -d simulation/ >/dev/null
                sleep 1 # without this sleep, it just works some times; sync doesnt do anything, idk anymore
                mv -T simulation/renode_*/ simulation/renode/
                rm simulation/renode.zip
            "
            ;;
        *)
            echo >&2 "Unknown OS $(uname -s)"
            exit 1
            ;;
    esac
    rm -rf simulation/renode/
    mkdir -p simulation/renode/
    curl -fsSL "$RENODE_URL" | /usr/bin/env bash -c "$EXTRACT_CMD"
fi
if [[ " $* " == *" --nuke "* || ! -f simulation/nucleo_g474re.repl ]]; then
    download_repl
    find simulation/patches/ -name "*.patch" | sort | xargs git apply || {
        ERR=$?
        rm simulation/nucleo_g474re.repl
        exit $ERR
    }
fi
if [[ " $* " == *" --nuke "* ]]; then
    ./build.sh >/dev/null
    exit
fi
./build.sh >/dev/null
cp G474Re-BMS/build/Debug/G474Re-BMS.elf simulation/
if [[ " $* " == *" --renode-stderr "* ]]; then
    RENODE_STOUT=/dev/stderr
else
    RENODE_STOUT=/dev/null
fi

function free_port() {
    while true; do
        local port=$((20000 + RANDOM % 30000))
        # parens bc the stderr comes from the `/dev/...` not from the `true`
        if ! (true >/dev/tcp/127.0.0.1/$port) 2>/dev/null; then
            echo $port
            return
        fi
    done
}
function wait_until_port_open() {
    local port=$1
    local -n out_fd=$2
    # parens bc the stderr comes from the `/dev/...` not from the `exec`
    # {} instead of () parens bc out_fd is generated now, so must be in same sh
    while ! { exec {out_fd}<>/dev/tcp/127.0.0.1/$port ; } 2>/dev/null; do
        sleep 0.1
    done
}
function cleanup() {
    if [[ -n "${STDOUT_PID:-}" ]]; then
        kill    $STDOUT_PID 2>/dev/null || true
        wait -f $STDOUT_PID 2>/dev/null || true
    fi
    if [[ -n "${RENODE_PID:-}" ]]; then
        kill    $RENODE_PID 2>/dev/null || true
        wait -f $RENODE_PID 2>/dev/null || true
    fi
}
trap cleanup EXIT

UART_PORT=$(free_port)

# * Log levels:
#    -1 = NOISY
#     0 = DEBUG
#     1 = INFO
#     2 = WARNING
#     3 = ERROR
# * The following command can be ran to log CAN traffic to Wireshark:
#    `emulation CreateCANHub "canHub"; connector Connect sysbus.fdcan1 canHub; emulation LogCANTraffic`
# * LogFunctionNames can be set to true for trace logging
exec {MONITOR}> >(
simulation/renode/renode --console --disable-gui \
    -e 'logLevel 1' \
    -e 'logFile $CWD/simulation/simulation.log' \
    -e 'include @simulation/nucleo_g474re.resc' \
    -e 'showAnalyzer sysbus.lpuart1' \
    -e 'sysbus.cpu0 LogFunctionNames false' \
    -e "emulation CreateServerSocketTerminal $UART_PORT \"my_terminal\" false" \
    -e 'connector Connect sysbus.lpuart1 my_terminal' >$RENODE_STOUT
)
RENODE_PID=$!

wait_until_port_open $UART_PORT UART

cat <&$UART &
STDOUT_PID=$!

echo start >&$MONITOR
cat >&$UART
