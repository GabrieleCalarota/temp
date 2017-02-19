if HAVE_X86_64_LINUX_GNU_GNU_STUBS_32_H  
STDINT_PATH = /usr/include/stdint.h -I /usr/include/ -I /usr/include/gnu -I /usr/include/x86_64-linux-gnu/ -I /usr/include/x86_64-linux-gnu/gnu 
else
STDINT_PATH =  ../include/stdint.h
endif
AM_CPPFLAGS = -Wall -mcpu=arm7tdmi
CC = arm-none-eabi-gcc
CCLD = arm-none-eabi-ld
AM_LDFLAGS =  -T /usr/include/uarm/ldscripts/elf32ltsarm.h.uarmcore.x /usr/include/uarm/crtso.o /usr/include/uarm/libuarm.o

bin_PROGRAMS = kernel

kernel_SOURCES = p1test.c ../include/mikabooq.h ../include/const.h ../include/listx.h /usr/include/uarm/uARMtypes.h /usr/include/uarm/uARMconst.h /usr/include/uarm/libuarm.h mikabooq.c 
AM_CPPFLAGS = -I ../include -I /usr/include/uarm/ -include $(STDINT_PATH)
