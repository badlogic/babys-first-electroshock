#!/bin/bash

mkdir -p data

for file in data-raw/*.wav; do
  filename=$(basename "$file" .wav)
  ./qoaconv "$file" "data/${filename}.qoa"
done
