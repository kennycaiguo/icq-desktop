#!/bin/bash

# APP_ID=0b8e725b29b418d5edb4aa672fe54526
# APP_TOKEN=d98fa628a447475db3933d46d8c291e6
APP_ID=$1
APP_TOKEN=$2
DSYM_ZIP=$3
APP_ZIP=$4


UPLOAD_URL="https://rink.hockeyapp.net/api/2/apps/${APP_ID}/app_versions"
POST_HEADER="X-HockeyAppToken:${APP_TOKEN}"
CURL_OPTIONS="--header ${POST_HEADER} --form dsym=@${DSYM_ZIP}  --form app=@${APP_ZIP} ${UPLOAD_URL}"
NOTES=""


echo "Executing curl ${CURL_OPTIONS}"

curl \
  -F "status=1" \
  -F "notify=1" \
  -F "notes=${NOTES}" \
  -F "notes_type=0" \
  -F "dsym=@${DSYM_ZIP}" \
  -H "X-HockeyAppToken: ${APP_TOKEN}" \
  "${UPLOAD_URL}" 

echo
