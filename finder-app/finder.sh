#!/bin/sh

if [ $# -lt 2 ]
then
    echo "enter file name and search str"
    exit 1
#else
    #echo "Correct number of params"
fi

if [ -d $1 ]
then
    #echo "dir name exists"
    nfile=$(ls $1 | wc -l)
    nline=$(grep -r $2 $1 | wc -l)
    echo The number of files are $nfile and the number of matching lines are $nline
else
    echo "dir does not exists"
    exit 1
fi