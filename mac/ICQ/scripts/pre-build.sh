#!/bin/bash
PROGRAM_NAME=$1
APP_VERSION=$2
APP_BUILD_VERSION=$3
UPDATE=$4
DEVELOP=$5
ITUNESSTORE=$6

PRODUCT_PREFIX="icq"
FEED_URL="http://mra.mail.ru/icq_mac_update/icq_update.xml"


./mac/ICQ/scripts/setversions.sh "${APP_VERSION}" "${APP_BUILD_VERSION}" 

if [ "${UPDATE}" == "ENABLE" ]
then
	echo Update is ${UPDATE}
	./mac/ICQ/scripts/enable-update.sh "${FEED_URL}" "mac/ICQ/macconfig.h" "${PRODUCT_PREFIX}"
fi

if [ "${ITUNESSTORE}" == "ENABLE" ]
then
	echo Store is ${ITUNESSTORE}
	echo "#define ITUNESSTORE" >> "mac/ICQ/Config.h" 
fi