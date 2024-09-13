#!/bin/bash

mkdir -p data

for file in data-raw/*.png; do
  filename=$(basename "$file" .png)
  ./qoiconv "$file" "data/${filename}.qoi"
done
