#/bin/bash

FEED_URL="${1}"
CONFIG_HEADER="${2}"
PRODUCT_PREFIX="${3}"

echo Setting spurkle update parameters with feed url - ${FEED_URL}

/usr/libexec/PlistBuddy -c "Add :SUEnableAutomaticChecks bool true" mac/ICQ/imosx-icq-Info.plist
/usr/libexec/PlistBuddy -c "Add :SUFeedURL string '${FEED_URL}'" mac/ICQ/imosx-icq-Info.plist
/usr/libexec/PlistBuddy -c "Add :SUPublicDSAKeyFile string 'dsa_pub.pem'" mac/ICQ/imosx-icq-Info.plist
/usr/libexec/PlistBuddy -c "Add :SUShowReleaseNotes bool true" mac/ICQ/imosx-icq-Info.plist
 
/usr/libexec/PlistBuddy -c "Merge mac/ICQ/scripts/sparkle.objects.pbxproj objects" mac/ICQ/ICQ.xcodeproj/project.pbxproj
/usr/libexec/PlistBuddy -c "Merge mac/ICQ/scripts/sparkle.copy.pbxproj objects:18AA235A1C11F69900A4A5CC:files" mac/ICQ/ICQ.xcodeproj/project.pbxproj
/usr/libexec/PlistBuddy -c "Merge mac/ICQ/scripts/sparkle.link.pbxproj objects:D5E1463C1BC54E39007F8671:files" mac/ICQ/ICQ.xcodeproj/project.pbxproj

# /usr/libexec/PlistBuddy -c "Merge scripts/imosx-helper.objects.pbxproj objects" mac/ICQ/ICQ.xcodeproj/project.pbxproj
# /usr/libexec/PlistBuddy -c "Merge scripts/imosx-helper.resources.pbxproj objects:C59E5B3A1681D57600040E97:files" mac/ICQ/ICQ.xcodeproj/project.pbxproj

echo "#define UPDATES" >> "${CONFIG_HEADER}"
echo "${PRODUCT_PREFIX}" > mac/ICQ/scripts/updates/product-prefix
 
