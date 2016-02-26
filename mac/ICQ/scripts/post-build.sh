#!/bin/bash
ARCHIVE_PATH=$1
SRCROOT=$2
# SHARE_PATH=${CI_FTP_PATH}/${TEAMCITY_BUILDCONF_NAME}/${APP_BUILD_VERSION}
env
echo "Prepare post build"

echo "${SHARE_PATH}"
echo "${PRODUCT_NAME}"

cd "${ARCHIVE_PATH}/Products/Applications"

APP_BUNDLE_NAME="${PRODUCT_NAME}.app"
APP_VERSION=`/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "${APP_BUNDLE_NAME}/Contents/Info.plist"`
APP_BUILD_VERSION=`/usr/libexec/PlistBuddy -c "Print :CFBundleVersion" "${APP_BUNDLE_NAME}/Contents/Info.plist"`

FEED_URL=""
if [ "${UPDATE}" == "ENABLE" ]
then
FEED_URL=`/usr/libexec/PlistBuddy -c "Print :SUFeedURL" "${APP_BUNDLE_NAME}/Contents/Info.plist"`
fi

DMG_NAME="${PRODUCT_NAME} ${APP_VERSION}.${APP_BUILD_VERSION}.dmg"
APP_DSYM="${ARCHIVE_PATH}/dSYMs/${APP_BUNDLE_NAME}.dSYM"

echo "Post build vars"
echo "APP_BUNDLE_NAME ${APP_BUNDLE_NAME}"
echo "APP_VERSION ${APP_VERSION}"
echo "APP_BUILD_VERSION ${APP_BUILD_VERSION}"
echo "FEED_URL ${FEED_URL}"
echo "DMG_NAME ${DMG_NAME}"
echo "APP_DSYM ${APP_DSYM}"

if [ "${FEED_URL}" == "" ]; then
	echo "Delete Sparkle framework"
	# rm -rf "${APP_BUNDLE_NAME}/Contents/Frameworks/Sparkle.framework"
fi

if [ "${CODESIGN_ENTITLEMENTS}" != "" ]; then
	CODESIGN_ENTITLEMENTS="${SRCROOT}/${CODESIGN_ENTITLEMENTS}"
	echo "CODESIGN_ENTITLEMENTS @{CODESIGN_ENTITLEMENTS}"
fi

echo "Start signing"
${SRCROOT}/scripts/sign-app.sh "${APP_BUNDLE_NAME}" "${CODESIGN_IDENTITY}" "${CODESIGN_ENTITLEMENTS}"

if [ "${CODESIGN_INSTALLER}" != "" ]; then
	echo "Start productbuild"
	/usr/bin/productbuild --component "${APP_BUNDLE_NAME}" /Applications "${PRODUCT_NAME}.pkg" --sign "${CODESIGN_INSTALLER}"
fi

echo "Start Zip Artifacts"
zip -ry "xcarchive.${APP_VERSION}.${APP_BUILD_VERSION}.zip" "${ARCHIVE_PATH}"
zip -9 -ry "${APP_BUNDLE_NAME}.zip" "${APP_BUNDLE_NAME}"
zip -9 -ry "${PRODUCT_NAME}.dSYM.zip" "${APP_DSYM}"

echo "Start DMG creating"
${SRCROOT}/scripts/dmg-tools.sh dmg "${PRODUCT_NAME}" "${SRCROOT}/DMG"

if [ "${UPDATE}" == "ENABLE" ]
then
echo "Start preparing updates"
${SRCROOT}/scripts/updates/prepare_update.sh "${DMG_NAME}" "${APP_BUNDLE_NAME}" . ${SRCROOT}/scripts/updates
fi

echo "Start sharing results"
${SRCROOT}/scripts/share-results-2.sh "${ARCHIVE_PATH}" "${SHARE_PATH}"

exit 0
