
SOURCE.c += aic7xxx_mach.c

BUILD=PRODUCTION
OBJDIR=obj/${BUILD}

OBJECTS.c.o = $(patsubst %.c, ${OBJDIR}/%.c.o, ${SOURCE.c})
OBJECTS.o += ${OBJECTS.c.o}

OLDOBJDIR=/usr/src/DRglobegate/osfmk/obj/powermac/mach_kernel/PRODUCTION

CC = gcc
CC.options += -fno-builtin -fsigned-char -msoft-float -fsigned-bitfields -fomit-frame-pointer
CC.options += -mcpu=604
CC.options += -O2
CC.options += -Wtrigraphs -Wcomment -Wstrict-prototypes -Wmissing-prototypes -Wimplicit -Wreturn-type -Wpointer-arith
# CC.options += -pedantic
CC.options += -nostdinc
CC.depend += -MD
CC.defines += -Dvolatile=__volatile -DPOWERMAC -DMACH -DMACH_KERNEL -DMACH_LOAD -DSCSI_SEQUENTIAL_DISKS -D_ANSI_C_SOURCE
CC.includes += -I${OLDOBJDIR}
CC.includes += -I${OLDOBJDIR}/..
CC.includes += -I/usr/src/DRglobegate/osfmk/src/mach_kernel
CC.includes += -I${OLDOBJDIR}/../..
CC.includes += -I/usr/src/DRglobegate/osfmk/src
CC.includes += -I${OLDOBJDIR}/../COPYLEFT
CC.includes += -I/usr/src/DRglobegate/osfmk/src/mach_kernel/COPYLEFT
CC.includes += -I/usr/src/DRglobegate/osfmk/export/powermac/mach_kernel
CC.includes += -I/usr/src/DRglobegate/osfmk/export/powermac/include
CC.includes += -I/usr/src/DRglobegate/osfmk/export/powermac/include/sa_mach

all :: ${OBJECTS.o}

${OBJECTS.c.o} : ${OBJDIR}/%.c.o : %.c
	@ if [ ! -d $(@D) ] ; then mkdir -p $(@D) ; fi
	${CC} -c ${CC.options} ${CC.depend} ${CC.defines} ${CC.includes} $*.c -o $@~
	mv $@~ $@

tests :: types.t

types.t : types.t.c
	${CC} -o $@~ types.t.c -I../../..
	mv $@~ $@

