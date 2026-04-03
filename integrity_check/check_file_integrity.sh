#!/bin/bash

MANIFEST_FILE="../.manifest/manifest.json"
SRC_DIR="../src"
FAIL_COUNT=0

if [ ! -f "$MANIFEST_FILE" ]; then
    echo "Missing manifest.json file!"
    exit 1
fi

while read -r FILE_PATH EXPECTED_HASH; do
    FULL_PATH="$SRC_DIR/$FILE_PATH"

    if [ ! -f "$FULL_PATH" ]; then
        echo "Missing File: $FILE_PATH"
        ((FAIL_COUNT++))
        continue
    fi

    ACTUAL_HASH=$(sha256sum "$FULL_PATH" | awk '{print $1}')

    if [ "$ACTUAL_HASH" != "$EXPECTED_HASH" ]; then
        echo "Tamper Detected: $FILE_PATH"
        echo "    Expected Hash: $EXPECTED_HASH"
        echo "    Hash Found: $ACTUAL_HASH"
        ((FAIL_COUNT++))
    else
        echo "File Verified: $FILE_PATH"
    fi

done < <(jq -r '.files[] | "\(.path) \(.hash)"' "$MANIFEST")

echo "      ---------------------------------       "

if [ $FAIL_COUNT -gt 0 ]; then
    echo "CRITICAL ISSUE DETECTED: $FAIL_COUNT files(s) failed Integrity Check!"
    echo "Could not Start the ENGINES until this is fixed!"
    
    exit 1
else
    echo "No issues detected!"
    echo "All files verified!"

    exit 0
fi
