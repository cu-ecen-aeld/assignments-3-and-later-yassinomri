#!/bin/bash

# Check if the number of arguments is not equal to 2
if [ "$#" -ne 2 ]; then
    echo "Usage: $0 <directory_path> <search_string>"
    exit 1
fi

# Check if directory exists
if [ ! -d "$1" ]; then
    echo "Error: $1 is not a directory."
    exit 1
fi

# Get the directory path and search string from command line arguments
filesdir="$1"
searchstr="$2"

# Count the number of files in the directory and all subdirectories
num_files=$(find "$filesdir" -type f | wc -l)

# Count the number of matching lines in respective files
num_matching_lines=$(grep -r "$searchstr" "$filesdir" | wc -l)

# Print the number of files and matching lines
echo "The number of files are $num_files and the number of matching lines are $num_matching_lines"

