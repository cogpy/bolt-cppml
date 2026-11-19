#!/usr/bin/env bash
# Smoke test for a built tree: checks that the bolt application and the demo
# programs actually start, respond and exit, rather than only that they link.
#
# Usage: scripts/smoke_test.sh [build-dir]      (default: build)
#
# A program fails the smoke test if it hangs (timeout) or dies from a signal.
# A demo that exits with an ordinary non-zero status is reported but not
# treated as a failure, because several demos need tools that may not be
# installed (clangd, a model file). GUI programs are started under xvfb when
# there is no display and must stay up for a few seconds.
#
# Set BOLT_SMOKE_MODEL=/path/to/model.gguf to also load a real model and
# generate text with it.

set -u

BUILD_DIR="${1:-build}"
TIMEOUT_SECS="${SMOKE_TIMEOUT:-60}"

if [ ! -x "$BUILD_DIR/bolt" ]; then
    echo "error: $BUILD_DIR/bolt not found or not executable" >&2
    exit 2
fi

BUILD_DIR="$(cd "$BUILD_DIR" && pwd)"

failures=0
pass() { echo "  ok    $1"; }
fail() { echo "  FAIL  $1"; failures=$((failures + 1)); }

echo "== shared libraries =="

# The programs must find their own libraries from the build tree without any
# help from the environment, so this check runs before LD_LIBRARY_PATH is set.
if out="$(env -u LD_LIBRARY_PATH timeout "$TIMEOUT_SECS" "$BUILD_DIR/bolt" --version 2>&1)"; then
    pass "bolt starts without LD_LIBRARY_PATH"
else
    fail "bolt does not start without LD_LIBRARY_PATH: $out"
fi

export LD_LIBRARY_PATH="$BUILD_DIR:$BUILD_DIR/bin:${LD_LIBRARY_PATH:-}"

echo "== bolt command line =="

out="$(timeout "$TIMEOUT_SECS" "$BUILD_DIR/bolt" --version 2>&1)"; rc=$?
if [ "$rc" -eq 0 ] && printf '%s' "$out" | grep -q '^bolt '; then
    pass "--version ($out)"
else
    fail "--version (exit $rc): $out"
fi

out="$(timeout "$TIMEOUT_SECS" "$BUILD_DIR/bolt" --help 2>&1)"; rc=$?
if [ "$rc" -eq 0 ] && printf '%s' "$out" | grep -q 'Usage:'; then
    pass "--help"
else
    fail "--help (exit $rc)"
fi

timeout "$TIMEOUT_SECS" "$BUILD_DIR/bolt" --no-such-option >/dev/null 2>&1; rc=$?
if [ "$rc" -eq 2 ]; then
    pass "unknown option is rejected"
else
    fail "unknown option should exit 2, got $rc"
fi

echo "== bolt interactive session =="

out="$(printf 'hello smoke test\n/help\n/exit\n' | timeout "$TIMEOUT_SECS" "$BUILD_DIR/bolt" 2>&1)"; rc=$?
if [ "$rc" -eq 0 ] && printf '%s' "$out" | grep -q 'Echo: hello smoke test' \
        && printf '%s' "$out" | grep -q 'Available commands'; then
    pass "scripted session answers and exits on /exit"
else
    fail "scripted session (exit $rc)"
fi

timeout "$TIMEOUT_SECS" "$BUILD_DIR/bolt" </dev/null >/dev/null 2>&1; rc=$?
if [ "$rc" -eq 0 ]; then
    pass "exits cleanly at end of input"
else
    fail "end of input (exit $rc; 124 means it hung)"
fi

echo "== demo programs =="

# Programs that open a window. They are run under a virtual display when no
# real one is available, and are expected to keep running until stopped.
gui_programs="demo_lsp_ui_integration demo_widget_framework gui_main"
GUI_SECS="${SMOKE_GUI_SECS:-10}"

for prog in "$BUILD_DIR"/demo_* "$BUILD_DIR"/bolt_terminal_ai "$BUILD_DIR"/benchmark_tool "$BUILD_DIR"/gui_main; do
    if [ ! -f "$prog" ] || [ ! -x "$prog" ]; then
        continue
    fi
    name="$(basename "$prog")"

    case " $gui_programs " in
        *" $name "*)
            if [ -n "${DISPLAY:-}" ] || [ -n "${WAYLAND_DISPLAY:-}" ]; then
                timeout "$GUI_SECS" "$prog" </dev/null >/dev/null 2>&1; rc=$?
            elif command -v xvfb-run >/dev/null 2>&1; then
                LIBGL_ALWAYS_SOFTWARE=1 xvfb-run -a timeout "$GUI_SECS" "$prog" </dev/null >/dev/null 2>&1; rc=$?
            else
                echo "  skip  $name (needs a display; install xvfb to test it headlessly)"
                continue
            fi
            # 124 means it was still running when the time was up, which is
            # what a healthy GUI does; 0 means it closed by itself.
            if [ "$rc" -eq 124 ] || [ "$rc" -eq 0 ]; then
                pass "$name (GUI started and stayed up for ${GUI_SECS}s)"
            else
                fail "$name GUI exited with status $rc"
            fi
            continue
            ;;
    esac

    timeout "$TIMEOUT_SECS" "$prog" </dev/null >/dev/null 2>&1; rc=$?
    if [ "$rc" -eq 0 ]; then
        pass "$name"
    elif [ "$rc" -eq 124 ]; then
        fail "$name hung (no exit within ${TIMEOUT_SECS}s)"
    elif [ "$rc" -ge 126 ]; then
        fail "$name crashed or could not start (exit $rc)"
    else
        echo "  note  $name exited $rc (usually a missing optional tool)"
    fi
done

# Optional: real inference with a GGUF model. Set BOLT_SMOKE_MODEL to a model
# file; the program looks for models in ./models, so it is run from a scratch
# directory that has the model linked there.
if [ -n "${BOLT_SMOKE_MODEL:-}" ]; then
    echo "== model inference =="
    if [ ! -f "$BOLT_SMOKE_MODEL" ]; then
        fail "BOLT_SMOKE_MODEL=$BOLT_SMOKE_MODEL does not exist"
    elif [ ! -x "$BUILD_DIR/demo_direct_gguf" ]; then
        fail "demo_direct_gguf is not built, cannot test model inference"
    else
        model_path="$(cd "$(dirname "$BOLT_SMOKE_MODEL")" && pwd)/$(basename "$BOLT_SMOKE_MODEL")"
        work_dir="$(mktemp -d)"
        mkdir -p "$work_dir/models"
        ln -s "$model_path" "$work_dir/models/$(basename "$model_path")"
        out="$(cd "$work_dir" && timeout "${SMOKE_MODEL_TIMEOUT:-300}" "$BUILD_DIR/demo_direct_gguf" </dev/null 2>&1)"; rc=$?
        rm -rf "$work_dir"
        if [ "$rc" -ne 0 ]; then
            fail "model inference exited $rc"
        elif ! printf '%s' "$out" | grep -q 'Model loaded successfully'; then
            fail "model was not loaded"
        elif ! printf '%s' "$out" | grep -q 'Current provider: direct_gguf'; then
            fail "AI manager did not switch to the loaded model"
        elif printf '%s' "$out" | grep -q 'Failed: [1-9]'; then
            fail "model inference reported failed requests"
        else
            pass "model loads and generates text ($(basename "$model_path"))"
        fi
    fi
fi

echo
if [ "$failures" -eq 0 ]; then
    echo "Smoke test passed."
    exit 0
fi
echo "Smoke test failed: $failures problem(s)."
exit 1
