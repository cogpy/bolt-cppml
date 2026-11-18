#!/usr/bin/env bash
# Smoke test for a built tree: checks that the bolt application and the demo
# programs actually start, respond and exit, rather than only that they link.
#
# Usage: scripts/smoke_test.sh [build-dir]      (default: build)
#
# A program fails the smoke test if it hangs (timeout) or dies from a signal.
# A demo that exits with an ordinary non-zero status is reported but not
# treated as a failure, because several demos need tools that may not be
# installed (clangd, a model file, a display).

set -u

BUILD_DIR="${1:-build}"
TIMEOUT_SECS="${SMOKE_TIMEOUT:-60}"

if [ ! -x "$BUILD_DIR/bolt" ]; then
    echo "error: $BUILD_DIR/bolt not found or not executable" >&2
    exit 2
fi

BUILD_DIR="$(cd "$BUILD_DIR" && pwd)"
export LD_LIBRARY_PATH="$BUILD_DIR:$BUILD_DIR/bin:${LD_LIBRARY_PATH:-}"

failures=0
pass() { echo "  ok    $1"; }
fail() { echo "  FAIL  $1"; failures=$((failures + 1)); }

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

# Programs that open a window and cannot run on a machine without a display.
needs_display="demo_widget_framework gui_main"

for prog in "$BUILD_DIR"/demo_* "$BUILD_DIR"/bolt_terminal_ai "$BUILD_DIR"/benchmark_tool; do
    [ -f "$prog" ] && [ -x "$prog" ] || continue
    name="$(basename "$prog")"

    case " $needs_display " in
        *" $name "*)
            if [ -z "${DISPLAY:-}" ] && [ -z "${WAYLAND_DISPLAY:-}" ]; then
                echo "  skip  $name (needs a display)"
                continue
            fi
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

echo
if [ "$failures" -eq 0 ]; then
    echo "Smoke test passed."
    exit 0
fi
echo "Smoke test failed: $failures problem(s)."
exit 1
