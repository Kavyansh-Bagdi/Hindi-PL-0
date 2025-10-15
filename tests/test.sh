#!/bin/bash

echo "Hindi PL/0 compiler test suite"
echo "========================"

if [ "$1" != "-c" ] && [ "$1" != "-o" ]; then
    echo "Usage: $0 [-c|-o]"
    echo "  -c : Generate only .c files"
    echo "  -o : Generate and compile executables"
    exit 1
fi

mkdir -p output

for i in *.hindi; do
    /usr/bin/printf "%.4s... " "$i"
    base_name="${i%.hindi}"
    c_file="output/${base_name}.c"

    ./../hindipl0c "$i" > "$c_file" 2>&1
    if [ $? -ne 0 ]; then
        echo "fail"
        continue
    fi

    echo "ok"

    if [ "$1" == "-o" ]; then
        gcc "$c_file" -o "output/$base_name"
        if [ $? -eq 0 ]; then
            echo "Compiled: output/$base_name"
        else
            echo "Compilation failed for $c_file"
        fi
    fi
done
