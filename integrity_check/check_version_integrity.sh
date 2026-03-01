#!/bin/bash

LOG_DIR="../.logs"
VI_LOG_FILE="$LOG_DIR"/.vi.log
ICONFIG_FILE="../.iconfig.ini"

git fetch origin main
git diff --quiet main origin/main
DIFF_STATUS=$?

VERSION_INTEGRITY="0"

mkdir -p "$LOG_DIR"
touch "$VI_LOG_FILE"

if [ "$DIFF_STATUS" -eq 0 ]; then
    VERSION_INTEGRITY="1"

    if [ -e "$ICONFIG_FILE" ]; then
        grep -q "^versionIntegrity=" "$ICONFIG_FILE" \
        && sed -i "s/^versionIntegrity=.*/versionIntegrity=$VERSION_INTEGRITY/" "$ICONFIG_FILE" \
        || echo "versionIntegrity=$VERSION_INTEGRITY" >> "$ICONFIG_FILE"

        TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")

        if grep -q "^versionIntegrity=1" "$ICONFIG_FILE"; then
            echo "$TIMESTAMP | Integrity Check: Version integrity verified..." | tee -a "$VI_LOG_FILE"
        fi

    else
        TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")
        echo "$TIMESTAMP | Missing iconfig.ini: No iconfig files found..." | tee -a "$VI_LOG_FILE"

    fi

elif [ "$DIFF_STATUS" -eq 1 ]; then
    TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")
    echo "$TIMESTAMP | Integrity Check: Version integrity compromised..." | tee -a "$VI_LOG_FILE"

else
    TIMESTAMP=$(date +"%Y-%m-%d %H:%M:%S")
    echo "$TIMESTAMP | Git Error: A Git error occurred..." | tee -a "$VI_LOG_FILE"

fi
