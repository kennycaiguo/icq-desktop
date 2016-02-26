#!/bin/bash

COMMAND=$1
PROGRAM_NAME=$2
RESOURCES_DIR=$3

TMP_DIR="./tmp"

function volume_from_app {
	PROGRAM_NAME="$1"
	TMP_DIR="./tmp"
	DMG_NAME_TMP="temp_image.dmg"
	VOL_NAME="${PROGRAM_NAME}"
	APP_BUNDLE_NAME="${PROGRAM_NAME}.app"

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
}

function set_volume_elements {
	PROGRAM_NAME="$1"
	APP_BUNDLE_NAME="${PROGRAM_NAME}.app"
	VOL_NAME="${PROGRAM_NAME}"
	BG_IMG_NAME="dmg-bg.png"
	VOL_ICON_NAME="dmg-icon.icns"

	echo "*** Setting style for temporary dmg image..."

	echo -n "    * Copying background image and volume icon... "

	BG_FOLDER="/Volumes/${VOL_NAME}/.background"
	mkdir "${BG_FOLDER}"
	cp "${RESOURCES_DIR}/${BG_IMG_NAME}" "${BG_FOLDER}/"

	ICON_FOLDER="/Volumes/${VOL_NAME}"
	cp "${RESOURCES_DIR}/${VOL_ICON_NAME}" "${ICON_FOLDER}/.VolumeIcon.icns"

	ln -s /Applications "/Volumes/${VOL_NAME}/Applications"
}

function close_volume {
	VOL_NAME=$1
	echo "    * Changing mode and syncing..."
	chmod -Rf go-w /Volumes/"${VOL_NAME}"
	sync
	sync
	echo "    * Detaching ${device}..."
	hdiutil detach ${device}
	echo "done!"
}

function remove_temp_files {
	TMP_DIR="./tmp"
	DMG_NAME_TMP="temp_image.dmg"
	echo -n "*** Removing temporary image... "
	rm -f "${DMG_NAME_TMP}"
	echo "done!"

	echo -n "*** Cleaning up temp folder... "
	rm -rf $TMP_DIR
	echo "done!"
}

function prepare_metainfo {
	PROGRAM_NAME="$1"
	APP_BUNDLE_NAME="${PROGRAM_NAME}.app"
	VOL_NAME="${PROGRAM_NAME}"
	ICON_FOLDER="/Volumes/${VOL_NAME}"
	BG_IMG_NAME="dmg-bg.png"
	VOL_ICON_NAME="dmg-icon.icns"

	BACK_SIZE="400, 100, 1200, 595"
	ICON_SIZE="128"
	APP_PLACE="240, 250"
	APPLICATIONS_PLACE="545, 250"

#   BACK_SIZE="400, 100, 1000, 500"
#	ICON_SIZE="72"
#	APP_PLACE="160, 190"
#	APPLICATIONS_PLACE="440, 190"

	echo -n "    * Executing applescript for further customization... "
	echo '
	tell application "Finder"
		tell disk "'${VOL_NAME}'"
			open
			-- Setting view options
			set current view of container window to icon view
			set toolbar visible of container window to false
			set statusbar visible of container window to false
			set the bounds of container window to {'${BACK_SIZE}'}
			set theViewOptions to the icon view options of container window
			set arrangement of theViewOptions to not arranged
			set icon size of theViewOptions to '${ICON_SIZE}'
			-- Settings background
			set background picture of theViewOptions to file ".background:'${BG_IMG_NAME}'"
			-- Reopening
			close
			open
			delay 2
			-- Rearranging
			set the position of item "Applications" to {'${APPLICATIONS_PLACE}'}
			update without registering applications
			close
			open
			delay 2
			set the position of item "'${APP_BUNDLE_NAME}'" to {'${APP_PLACE}'}
			-- Updating and sleeping for 5 secs
			update without registering applications
			close
			open
			delay 15
		end tell
	end tell
	' | osascript
	echo "done!"

	echo -n "    * Setting volume icon... "
	SetFile -c icnC "${ICON_FOLDER}/.VolumeIcon.icns"
	SetFile -a C "${ICON_FOLDER}"
	echo "done!"

	echo -n "* Copy .DS_Store to metainfo... "
	cp "/Volumes/${VOL_NAME}/.DS_Store" metainfo
	echo "done!"

	echo -n "*** Check result for 5 seconds..."
	sleep 5
	echo "done!"
}

function convert_image {
	PROGRAM_NAME="$1"
	APP_BUNDLE_NAME="${PROGRAM_NAME}.app"
	APP_VERSION=`/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "${APP_BUNDLE_NAME}/Contents/Info.plist"`
	APP_BUILD_VERSION=`/usr/libexec/PlistBuddy -c "Print :CFBundleVersion" "${APP_BUNDLE_NAME}/Contents/Info.plist"`
	DMG_NAME="${PROGRAM_NAME} ${APP_VERSION}.${APP_BUILD_VERSION}.dmg"

	DMG_NAME_TMP="temp_image.dmg"

	echo "*** Converting tempoprary dmg image in compressed readonly final image... "
	rm -f ${DMG_NAME}
	echo "    * Converting to ${DMG_NAME}..."
	hdiutil convert "${DMG_NAME_TMP}" -format UDZO -imagekey zlib-level=9 -o "${DMG_NAME}"
	echo "done!"
}

case "$COMMAND" in
	"metainfo")
		volume_from_app "${PROGRAM_NAME}"
		set_volume_elements "${PROGRAM_NAME}"

		# cp "${RESOURCES_DIR}/metainfo" "/Volumes/${VOL_NAME}/.DS_Store"
		prepare_metainfo "${PROGRAM_NAME}" 

		close_volume "${PROGRAM_NAME}"
		remove_temp_files
	;;

    "metainfo_noclose")
        volume_from_app "${PROGRAM_NAME}"
        set_volume_elements "${PROGRAM_NAME}"

        # cp "${RESOURCES_DIR}/metainfo" "/Volumes/${VOL_NAME}/.DS_Store"
        prepare_metainfo "${PROGRAM_NAME}"
    ;;

	"dmg")
		volume_from_app "${PROGRAM_NAME}"
		set_volume_elements "${PROGRAM_NAME}"

		cp "${RESOURCES_DIR}/metainfo" "/Volumes/${VOL_NAME}/.DS_Store"

		close_volume "${PROGRAM_NAME}"
		convert_image "${PROGRAM_NAME}"
		remove_temp_files
	;;

esac

echo "
*** Everything done. File 'metainfo' ready.
"




