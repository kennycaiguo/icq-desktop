#!/bin/bash
TARGET_DIR=$1

cd "${TARGET_DIR}"

FILE="${HOME}/Documents/imosx-icq-build"
APP_BUNDLE_NAME="ICQ.app"
if [ -f $FILE ]; then
	IMOSX_ICQ_BUILD=`cat "${FILE}"`
	/usr/libexec/PlistBuddy -c "Set :CFBundleVersion $IMOSX_ICQ_BUILD" -c "Save" "dSYMs/${APP_BUNDLE_NAME}.dSYM/Contents/Info.plist"
fi
