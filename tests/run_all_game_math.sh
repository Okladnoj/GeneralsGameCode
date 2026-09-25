#!/bin/sh
#
# The whole GameMath refresh on macOS and other Unix in one command: the local
# dump and benchmark, then the comparison of every dump and every benchmark
# result present, the ones pushed from Windows included.
#
#   sh tests/run_all_game_math.sh [--pull] [runner options]
#
# --pull, when it comes first, runs git pull before anything else. Every other
# option (--rev, --source, --config, --intrinsics, ...) goes to both runners
# unchanged.
#
# Results land in tests/math and tests/bench, wherever this is started from.

set -e

TESTS_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
SCRIPTS=$TESTS_DIR/scripts

if [ "$1" = --pull ]; then
    shift
    echo '=== git pull ==='
    git -C "$TESTS_DIR/.." pull
    echo ''
fi

echo '=== verify ==='
sh "$SCRIPTS/run_verify_game_math.sh" "$@"

echo ''
echo '=== bench ==='
sh "$SCRIPTS/run_bench_game_math.sh" "$@"

echo ''
echo '=== compare dumps ==='
sh "$SCRIPTS/compare_math.sh"

echo ''
echo '=== weigh benchmarks ==='
sh "$SCRIPTS/weigh_bench.sh"

echo ''
echo 'done; dumps and their comparison are in tests/math, timings and their weighting in tests/bench'
