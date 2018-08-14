SRC = 
LIB = 
OBJ = 

CFLAGS += -Wall -DPRINT_LOG -no-pie
INCLUDES = -I. -I$(BUILD_ROOT)/include/libj2735 -I$(BUILD_ROOT)/include/common
LIBS = -L$(BUILD_ROOT)/lib/ -L$(BUILD_ROOT)/lib/

.PHONY: all linux clean

linux: CFLAGS+= -Dsimulation
linux: CC=gcc
linux: LD=ld
linux: AR=ar
linux: RANLIB=ranlib
linux: DEVICE=linux
linux: LIBS+=-lj2735-linux
linux: LIBS+=-lmmitss-common
