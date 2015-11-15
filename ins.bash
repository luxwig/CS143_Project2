#!/bin/bash

for i in $(seq 0 $1); do
  if [[ $((i%5)) != 0  || $((i%20)) == 15 ]]; then
    echo $i
  fi
done
  echo -1
