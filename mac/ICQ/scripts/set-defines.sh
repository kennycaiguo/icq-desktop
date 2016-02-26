#!/bin/bash

CONFIG_HEADER="${1}"
HOCKEY_APPID="${2}"
TARGET="${3}"
DEVELOP="${4}"

echo "//Generated header" > "${CONFIG_HEADER}"
echo "#define ${TARGET}" >> "${CONFIG_HEADER}"
echo "#define HOCKEY_APPID @\"${HOCKEY_APPID}\"" >> "${CONFIG_HEADER}"

if [ "${DEVELOP}" == "1" ]; then
echo "#define DEVELOP" >> "${CONFIG_HEADER}"
fi

