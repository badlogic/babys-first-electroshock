#!/bin/bash
set -e

# Check if two arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <input_dir> <output_dir>"
    exit 1
fi

input_dir=$1
output_dir=$2
mkdir -p "$output_dir"

# Convert all .wav files in the input directory to 22kHz, mono, and QOA format, then copy to output dir
for file in "$input_dir"/*.wav; do
    [ -e "$file" ] || continue
    temp_wav="$output_dir/$(basename "$file")"
    ffmpeg -y -i "$file" -ar 22050 -ac 1 "$temp_wav"  # Resample to 22kHz and convert to mono
    ./qoaconv "$temp_wav" "$output_dir/$(basename "${file%.wav}.qoa")"
    rm "$temp_wav"  # Remove intermediate WAV file after conversion
done