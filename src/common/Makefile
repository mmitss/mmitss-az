# MMITSS-common.so Makefile

BUILD_ROOT=../..
INCLUDES = -I. -I$(BUILD_ROOT)/include/common
CC = gcc # C compiler
CFLAGS = -fPIC -Wall -Wextra -O2 -g # C flags
LDFLAGS = -shared  # linking flags
TGT = $(BUILD_ROOT)/lib/libmmitss-common.so
SRC = AccelerationSet4Way.cpp Brakes.cpp GeoCoord.cpp Motion.cpp PriorityRequest.cpp
OBJ = $(patsubst %.cpp, %.o, ${SRC})

$(TGT) : $(OBJ)
	$(CC) ${LDFLAGS} ${INCLUDES} -o $@ $^

%.o: %.c
	${CC} ${CFLAGS} ${INCLUDES} -c -o $@ $<

%.o: %.cpp
	${CC} $(INCLUDES) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f $(TGT) $(LIB_NAME)*.a $(OBJ) *~ *.o

