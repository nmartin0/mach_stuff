#! /bin/sh

# This shell script makes an installation/maintenance ramdisk.
# Once this is done, build a new mach_kernel.

VERSION="MacMach Ramdisk Version 3.0"

# The size just happens to be the size of a high-density floppy...
# Note that this must match the "mac2ramdisk" entry in /etc/disktab!
SIZE=2880

DEVICE=/dev/ramdisk0a
RDEVICE=/dev/rramdisk0a
XDEVICE=/dev/xramdisk0
DIR=/tmp/ramdisk
SRC=/usr/src

# Make /usr/src/Makeconf happy.
SOURCE_TREE=$SRC
BUILD_TREE=/
OBJECT_TREE=$SRC
export SOURCE_TREE BUILD_TREE OBJECT_TREE

set -x

umount $DEVICE
(
  set -x
  dd if=/mach_kernel of=$XDEVICE seek=$SIZE bs=512 count=1
  disklabel -w -r $RDEVICE mac2ramdisk
  newfs $DEVICE
  [ -d $DIR ] || mkdir $DIR
  mount $DEVICE $DIR
) <$XDEVICE

(
  set -x
  cd $DIR
  mkdir mach_servers mnt disk bin etc dev tmp
  chmod 755 mach_servers mnt disk bin etc dev
  chown root.bin mach_servers mnt disk bin etc dev
  ln -s /tmp/.dest/usr usr
)

sh $SRC/etc/makedev/makedev.mac2 $DIR/dev
(
  set -x
  cd $DIR/dev
  rm -f keyboard mouse
  rm -f rdisk0b rdisk0d rdisk0e rdisk0f rdisk1b rdisk1d rdisk1e rdisk1f rdisk2b rdisk2d rdisk2e rdisk2f
  rm -f rdisk3b rdisk3d rdisk3e rdisk3f rdisk4b rdisk4d rdisk4e rdisk4f rdisk5b rdisk5d rdisk5e rdisk5f
  rm -f rdisk6b rdisk6d rdisk6e rdisk6f
  rm -f disk0b disk0d disk0e disk0f disk1b disk1d disk1e disk1f disk2b disk2d disk2e disk2f
  rm -f disk3b disk3d disk3e disk3f disk4b disk4d disk4e disk4f disk5b disk5d disk5e disk5f
  rm -f disk6b disk6d disk6e disk6f
  rm -f xramdisk0 xramdisk1 xramdisk2 xramdisk3 xramdisk4 xramdisk5 xramdisk6 xramdisk7
  rm -f ptyp5 ptyp6 ptyp7 ptyp8 ptyp9 ptypa ptypb ptypc ptypd ptype ptypf
  rm -f ttyp5 ttyp6 ttyp7 ttyp8 ttyp9 ttypa ttypb ttypc ttypd ttype ttypf
)

(
  set -x
  cd $SRC/bin/true
  rm -f true
  make CFLAGS="-n -O" compile
  install -c -m 555 -o root -g bin true.sh $DIR/bin/true
)

(
  set -x
  cd $SRC/bin/false
  rm -f false
  make CFLAGS="-n -O" compile
  install -c -m 555 -o root -g bin false.sh $DIR/bin/false
)

(
  set -x
  cd $SRC/bin/stty
  rm -f stty
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin stty $DIR/bin
)

(
  set -x
  cd $SRC/bin/cp
  rm -f cp
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin cp $DIR/bin
)

(
  set -x
  cd $SRC/bin/pwd
  rm -f pwd
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin pwd $DIR/bin
)

(
  set -x
  cd $SRC/bin/ls
  rm -f ls
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin ls $DIR/bin
)

(
  set -x
  cd $SRC/bin/ln
  rm -f ln
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin ln $DIR/bin
)

(
  set -x
  cd $SRC/bin/rm
  rm -f rm
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin rm $DIR/bin
)

(
  set -x
  cd $SRC/bin/mkdir
  rm -f mkdir
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin mkdir $DIR/bin
)

(
  set -x
  cd $SRC/bin/cat
  rm -f cat
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin cat $DIR/bin
)

(
  set -x
  cd $SRC/bin/sync
  rm -f sync
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin sync $DIR/bin
)

(
  set -x
  cd $SRC/bin/echo
  rm -f echo
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin echo $DIR/bin
)

(
  set -x
  cd $SRC/bin/chmod
  rm -f chmod
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin chmod $DIR/bin
)

(
  set -x
  cd $SRC/bin/ed
  rm -f ed
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin ed $DIR/bin
)

(
  set -x
  cd $SRC/bin/df
  rm -f df
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin df $DIR/bin
)

(
  set -x
  cd $SRC/bin/test
  rm -f test
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin test $DIR/bin
  rm -f $DIR/bin/[; ln $DIR/bin/test $DIR/bin/[
)

(
  set -x
  cd $SRC/bin/sh
  rm -f sh
  make CFLAGS="-n -O -w" compile
  install -s -m 555 -o root -g bin sh $DIR/bin
)

(
  set -x
  cd $SRC/usr/ucb/ftp
  rm -f ftp
  make CFLAGS="-n -O -w" compile
  install -s -m 555 -o root -g bin ftp $DIR/bin
)

(
  set -x
  cd $SRC/etc/mklost+found
  rm -f mklost+found
  make CFLAGS="-n -O" compile
  install -c -m 555 -o root -g bin mklost+found.sh $DIR/etc/mklost+found
)

(
  set -x
  cd $SRC/etc/init
  rm -f init
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin init $DIR/etc
)

(
  set -x
  cd $SRC/etc/mount
  rm -f mount
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin mount $DIR/etc
  install -s -m 555 -o root -g bin umount $DIR/etc
)

(
  set -x
  cd $SRC/etc/ifconfig
  rm -f ifconfig
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin ifconfig $DIR/etc
  install -c -m 444 -o root -g bin services $DIR/etc
)

(
  set -x
  cd $SRC/etc/route
  rm -f route
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin route $DIR/etc
)

(
  set -x
  cd $SRC/etc/reboot
  rm -f reboot
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin reboot $DIR/etc
)

(
  set -x
  cd $SRC/etc/fsck
  rm -f fsck
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin fsck $DIR/etc
)

(
  set -x
  cd $SRC/etc/newfs
  rm -f newfs
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin newfs $DIR/etc
)

(
  set -x
  cd $SRC/etc/sony
  rm -f sony
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin sony $DIR/etc
)

(
  set -x
  cd $SRC/etc/newsys
  rm -f newsys
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin newsys $DIR/etc
)

(
  set -x
  cd $SRC/etc/mac2part
  rm -f mac2part
  make CFLAGS="-n -O" compile
  install -s -m 555 -o root -g bin mac2part $DIR/etc
)

(
  set -x
  cd $SRC/bin/machterm
  rm -f machterm
  make CFLAGS="-n -O -DRAMDISK_MACHTERM" compile
  install -s -m 555 -o root -g bin machterm $DIR/bin
)

(
  set -x
  cd $SRC/mach_servers/mach_init
  rm -f mach_init
  make CFLAGS="-n -O" compile
  install -s -m 544 -o root -g bin mach_init $DIR/mach_servers
)

(
  set -x
  cd $SRC/mach_servers/ux
  make compile
  install -c -o root -g bin -m 544 server/MACMACH/vmunix $DIR/mach_servers/startup
  install -c -o root -g bin -m 544 emulator/emulator $DIR/mach_servers/emulator
  rm -f /vmunix;  ln -s mach_servers/startup $DIR/vmunix
)

set +x

cat >$DIR/etc/passwd <<'@EOF@'
root::0:1:System Operator:/:/bin/csh
daemon:*:1:1::/:
sys:*:2:2::/:/bin/csh
bin:*:3:3::/bin:
news:*:6:6::/usr/spool/news:
sync::8:8::/:/bin/sync
xdm:*:0:1:X-D-M:/:/usr/bin/X11/xdm
guest:*:10:10:Guest User:/usr/guest:/bin/csh
uucp:*:46:4:Unix-to-Unix-Copy:/usr/spool/uucppublic:/usr/lib/uucp/uucico
games:*:4310:10:Games Administrator:/usr/games:/bin/csh
@EOF@

cat >$DIR/etc/group <<'@EOF@'
wheel:*:0:root
daemon:*:1:daemon,root
kmem:*:2:root
bin:*:3:root
tty:*:4:root
operator:*:5:root
news:*:6:root
staff:*:10:root
other:*:20:root
guest:*:31:root
@EOF@

echo "#! /bin/sh">$DIR/etc/rc
echo "">>$DIR/etc/rc
echo "RAMDISK_VERSION=\"$VERSION\"">>$DIR/etc/rc

cat >>$DIR/etc/rc <<'@EOF@'

HOME="/"; export HOME
PATH="/bin:/etc"; export PATH

[ -f /dev/console ] || reboot -h
exec </dev/console >/dev/console 2>&1

stty dec
eval "`machterm -sh`"

trap "echo \"interrupted...\"" 2

echo ""
echo "$RAMDISK_VERSION"
echo ""
ifconfig en0 down
newsys -install || {
  echo ""
  echo "Installation failed.  Type return to reboot."
  read foo
}

echo ""
echo "Rebooting..."
reboot
@EOF@

chown root.bin $DIR/etc/rc
chmod 555 $DIR/etc/rc
echo "Created ramdisk /etc/rc"

echo "# ramdisk /.profile" >$DIR/.profile
echo "echo \"\"" >>$DIR/.profile
echo "echo \"$VERSION\"" >>$DIR/.profile

cat >>$DIR/.profile <<'@EOF@'
echo ""
echo "Entering single user shell..."
echo ""
PATH=/bin:/etc:/mnt/bin:/mnt/etc:/mnt/usr/bin:/mnt/usr/ucb
export PATH
stty dec
eval "`machterm -sh`"
@EOF@

chown root.bin $DIR/.profile
chmod 444 $DIR/.profile
echo "Created ramdisk /.profile"

(
  set -x
  umount $DEVICE
  fsck $DEVICE
  cd /usr/src/mach_kernel
  make
  cp /usr/src/mach_kernel/kernel/MACMACH/mach_kernel /usr/tmp/mach_kernel.ramdisk
) <$XDEVICE

