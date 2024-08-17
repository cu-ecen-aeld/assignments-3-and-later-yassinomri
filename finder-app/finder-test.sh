#!/bin/bash

# Check if the script is running from /usr/bin
SCRIPT_PATH=$(realpath "$0")
if [[ $SCRIPT_PATH != /usr/bin/finder-test.sh ]]; then
    echo "Error: The script must be located at /usr/bin/finder-test.sh."
    exit 1
fi

# Clean previous build artifacts
make clean

# Compile the writer application
make

# Use default values if not set
WRITE_STR=${1:-"AELD_IS_FUN"}
NUM_FILES=${2:-10}
WRITR_PATH="/tmp/aeld-data"

# Create the directory if it does not exist
mkdir -p ${WRITR_PATH}

# Write files
for i in $(seq 1 ${NUM_FILES}); do
    ./writer ${WRITR_PATH}/file${i}.txt ${WRITE_STR}
done

# Verify the number of files and lines
NUM_WRITTEN=$(find ${WRITR_PATH} -type f | wc -l)
NUM_MATCHING_LINES=$(grep -r ${WRITE_STR} ${WRITR_PATH} | wc -l)

if [[ ${NUM_WRITTEN} -ne ${NUM_FILES} || ${NUM_MATCHING_LINES} -ne ${NUM_FILES} ]]; then
    echo "failed: expected The number of files are ${NUM_FILES} and the number of matching lines are ${NUM_FILES} but instead found"
    echo "The number of files are ${NUM_WRITTEN} and the number of matching lines are ${NUM_MATCHING_LINES}"
    exit 1
else
    echo "success"
fi

# Run the finder command and write its output to /tmp/assignment4-result.txt
finder > /tmp/assignment4-result.txt

