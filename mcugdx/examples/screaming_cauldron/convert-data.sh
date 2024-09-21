#!/bin/bash

mkdir -p data

for file in data-raw/*.mp3; do
  filename=$(basename "$file" .mp3)
  ./qoaconv "$file" "data/${filename}.qoa"
done
