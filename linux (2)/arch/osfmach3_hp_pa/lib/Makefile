# Copyright (c) Open Software Foundation, Inc.   
# 
# 
# pmk1.1



L_TARGET = lib.o
L_OBJS  = checksum.o

.S.o:
	$(CC) -DASSEMBLER -traditional -c $< -o $*.o

${L_TARGET}: $(L_OBJS)
	$(LD) -r -o ${L_TARGET} $(L_OBJS)

fastdep:

