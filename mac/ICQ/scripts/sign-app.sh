#!/bin/bash

APP_BUNDLE_NAME=$1
CODESIGN_IDENTITY=$2
CODESIGN_ENTITLEMENTS=$3


echo -n "*** Signing ${APP_BUNDLE_NAME} with identity '${CODESIGN_IDENTITY}'... "
if [ "${CODESIGN_ENTITLEMENTS}" == "" ]
then
	codesign -fs "${CODESIGN_IDENTITY}" --force --verbose --deep "${APP_BUNDLE_NAME}"
else
	codesign --entitlements "${CODESIGN_ENTITLEMENTS}" -fs "${CODESIGN_IDENTITY}" --force --verbose --deep "${APP_BUNDLE_NAME}"
fi
codesign -display --entitlements - "${APP_BUNDLE_NAME}"
echo "done!"
