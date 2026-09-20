#!/bin/sh
#
# usage: program packet_length [data files...]
#
# prints out trials that match the given length
#
#set -x
tempfile=tempfile.$$
rm -f $tempfile

echo '$1 == "#" { next }' >$tempfile
echo '$3 == ' "\"$1\" { print " '$4 }' >>$tempfile

shift
awk -f $tempfile $@

rm -f $tempfile
