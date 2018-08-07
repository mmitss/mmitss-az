SRC = 
LIB = 
OBJ = 

CFLAGS += -Wall -DPRINT_LOG
INCLUDES = -I. -I../include/libj2735
LIBS = -L../lib/

.PHONY: all linux clean

linux: CFLAGS+= -Dsimulation
linux: CC=gcc
linux: LD=ld
linux: AR=ar
linux: RANLIB=ranlib
linux: DEVICE=linux
linux: LIBS+=-lj2735-linux

