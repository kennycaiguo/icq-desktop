#!/bin/sh

if [ "$1" == "" ]; then
	echo "Usage: sign.sh file.zip"
	exit 1;
fi

ruby sign_update.rb $1 ../../dsa_priv.pem