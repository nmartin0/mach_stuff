#! /bin/sh

# This shell script makes an installation/maintenance ramdisk.

# Once this is done, build a new mach_kernel.  Use this mach_kernel when
# building the Mach control panel (with MPW).

VERSION="MacMach Ramdisk Version 2.0"

# The size just happens to be the size of a high-density floppy...
# Note that this must match the "mac2ramdisk" entry in /etc/disktab!
SIZE=2880

DEVICE=/dev/ramdisk0a
RDEVICE=/dev/rramdisk0a
DIR=/tmp/ramdisk
SRC="`pwd`/../.."

set -x

umount $DEVICE
(
  dd if=/mach_kernel of=$RDEVICE seek=$SIZE bs=512 count=1
  newfs $RDEVICE mac2ramdisk
  [ -d $DIR ] || mkdir $DIR
  mount $DEVICE $DIR
) <$RDEVICE

(
  cd $DIR
  mkdir mach_servers mnt disk bin etc dev
  chmod 755 mach_servers mnt disk bin etc dev
  chown root.bin mach_servers mnt disk bin etc dev
  ln -s /mnt/tmp tmp
  ln -s /mnt/usr usr

)

sh $SRC/etc/etc.mac2/makedev/MAKEDEV.sh $DIR/dev
( cd $DIR/dev
  rm keyboard mouse
  rm ptyp5 ptyp6 ptyp7 ptyp8 ptyp9 ptypa ptypb ptypc ptypd ptype ptypf
  rm rsd0b rsd0d rsd0e rsd0f rsd1b rsd1d rsd1e rsd1f rsd2b rsd2d rsd2e rsd2f
  rm rsd3b rsd3d rsd3e rsd3f rsd4b rsd4d rsd4e rsd4f rsd5b rsd5d rsd5e rsd5f
  rm rsd6b rsd6d rsd6e rsd6f
  rm sd0b sd0d sd0e sd0f sd1b sd1d sd1e sd1f sd2b sd2d sd2e sd2f
  rm sd3b sd3d sd3e sd3f sd4b sd4d sd4e sd4f sd5b sd5d sd5e sd5f
  rm sd6b sd6d sd6e sd6f
  rm ttyp5 ttyp6 ttyp7 ttyp8 ttyp9 ttypa ttypb ttypc ttypd ttype ttypf

)

(
  cd $SRC/bin/true
  install -c -m 555 -o root -g bin true.sh $DIR/bin/true
)

(
  cd $SRC/bin/false
  install -c -m 555 -o root -g bin false.sh $DIR/bin/false
)

(
  cd $SRC/bin/cp
  cc -n -O cp.c -o cp
  install -s -m 555 -o root -g bin cp $DIR/bin
)

(
  cd $SRC/bin/ls
  cc -n -O ls.c -o ls
  install -s -m 555 -o root -g bin ls $DIR/bin
)

(
  cd $SRC/bin/dd
  cc -n -O dd.c -o dd
  install -s -m 555 -o root -g bin dd $DIR/bin
)

(
  cd $SRC/bin/pwd
  cc -n -O pwd.c -o pwd
  install -s -m 555 -o root -g bin pwd $DIR/bin
)

(
  cd $SRC/bin/cmp
  cc -n -O cmp.c -o cmp
  install -s -m 555 -o root -g bin cmp $DIR/bin
)

(
  cd $SRC/bin/ln
  cc -n -O ln.c -o ln
  install -s -m 555 -o root -g bin ln $DIR/bin
)

(
  cd $SRC/bin/rm
  cc -n -O rm.c -o rm
  install -s -m 555 -o root -g bin rm $DIR/bin
)

(
  cd $SRC/bin/mkdir
  cc -n -O mkdir.c -o mkdir
  install -s -m 555 -o root -g bin mkdir $DIR/bin
)

(
  cd $SRC/bin/cat
  cc -n -O cat.c -o cat
  install -s -m 555 -o root -g bin cat $DIR/bin
)

(
  cd $SRC/bin/sync
  cc -n -O sync.c -o sync
  install -s -m 555 -o root -g bin sync $DIR/bin
)

(
  cd $SRC/bin/stty
  cc -n -O stty.c -o stty
  install -s -m 555 -o root -g bin stty $DIR/bin
)

(
  cd $SRC/bin/echo
  cc -n -O echo.c -o echo
  install -s -m 555 -o root -g bin echo $DIR/bin
)

(
  cd $SRC/bin/chmod
  cc -n -O chmod.c -o chmod
  install -s -m 555 -o root -g bin chmod $DIR/bin
)

(
  cd $SRC/bin/ed
  cc -n -O ed.c -o ed
  install -s -m 555 -o root -g bin ed $DIR/bin
)

(
  cd $SRC/bin/df
  cc -n -O df.c -o df
  install -s -m 555 -o root -g bin df $DIR/bin
)

(
  cd $SRC/bin/test
  cc -n -O test.c -o test
  install -s -m 555 -o root -g bin test $DIR/bin
  rm -f $DIR/bin/[
  ln $DIR/bin/test $DIR/bin/[
)

(
  cd $SRC/bin/expr
  make expr.c
  cc -n -O expr.c -o expr
  install -s -m 555 -o root -g bin expr $DIR/bin
  rm expr.c
)

(
  cd $SRC/bin/sh
  make clean; make CFLAGS="-n -O -w" sh
  install -s -m 555 -o root -g bin sh $DIR/bin
  make clean
)

(
  cd $SRC/usr/ucb/ftp
  make clean; make CFLAGS="-n -O -w" ftp
  install -s -m 555 -o root -g bin ftp $DIR/bin
  make clean
)

(
  cd $SRC/etc/mklost+found
  install -c -m 555 -o root -g bin mklost+found.sh $DIR/etc/mklost+found
)

(
  cd $SRC/etc/init
  cc -n -O init.c -o init
  install -s -m 555 -o root -g bin init $DIR/etc
)

(
  cd $SRC/etc/mknod
  cc -n -O mknod.c -o mknod
  install -s -m 555 -o root -g bin mknod $DIR/etc
)

(
  cd $SRC/etc/mount
  cc -n -O mount.c -o mount
  install -s -m 555 -o root -g bin mount $DIR/etc
  cc -n -O umount.c -o umount
  install -s -m 555 -o root -g bin umount $DIR/etc
)

(
  cd $SRC/etc/ifconfig
  cc -n -O ifconfig.c -o ifconfig
  install -s -m 555 -o root -g bin ifconfig $DIR/etc
  install -c -m 444 -o root -g bin services $DIR/etc
)

(
  cd $SRC/etc/route
  cc -n -O route.c -o route
  install -s -m 555 -o root -g bin route $DIR/etc
)

(
  cd $SRC/etc/reboot
  cc -n -O reboot.c -o reboot
  install -s -m 555 -o root -g bin reboot $DIR/etc
)

(
  cd $SRC/etc/fsck
  make clean; make CFLAGS="-n -O" fsck
  install -s -m 555 -o root -g bin fsck $DIR/etc
  make clean
)

(
  cd $SRC/etc/newfs
  make clean; make CFLAGS="-n -O" newfs
  install -s -m 555 -o root -g bin newfs $DIR/etc
  make clean
)

(
  cd $SRC/etc/etc.mac2/sony
  cc -n -O sony.c -o sony
  install -s -m 555 -o root -g bin sony $DIR/etc
)

(
  cd $SRC/etc/etc.mac2/newsys
  install -c -m 555 -o root -g bin newsys.sh $DIR/etc/newsys
)

(
  cd $SRC/etc/etc.mac2/mac2part
  cc -n -O mac2part.c -o mac2part
  install -s -m 555 -o root -g bin mac2part $DIR/etc
)

(
  cd $SRC/etc/machterm
  cc -n -O main.c machterm.c -o machterm -ltermcap
  install -s -m 555 -o root -g bin machterm $DIR/etc
)

(
  cd $SRC/etc/getpass
  cc -n -O getpass.c -o getpass
  install -s -m 555 -o root -g bin getpass $DIR/etc
)

(
  cd $SRC/mach_servers/mach_init
  cc -n -O main.c service.c -o mach_init -lmach -lthreads
  install -s -m 544 -o root -g bin mach_init $DIR/mach_servers
)

(
  cd $SRC/mach_servers/ux
  make
  make DESTDIR=$DIR install
)

set +x

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
newsys || {
  echo ""
  echo "Installation/update failed.  Type return to reboot."
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


