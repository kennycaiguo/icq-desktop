#!/bin/bash
TARGET_DIR=$1

cd "${TARGET_DIR}"

APP_BUNDLE_NAME="${PROGRAM_NAME}.app"
APP_VERSION=`/usr/libexec/PlistBuddy -c "Print :CFBundleShortVersionString" "${TARGET_DIR}/Products/Applications/${APP_BUNDLE_NAME}/Contents/Info.plist"`
APP_BUILD_VERSION=`/usr/libexec/PlistBuddy -c "Print :CFBundleVersion" "${TARGET_DIR}/Products/Applications/${APP_BUNDLE_NAME}/Contents/Info.plist"`
DMG_NAME="${PROGRAM_NAME} ${APP_VERSION}.${APP_BUILD_VERSION}.dmg"
#if [ -z "${CI_FTP_PATH}" ]; then
    mkdir "${CI_FTP_PATH}/${TEAMCITY_BUILDCONF_NAME}"
    mkdir "${CI_FTP_PATH}/${TEAMCITY_BUILDCONF_NAME}/${APP_BUILD_VERSION}"
    cp -R "${TARGET_DIR}" "${CI_FTP_PATH}/${TEAMCITY_BUILDCONF_NAME}/${APP_BUILD_VERSION}"
    cp -R "${TARGET_DIR}/Products/Applications/${APP_BUNDLE_NAME}" "${CI_FTP_PATH}/${TEAMCITY_BUILDCONF_NAME}/${APP_BUILD_VERSION}"
    cp -R "${TARGET_DIR}/Products/Applications/${DMG_NAME}" "${CI_FTP_PATH}/${TEAMCITY_BUILDCONF_NAME}/${APP_BUILD_VERSION}"
#fi
