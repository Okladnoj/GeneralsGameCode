#!/bin/sh
#
# Compares the macOS dump against the two win32 dumps and writes math-diff.txt.
#
# Run it from the directory holding the dumps:
#   sh compare_math.sh
#
# Windows writes CRLF, so line endings are normalised before comparing.

set -e

MAC=math-mac.txt
P24=math-win-PC24.txt
P53=math-win-PC53.txt
OUT=math-diff.txt

for f in "$MAC" "$P24" "$P53"; do
    if [ ! -f "$f" ]; then
        echo "missing $f" >&2
        exit 1
    fi
done

norm() {
    tr -d '\r' < "$1" | grep -v '^================'
}

pair_up() {
    # $1 mac file, $2 win file, $3 label for the win column
    a=$(mktemp)
    b=$(mktemp)
    norm "$1" > "$a"
    norm "$2" > "$b"

    count=$(diff "$a" "$b" | grep -c '^<' || true)

    printf '================ macOS vs %s ================\n' "$3"
    printf 'differing lines: %s\n\n' "$count"

    if [ "$count" -gt 0 ]; then
        printf '%-20s %-46s %-18s %s\n' "row" "arguments" "macOS" "$3"
        printf '%-20s %-46s %-18s %s\n' \
            "--------------------" \
            "----------------------------------------------" \
            "------------------" "------------------"

        # Walk both files in step and print only the lines that differ.
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
    printf 'generated %s\n\n' "$(date -u '+%Y-%m-%d %H:%M UTC')"

    pair_up "$MAC" "$P24" "win32 _PC_24"
    pair_up "$MAC" "$P53" "win32 _PC_53"
} > "$OUT"

echo "wrote $OUT"
grep '^differing lines' "$OUT"
