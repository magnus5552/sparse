#!/bin/bash

mkdir test

gcc -o test/myprogram sparce.c

pushd test

FILE_SIZE=$((4 * 1024 * 1024 + 1))
OUTPUT_FILE="fileA"

truncate -s $FILE_SIZE fileA

echo '1' | dd of="$OUTPUT_FILE" bs=1 seek=0 count=1 conv=notrunc

echo '1' | dd of="$OUTPUT_FILE" bs=1 seek=10000 count=1 conv=notrunc

echo '1' | dd of="$OUTPUT_FILE" bs=1 seek=$((FILE_SIZE - 1)) count=1 conv=notrunc

./myprogram fileA fileB

gzip -k fileA
gzip -k fileB

gzip -cd fileB.gz | ./myprogram fileC

BLOCK_SIZE=100 ./myprogram fileA fileD

stat fileA
stat fileA.gz
stat fileB
stat fileB.gz
stat fileC
stat fileD

echo -e "File\tApparent\tDisk blocks"
ls -l --block-size=1 fileA fileA.gz fileB fileB.gz fileC fileD | awk '{print $9 "\t" $5 "\t" $5}'
du -h fileA fileA.gz fileB fileB.gz fileC fileD

popd