#!/bin/bash

# Check if the script is running from /usr/bin
SCRIPT_PATH=$(realpath "$0")
if [[ $SCRIPT_PATH != /usr/bin/finder-test.sh ]]; then
    echo "Error: The script must be located at /usr/bin/finder-test.sh."
    exit 1
fi

# Use default values if not set
NUMFILES=${1:-10}
WRITESTR=${2:-"AELD_IS_FUN"}
WRITEDIR="/tmp/aeld-data"
CONFDIR="/etc/finder-app/conf"

# Create the directory if it does not exist
mkdir -p "$WRITEDIR"

# Write files
for i in $(seq 1 $NUMFILES)
do
    writer "$WRITEDIR/file$i.txt" "$WRITESTR"
done

# Run finder.sh and save output to /tmp/assignment4-result.txt
if [ -f "$CONFDIR/username.txt" ]; then
    USERNAME=$(cat "$CONFDIR/username.txt")
else
    echo "Error: $CONFDIR/username.txt not found"
    exit 1
fi

finder.sh "$WRITEDIR" "$WRITESTR" > /tmp/assignment4-result.txt

# Verify the number of files and matching lines
MATCHFILES=$(grep "The number of files are" /tmp/assignment4-result.txt | cut -d ' ' -f 5)
MATCHLINES=$(grep "The number of matching lines are" /tmp/assignment4-result.txt | cut -d ' ' -f 6)

if [ "$MATCHFILES" -eq "$NUMFILES" ] && [ "$MATCHLINES" -eq "$NUMFILES" ]; then
    echo "success"
    exit 0
else
    echo "failed: expected $NUMFILES files and $NUMFILES matching lines,"
    echo "but found $MATCHFILES files and $MATCHLINES matching lines."
    exit 1
fi
