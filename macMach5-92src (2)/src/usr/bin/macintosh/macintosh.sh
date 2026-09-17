#! /bin/sh

# This shell script starts up the Macintosh Emulator.

MACPATCHES=`wh -L macpatches`
MACSERVER=`wh -L macserver`

$MACSERVER $MACPATCHES
