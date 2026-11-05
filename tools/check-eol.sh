#!/usr/bin/env bash
# Fail on CRLF in source/text files. Usage: bash tools/check-eol.sh [root]
# Skips backups/, build/, review/, .git/, debug-evidence/ and *.log.
set -u

root="${1:-.}"
bad=0

while IFS= read -r -d '' f; do
    if grep -qU $'\r' "$f" 2>/dev/null; then
        printf 'CRLF: %s\n' "$f"
        bad=1
    fi
done < <(find "$root" \
    \( -path "$root/.git" -o -path "$root/backups" -o -path "$root/build" \
       -o -path "$root/review" -o -path "$root/debug-evidence" \
       -o -name 'debug-evidence*' -o -name '*.log' \) -prune -o \
    -type f \( -name '*.sh' -o -name '*.prop' -o -name '*.xml' -o -name '*.md' \
        -o -name '*.cpp' -o -name '*.c' -o -name '*.h' -o -name '*.py' \
        -o -name '*.txt' -o -name '*.json' \) -print0)

if [ "$bad" -ne 0 ]; then
    printf '\nFix with: sed -i "s/\\r$//" <file>\n'
    exit 1
fi
printf 'OK: no CRLF in text files\n'
