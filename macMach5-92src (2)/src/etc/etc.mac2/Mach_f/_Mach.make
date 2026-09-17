# the control panel
cdev_main.c.o Ä cdev_main.c
	C -b cdev_main.c

# entry point for the INIT resource
INIT_main.c.o Ä INIT_main.c
	C -b INIT_main.c

# check that this Macintosh can run Mach
GestaltMach.c.o Ä GestaltMach.c
	C -b GestaltMach.c

# the startup dialog: INIT_dialog()
INIT_dialog.c.o Ä INIT_dialog.c
	C -mc68020 -b INIT_dialog.c

# the Mach kernel loader: LoadMach()
LoadMach.c.o Ä LoadMach.c
	C -mc68020 -b LoadMach.c
	
# decompressor
decompress.c.o Ä decompress.c
	C -mc68020 -b decompress.c

# Unix file system: UFSopen(), UFSseek() and UFSread()
ufs.c.o Ä ufs.c
	C -mc68020 -b ufs.c

# Macintosh file system: HFSopen(), HFSseek() and HFSread()
hfs.c.o Ä hfs.c
	C -mc68020 -b hfs.c

# scsi_disk_open() and scsi_disk_read()
scsi.c.o Ä scsi.c
	C -mc68020 -b scsi.c

# load serial port drivers: SerialPorts()
SerialPorts.c.o Ä SerialPorts.c
	C -b SerialPorts.c

# AlertMsg()
AlertMsg.c.o Ä AlertMsg.c
	C -b AlertMsg.c

# device functions
device.c.o Ä device.c
	C -b device.c

# standard string functions and getstr()
string.c.o Ä string.c
	C -mc68020 -b string.c

# NATIVE() checks to see if Mach is already running
native.a.o Ä native.a
	Asm native.a

# the data fork can be a mach_kernel (a.out format) that contains a ramdisk
×Mach.data Ä ×Mach.make
	if "" == "`exists -f mach_kernel`"
		echo >×Mach
	else
		duplicate mach_kernel ×Mach -y
	end

# build the various resources
# version.r -- the 'vers' resource
# AlertMsg.r -- the AlertMsg() messages
# cdev.r -- things to make a control panel
# conf.r -- the 'conf' resource
# cdev_dialog.r -- the control panel 'ditl'
# string.r -- various string constants
# sysz.r -- the 'sysz' resource
# INIT_dialog.r -- the startup 'ditl'
# icons.r -- the icons
# device.r -- the device menu
# TimeOffset.r -- the TimeOffset menu
×Mach.rsrc Ä ×Mach.make version.r AlertMsg.r cdev.r conf.r ¶
		cdev_dialog.r string.r sysz.r INIT_dialog.r ¶
		icons.r device.r TimeOffset.r
	Rez -o ×Mach -t cdev -c Mach -ov ¶
		version.r AlertMsg.r cdev.r conf.r cdev_dialog.r ¶
		string.r sysz.r INIT_dialog.r icons.r device.r TimeOffset.r
	setfile ×Mach -a B

# compile and load the INIT and cdev resources
×Mach ÄÄ ×Mach.make ×Mach.data ×Mach.rsrc ¶
		cdev_main.c.o INIT_main.c.o INIT_dialog.c.o LoadMach.c.o decompress.c.o ¶
		ufs.c.o hfs.c.o scsi.c.o SerialPorts.c.o AlertMsg.c.o ¶
		device.c.o GestaltMach.c.o string.c.o native.a.o
	Link -rt cdev=-4064 ¶
		cdev_main.c.o ¶
		device.c.o ¶
		"{Libraries}"Interface.o "{CLibraries}"CInterface.o ¶
		-o ×Mach
	Link -ra =16 -rt INIT=33 ¶
		INIT_main.c.o ¶
		INIT_dialog.c.o ¶
		LoadMach.c.o ¶
		decompress.c.o ¶
		ufs.c.o ¶
		hfs.c.o ¶
		scsi.c.o ¶
		SerialPorts.c.o ¶
		AlertMsg.c.o ¶
		device.c.o ¶
		GestaltMach.c.o ¶
		string.c.o ¶
		native.a.o ¶
		"{Libraries}"Interface.o "{CLibraries}"CInterface.o ¶
		-o ×Mach
