#!/bin/bash
set -e

# Check if two arguments are provided
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <input_dir> <output_file>"
    exit 1
fi

input_dir=$1
output_file=$2
temp_dir=$(mktemp -d)

# Convert all .wav files in the input directory to 22kHz and then to QOA
for file in "$input_dir"/*.wav; do
    [ -e "$file" ] || continue
    temp_wav="$temp_dir/$(basename "$file")"
    ffmpeg -y -i "$file" -ar 22050 "$temp_wav"  # Resample to 22kHz
    mv "$temp_wav" "$file"
    ./qoaconv "$file" "$temp_dir/$(basename "${file%.wav}.qoa")"
done

# Copy index.html to the temporary directory
cp data/index.html "$temp_dir"

# Run rofs.sh with the output file and temp directory
../../rofs.sh "$output_file" "$temp_dir"

# Delete the temporary directory
rm -rf "$temp_dir"
