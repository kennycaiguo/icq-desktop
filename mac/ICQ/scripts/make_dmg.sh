#!/bin/bash

TARGET_DIR=$1
SRCROOT_DIR=$2

cd "${TARGET_DIR}"

echo "Target dir is ${TARGET_DIR}"
pwd

env

TMP_DIR="./tmp"

VOL_NAME="${PROGRAM_NAME}"
BG_IMG_NAME="imosx-dmg-bg.png"
VOL_ICON_NAME="icq-dmg-icon.icns"
APP_BUNDLE_NAME="${PROGRAM_NAME}.app"
APP_VERSION=`/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "${APP_BUNDLE_NAME}/Contents/Info.plist"`
APP_BUILD_VERSION=`/usr/libexec/PlistBuddy -c "Print :CFBundleVersion" "${APP_BUNDLE_NAME}/Contents/Info.plist"`

DMG_NAME_TMP="icq_tmp.dmg"
DMG_NAME="${PROGRAM_NAME} ${APP_VERSION}.${APP_BUILD_VERSION}.dmg"

CODESIGN_ENTITLEMENTS="ICQ.entitlements"

echo -n "*** Signing ${APP_BUNDLE_NAME} with identity '${CODESIGN_IDENTITY}'... "
# codesign --entitlements "${SRCROOT_DIR}/${CODESIGN_ENTITLEMENTS}" -fs "${CODESIGN_IDENTITY}" "${APP_BUNDLE_NAME}"
codesign -fs "${CODESIGN_IDENTITY}" "${APP_BUNDLE_NAME}"
codesign -display --entitlements - "${APP_BUNDLE_NAME}"
echo "done!"

echo -n "*** Copying ${APP_BUNDLE_NAME} to the temporary dir... "
mkdir $TMP_DIR
cp -R "${APP_BUNDLE_NAME}" ${TMP_DIR}/
echo "done!"

echo -n "*** Creating temporary dmg disk image..."
rm -f ${DMG_NAME_TMP}
hdiutil create -ov -srcfolder $TMP_DIR -format UDRW -volname "${VOL_NAME}" "${DMG_NAME_TMP}"

echo -n "*** Mounting temporary image... "
device=$(hdiutil attach -readwrite -noverify -noautoopen ${DMG_NAME_TMP} | egrep '^/dev/' | sed 1q | awk '{print $1}')
echo "done! (device ${device})"

echo -n "*** Sleeping for 5 seconds..."
sleep 5
echo " done!"

echo "*** Setting style for temporary dmg image..."

echo -n "    * Copying background image and volume icon... "

BG_FOLDER="/Volumes/${VOL_NAME}/.background"
mkdir "${BG_FOLDER}"
cp "${APP_BUNDLE_NAME}/Contents/Resources/${BG_IMG_NAME}" "${BG_FOLDER}/"

ICON_FOLDER="/Volumes/${VOL_NAME}"
cp "${APP_BUNDLE_NAME}/Contents/Resources/${VOL_ICON_NAME}" "${ICON_FOLDER}/.VolumeIcon.icns"

ln -s /Applications "/Volumes/${VOL_NAME}/"

cp "${APP_BUNDLE_NAME}/Contents/Resources/metainfo" "/Volumes/${VOL_NAME}/.DS_Store"

# echo "done!"

# echo -n "    * Executing applescript for further customization... "
# echo '
# tell application "Finder"
# 	tell disk "'${VOL_NAME}'"
# 		open
# 		-- Setting view options
# 		set current view of container window to icon view
# 		set toolbar visible of container window to false
# 		set statusbar visible of container window to false
# 		set the bounds of container window to {400, 100, 1000, 500}
# 		set theViewOptions to the icon view options of container window
# 		set arrangement of theViewOptions to not arranged
# 		set icon size of theViewOptions to 72
# 		-- Settings background
# 		set background picture of theViewOptions to file ".background:'${BG_IMG_NAME}'"
# 		-- Adding symlink to /Applications
# 		make new alias file at container window to POSIX file "/Applications" with properties {name:"Applications"}
# 		-- Reopening
# 		close
# 		open
# 		-- Rearranging
# 		set the position of item "Applications" to {440, 190}
# 		set the position of item "'${APP_BUNDLE_NAME}'" to {160, 190}
# 		-- Updating and sleeping for 5 secs
# 		update without registering applications
# 		delay 5
# 	end tell
# end tell
# ' | osascript
# echo "done!"

# echo -n "    * Setting volume icon... "


# SetFile -c icnC "${ICON_FOLDER}/.VolumeIcon.icns"
# SetFile -a C "${ICON_FOLDER}"

echo "done!"

echo "*** Converting tempoprary dmg image in compressed readonly final image... "
echo "    * Changing mode and syncing..."
chmod -Rf go-w /Volumes/"${VOL_NAME}"
sync
sync
echo "    * Detaching ${device}..."
hdiutil detach ${device}
rm -f ${DMG_NAME}
echo "    * Converting..."
hdiutil convert "${DMG_NAME_TMP}" -format UDZO -imagekey zlib-level=9 -o "${DMG_NAME}"
echo "done!"

echo -n "*** Removing temporary image... "
rm -f "${DMG_NAME_TMP}"
echo "done!"

echo -n "*** Cleaning up temp folder... "
rm -rf $TMP_DIR
echo "done!"

echo "
*** Everything done. DMG disk image is ready for distribution.
"
