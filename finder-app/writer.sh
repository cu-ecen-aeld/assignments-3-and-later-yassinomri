#!/bin/bash

# Check if the number of arguments is not equal to 2
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <file_path> <text_string>"
    exit 1
fi

# Get the file path and text string from command line arguments
writefile="$1"
writestr="$2"

# Create the directory path if it doesn't exist
mkdir -p "$(dirname "$writefile")"

# Write the text string to the file, overwriting any existing file
echo "$writestr" > "$writefile"

# Check if the file was created successfully
if [ $? -ne 0 ]; then
    echo "Error: Could not create file $writefile"
    exit 1
fi

echo "File $writefile created successfully with content:"
echo "$writestr"

