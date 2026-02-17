#!/bin/bash
LOG_FILE="full_build.log"
OUTPUT_FILE="filtered_64bit_warnings.log"

echo "=== 64-bit Pointer/Integer Casting Warnings ===" > $OUTPUT_FILE
grep -E "cast from pointer to integer of different size|cast to pointer from integer of different size|larger base type|makes integer from pointer without a cast" $LOG_FILE >> $OUTPUT_FILE

echo -e "\n=== Format String Ptr/Int Mismatch Warnings ===" >> $OUTPUT_FILE
grep -E "format specifies type '.*' but the argument has type '.*'" $LOG_FILE | grep -E "int|long|void \*|ptr" >> $OUTPUT_FILE

echo -e "\n=== Missing Implementations / Pure Virtual ===" >> $OUTPUT_FILE
grep -E "is abstract|not implemented|override" $LOG_FILE >> $OUTPUT_FILE

echo "Filtering complete. Results saved in $OUTPUT_FILE"
