#!/bin/bash

echo Hindi PL/0 compiler test suite
echo ========================

for i in *.hindi ; do
  /usr/bin/printf "%.4s... " $i
  ../hidni\ pl0 $i > /dev/null 2>&1
  if [ $? -eq 0 ] ; then
    echo ok
  else
    echo fail
  fi
done
