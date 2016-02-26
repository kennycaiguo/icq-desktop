#!/bin/bash

FILE=$1
BUNDLE_PATH="$2"
BUNDLE_PLIST_PATH="$2/Contents/Info.plist"
RESULTING_PATH="$3"
TEMPLATES_PATH="$4"
PRODUCT_PREFIX=`cat ${TEMPLATES_PATH}/product-prefix`

if [ "${RESULTING_PATH}" == "" ]; then
	RESULTING_PATH="$PWD/update"
fi

if [ "$FILE" = "" -o "${BUNDLE_PATH}" = "" ]; then
	echo "usage: ./prepare_update.sh filename.dmg bundle.app"
	exit 1
fi

if [ ! -e "$FILE" ]; then
	echo "File $FILE does not exist"
	exit 2
fi

if [ ! -e "$BUNDLE_PLIST_PATH" ]; then
	echo "File $BUNDLE_PLIST_PATH does not exist"
	exit 3
fi

echo "Preparing update..."

function read_plist()
{
	/usr/libexec/PlistBuddy -c "Print :$1" "${BUNDLE_PLIST_PATH}"
}

function app_ver()
{
	read_plist "CFBundleVersion"
}

function app_full_ver()
{
	read_plist "CFBundleShortVersionString"
}

function signature()
{
	openssl dgst -sha1 -binary < "${FILE}" | openssl dgst -dss1 -sign "${TEMPLATES_PATH}/../../dsa_priv.pem" | openssl enc -base64
}

function filesize()
{
	stat -f "%z" "$FILE"
}

function read_changes()
{
	CHANGES="${TEMPLATES_PATH}/changes.txt"
	while IFS= read -r line
	do
		echo "		<li>$line</li>"
	done < "$CHANGES"
}

function prepare_appcast()
{
	APPCAST_FILE="${RESULTING_PATH}/${PRODUCT_PREFIX}_update.xml"
	cat "${TEMPLATES_PATH}/${PRODUCT_PREFIX}_appcast_header_xml.template" > ${APPCAST_FILE}
	CURRENT_DATE=`LANG=en_EN date "+%a, %d %b %Y %T %z"`
	TMP_ITEM="${TEMPLATES_PATH}/${PRODUCT_PREFIX}_appcast_item_xml.template"
	sed "s|APP_VERSION|$(app_ver)|g;s|APP_FULL_VERSION_STRING|$(app_full_ver)|g;s|FILE_SIGNATURE|$(signature)|g;s|FILE_SIZE|$(filesize)|g;s|PUB_DATE|${CURRENT_DATE}|g" ${TMP_ITEM} >> ${APPCAST_FILE}
	cat "${TEMPLATES_PATH}/appcast_footer_xml.template" >> ${APPCAST_FILE}
}

function prepare_changelog()
{
	CHANGELOG_FILE="${RESULTING_PATH}/release_notes_icq.html"
	sed "s|APP_VERSION|$(app_ver)|g;s|APP_FULL_VERSION_STRING|$(app_full_ver)|g" "${TEMPLATES_PATH}/changelog_header_html.template" > ${CHANGELOG_FILE}
	read_changes >> ${CHANGELOG_FILE}
	cat "${TEMPLATES_PATH}/changelog_footer_html.template" >> ${CHANGELOG_FILE}
}

# go

mkdir -p "${RESULTING_PATH}"

prepare_appcast
prepare_changelog
