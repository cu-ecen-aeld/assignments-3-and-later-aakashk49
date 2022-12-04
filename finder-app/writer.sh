#!/bin/bash

if [ $# -lt 2 ]
then
    echo "Enter file name and string"
    exit 1
fi
CHECKDIR=$( dirname $1)
#echo $CHECKDIR
if [ ! -d $CHECKDIR ]
then
    mkdir -p $CHECKDIR
fi
touch $1
echo $2 > $1