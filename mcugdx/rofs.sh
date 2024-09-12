#!/bin/bash

get_file_size() {
    FILE="$1"
    if [[ "$OSTYPE" == "linux-gnu"* ]]; then
        stat -c%s "$FILE" # Linux
    elif [[ "$OSTYPE" == "darwin"* ]]; then
        stat -f%z "$FILE" # macOS
    else
        echo "Unsupported OS: $OSTYPE" >&2
        exit 1
    fi
}

OUTPUT_FILE="$1"
INPUT_DIR="$2"

# Clear the existing file
> "$OUTPUT_FILE"

# Find all files recursively in the input directory
DATA_FILES=$(find "$INPUT_DIR" -type f)

# Initialize variables
NUM_FILES=0
HEADER=""
CONTENT_OFFSET=0

# Loop through each file and process them
for FILE in $DATA_FILES; do
    NUM_FILES=$((NUM_FILES + 1))

    # Get the relative filename (relative to the input directory)
    RELATIVE_FILENAME="${FILE#$INPUT_DIR/}"

    # Calculate file content size and update the offset
    FILE_SIZE=$(get_file_size "$FILE")

    # Append the relative filename, current content offset, and file size to the header
    HEADER+="$RELATIVE_FILENAME\n"
    HEADER+="$CONTENT_OFFSET\n"
    HEADER+="$FILE_SIZE\n"

    # Update the content offset (for the next file)
    CONTENT_OFFSET=$((CONTENT_OFFSET + FILE_SIZE))
done

# Write the number of files at the beginning of the output file
printf "%s\n" "$NUM_FILES" > "$OUTPUT_FILE"

# Append the header with relative filenames, offsets, and file sizes
printf "%b" "$HEADER" >> "$OUTPUT_FILE"

# Append the actual contents of each file after the header
for FILE in $DATA_FILES; do
    cat "$FILE" >> "$OUTPUT_FILE"
done