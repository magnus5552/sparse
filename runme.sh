#!/bin/bash

mkdir -p test

gcc -o test/myprogram sparse.c

pushd test

FILE_SIZE=$((4 * 1024 * 1024 + 1))
OUTPUT_FILE="A"

truncate -s $FILE_SIZE A
dd if=A of=A conv=notrunc

echo '1' | dd of="$OUTPUT_FILE" bs=1 seek=0 count=1 conv=notrunc

echo '1' | dd of="$OUTPUT_FILE" bs=1 seek=10000 count=1 conv=notrunc

echo '1' | dd of="$OUTPUT_FILE" bs=1 seek=$((FILE_SIZE - 1)) count=1 conv=notrunc

./myprogram A B

gzip -kf A
gzip -kf B

gzip -cd B.gz | ./myprogram C

BLOCK_SIZE=100 ./myprogram A D

stat A
stat A.gz
stat B
stat B.gz
stat C
stat D

for file in A A.gz B B.gz C D; do
    if [ -f "$file" ]; then
        printf "%-8s %s\n" "$file:" "$(stat --format="size=%s blocks=%b" "$file")"
    else
        echo "$file: not found"
    fi
done

popd