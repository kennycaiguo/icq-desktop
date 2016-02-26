#!/bin/sh

#  duplicator.sh
#  imosx
#
#  Created by v.kubyshev on 13/03/14.
#  Copyright (c) 2014 Mail.Ru. All rights reserved.

# can be used for duplicate one file to another
# example for generate file01.png file02.png etc

#usage:
#duplicator <from_counter> <to_counter> <padding_length> <prefix_name> <ext> <source>

padlength=$3
seqfrom=$1
seqto=$2

prefix=$4
ext=$5
source=$6

for i in $(seq -f "%0"$padlength"g" $seqfrom $seqto)
do
new_name=$prefix$i"."$ext
echo "Copy to "$new_name
cp $source $new_name
done