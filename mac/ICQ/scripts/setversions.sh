#/bin/bash

if [ ! $# -eq 2 ]
then
	echo Give program version and build version as agrs
	exit -1
fi

BUILD_VERSION=$1
BUILD_NUMBER=$2

echo Setting program version to ${BUILD_VERSION}, build number to ${BUILD_NUMBER}

VERSION_INFO_STR='#define VERSION_INFO_STR "'${BUILD_VERSION}.${BUILD_NUMBER}'"'
VERSION_INFO='#define VERSION_INFO "'${BUILD_VERSION}.${BUILD_NUMBER}'"'

SED_MACRO_VER='s;#define VERSION_INFO .*$;'${VERSION_INFO}';'
SED_MACRO_VER_STR='s;#define VERSION_INFO_STR .*$;'${VERSION_INFO_STR}';'

sed -i '.bak' "$SED_MACRO_VER_STR" common.shared/version_info_constants.h

/usr/libexec/PlistBuddy -c "Set :CFBundleShortVersionString ${BUILD_VERSION}" mac/ICQ/imosx-icq-Info.plist
/usr/libexec/PlistBuddy -c "Set :CFBundleVersion ${BUILD_NUMBER}" mac/ICQ/imosx-icq-Info.plist
