#!/bin/bash
INPUT_FILE=$1   

if [ -z "$INPUT_FILE" ]; then
    echo "Usage: $0 <input-file>"
    exit 1
fi

./hindipl0c "$INPUT_FILE" > code.c
if gcc code.c -o output; then
    ./output
else
    echo "Compilation failed!"
fi

rm code.c output