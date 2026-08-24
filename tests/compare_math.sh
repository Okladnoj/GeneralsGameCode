#!/bin/sh
#
# Compares every math-*.txt dump against the macOS one and writes math-diff.txt.
#
# Run it from the directory holding the dumps:
#   sh compare_math.sh
#
# Windows writes CRLF, so line endings are normalised before comparing.

set -e

OUT=math-diff.txt

BASE=$(ls math-mac-*.txt 2>/dev/null | head -1)
if [ -z "$BASE" ]; then
    echo "no macOS dump found (math-mac-*.txt)" >&2
    exit 1
fi

OTHERS=$(ls math-*.txt 2>/dev/null | grep -v "^$BASE$" | grep -v "^$OUT$" || true)
if [ -z "$OTHERS" ]; then
    echo "nothing to compare against $BASE" >&2
    exit 1
fi

norm() {
    tr -d '\r' < "$1" | grep -v '^================'
}

# $1 other file
pair_up() {
    a=$(mktemp)
    b=$(mktemp)
    norm "$BASE" > "$a"
    norm "$1" > "$b"

    label=$(printf '%s' "$1" | sed -E 's/^math-//; s/\.txt$//')
    count=$(diff "$a" "$b" | grep -c '^<' || true)

    printf '================ %s vs %s ================\n' \
        "$(printf '%s' "$BASE" | sed -E 's/^math-//; s/\.txt$//')" "$label"
    printf 'differing lines: %s\n\n' "$count"

    if [ "$count" -gt 0 ]; then
        printf '%-20s %-46s %-18s %s\n' "row" "arguments" "macOS" "$label"
        printf '%-20s %-46s %-18s %s\n' \
            "--------------------" \
            "----------------------------------------------" \
            "------------------" "------------------"

        paste "$a" "$b" | while IFS="$(printf '\t')" read -r l r; do
            [ "$l" = "$r" ] && continue
            case "$l" in
                ----*) continue ;;
            esac
            row=$(printf '%s' "$l" | sed -E 's/ .*//')
            args=$(printf '%s' "$l" | sed -E 's/^[^ ]+ +//; s/ +[^ ]+$//')
            lv=$(printf '%s' "$l" | sed -E 's/.* //')
            rv=$(printf '%s' "$r" | sed -E 's/.* //')
            printf '%-20s %-46s %-18s %s\n' "$row" "$args" "$lv" "$rv"
        done
    fi

    printf '\n'
    rm -f "$a" "$b"
}

{
    printf 'GameMath cross-platform comparison\n'
    printf 'generated %s\n' "$(date -u '+%Y-%m-%d %H:%M UTC')"
    printf 'baseline %s\n\n' "$BASE"

    printf 'Each function is called on the same value several ways. The suffix on\n'
    printf 'the row name says which way, so a difference can be traced to the\n'
    printf 'function itself or to a conversion around it.\n\n'
    printf '  .d     double function, double result        gm_f(x)\n'
    printf '  .f     float function, float result          gm_ff((float)x)\n'
    printf '  .f2d   float function, result widened        (double)gm_ff((float)x)\n'
    printf '  .d2f   double function, result narrowed      (float)gm_f(x)\n\n'
    printf 'Two argument functions vary each argument on its own, since a value can\n'
    printf 'arrive as a full double or as one that already went through a float,\n'
    printf 'and each combination is recorded both ways round:\n\n'
    printf '  .dd     gm_f(x, y)\n'
    printf '  .dd2f   (float)gm_f(x, y)\n'
    printf '  .df     gm_f(x, (double)(float)y)\n'
    printf '  .df2f   (float)gm_f(x, (double)(float)y)\n'
    printf '  .fd     gm_f((double)(float)x, y)\n'
    printf '  .fd2f   (float)gm_f((double)(float)x, y)\n'
    printf '  .ff     gm_f((double)(float)x, (double)(float)y)\n'
    printf '  .ff2f   (float)gm_f((double)(float)x, (double)(float)y)\n\n'
    printf 'The .f2d row is what the WWMath wrappers do, so it is the one that\n'
    printf 'matters for the game.\n\n'

    for f in $OTHERS; do
        pair_up "$f"
    done
} > "$OUT"

echo "wrote $OUT"
grep -E '^(=====|differing lines)' "$OUT" | sed 's/^/  /'
