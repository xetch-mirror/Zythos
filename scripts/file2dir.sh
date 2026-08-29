#!/bin/bash
# Scans initrd/ and generates file_register_gen.c automatically.
# Every file under initrd/<dir>/<name> becomes a REGISTER_FILE entry
# targeting /<dir>/<name> — no manual editing required.

OUT="file_register_gen.c"
SRC_ROOT="initrd"

echo '#include "file_register.h"' > "$OUT"
echo "" >> "$OUT"

i=0
find "$SRC_ROOT" -type f | while read -r path; do
    rel="${path#$SRC_ROOT/}"          # e.g. sbin/zsh.elf
    dir="/$(dirname "$rel")"          # e.g. /sbin
    name="$(basename "$rel")"         # e.g. zsh.elf
    sym="blob_${i}"

    # Embed the raw bytes as a C array
    xxd -i "$path" | sed "s/unsigned char .*\[\]/static const uint8_t ${sym}_data[]/;s/unsigned int .*_len/static const uint32_t ${sym}_size/" >> "$OUT"

    echo "REGISTER_FILE(${sym}, \"$dir\", \"$name\", ${sym}_data, ${sym}_size);" >> "$OUT"
    echo "" >> "$OUT"

    i=$((i+1))
done