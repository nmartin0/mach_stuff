#! /bin/sh

# To distribute MacMach, set up a user "macmach".  Build a system from
# source and place the binaries and sources on a disk mounted read-only.
# This shell script will set up all of the details for distribution.

# The following directory structure works:
# /users/macmach              -- link to macmach user, /usr/users/macmach
# /users/macmach/.mach        -- read-only disk mount point
# /users/macmach/.mach/os     -- binaries
# /users/macmach/.mach/src    -- sources
# /users/macmach/mach         -- link to /users/macmach/.mach/os
# /users/macmach/mach/usr/src -- link to /users/macmach/.mach/src
# /users/macmach/install      -- contains BINHEX of Mach INIT/cdev

echo "Securing the MacMach distribution in /users/macmach..."

# The following user and group id's should exist:
# user macmach  -- this user owns everything
# group macmach -- access to macmach installation kit
# group binary  -- access to system binaries
# group source  -- access to system sources

echo "Checking that directories exist..."

[ -d /users/macmach ] || {
  echo "Please create /users/macmach and try again."
  exit 1
}
[ -d /users/macmach/install ] || mkdir /users/macmach/install
[ -d /users/macmach/.mach ] || mkdir /users/macmach/.mach
[ -d /users/macmach/.mach/os ] || {
  echo "Please create /users/macmach/.mach/os and try again."
  exit 1
}
[ -d /users/macmach/.mach/src ] || {
  echo "Please create /users/macmach/.mach/src and try again."
  exit 1
}
[ -f /users/macmach/install/Mach.Hqx ] || {
  echo "Please create /users/macmach/install/Mach.Hqx and try again."
  exit 1
}

echo "Installing files..."

cp /.login /users/macmach/.login
cp /.cshrc /users/macmach/.cshrc

cat >/users/macmach/.plan <<'@EOF@'
MacMach is BSD 4.3 (Tahoe) unix on top of the CMU Mach kernel.

The MacMach distribution server allows users to install MacMach systems
over the internet via ftp.  Send mail to macmach@andrew.cmu.edu for more
information.
@EOF@

cat >/users/macmach/README <<'@EOF@'
macmach/README, 1/5/92

MacMach is BSD 4.3 (Tahoe) unix on top of the CMU Mach kernel.

For more information, send mail to macmach@andrew.cmu.edu.

The macmach home directory on the MacMach distribution server contains:
  README  -- this document
  install -- directory containing installation kit
  mach   -- a Mach 3.0 system

Access is controlled via these groups:
  macmach -- access to installation kit
  binary -- access to Mach 3.0 binaries
  source -- access to Mach 3.0 sources
@EOF@

cat >/users/macmach/install/README <<'@EOF@'
macmach/install/README, 1/5/92

To install MacMach, download Mach.Hqx.  This is a binhex'ed self-extracting
archive that contains a control panel that must be placed into the system
folder.  Once this is done, go into the Mach control panel to set up the
default root device and load device.  Enable Mach startup and reboot.  When
the Mach startup dialog is presented, press the Alternate startup button.
This will startup a small system from a ramdisk contained within the
control panel file.  Answer the questions presented and a complete system
will be installed.

Hints:  MacMach needs at least 55MB of disk space for the binaries.
It needs another 200MB for sources.  The installation will grab any
free partition on the Mac disk.  The Mach 3.0 system pages to an
Apple_Scratch partition, it needs about 20MB.  The Mach control panel
takes up about 2MB.  Give at least 5MB for a System Volume.  Use a
Macintosh disk utility to partition the disk before installing MacMach.

Send mail to macmach@andrew.cmu.edu for more information.
@EOF@

set -x

echo "Securing binary and source trees..."

fixit -user macmach -group binary /users/macmach/.mach/os
fixit -user macmach -group source /users/macmach/.mach/src

echo "Connecting binary and source trees..."
rm -f /users/macmach/mach
ln -s .mach/os /users/macmach/mach
rm -f /users/macmach/.mach/os/usr/src
ln -s ../../src /users/macmach/.mach/os/usr/src

echo "Setting bits for individual files..."

chown macmach.binary /users/macmach/.mach
chmod 550 /users/macmach/.mach

chown macmach.macmach /users/macmach/.login
chmod 400 /users/macmach/.login

chown macmach.macmach /users/macmach/.cshrc
chmod 400 /users/macmach/.cshrc

chown macmach.macmach /users/macmach/.plan
chmod 444 /users/macmach/.plan

chown macmach.macmach /users/macmach/README
chmod 444 /users/macmach/README

chown macmach.macmach /users/macmach/install
chmod 555 /users/macmach/install

chown macmach.macmach /users/macmach/install/*
chmod 400 /users/macmach/install/*

chown macmach.macmach /users/macmach/install/README
chmod 444 /users/macmach/install/README

chown macmach.macmach /users/macmach/install/Mach*.Hqx
chmod 440 /users/macmach/install/Mach*.Hqx

echo "Verifying binary checksums..."
fixit -check /users/macmach/mach/ < /users/macmach/mach/.fixit |
  grep ': sum'

echo "Verifying source checksums..."
fixit -check /users/macmach/mach/ < /users/macmach/mach/usr/src/.fixit |
  grep ': sum'

