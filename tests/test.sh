#!/bin/bash

echo "Hindi PL/0 compiler test suite"
echo "========================"

mkdir -p output

for i in *.hindi; do
  /usr/bin/printf "%.4s... " "$i"  
  output_file="output/${i%.hindi}.c"
  ./../hindipl0c "$i" > "$output_file" 2>&1
  
  if [ $? -eq 0 ]; then
    echo "ok"
  else
    echo "fail"
  fi
done
