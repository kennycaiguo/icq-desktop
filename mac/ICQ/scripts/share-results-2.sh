#!/bin/bash
TARGET_DIR=$1
SHARE_DIR=$2

if [ "${CI_FTP_PATH}" != "" ]; then

if [ -d "${SHARE_DIR}" ]; then
    rm -R "${SHARE_DIR}"
fi	

    mkdir -p "${SHARE_DIR}"
    cp -R "${TARGET_DIR}/Products/Applications/"* "${SHARE_DIR}"
fi
