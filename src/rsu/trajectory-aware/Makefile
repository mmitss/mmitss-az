BUILD_ROOT=../../..
MAKE_ROOT=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC += M_TrajectoryAware.cpp NMAP.cpp BasicVehicle.cpp PositionLocal3D.cpp
TGT:=M_TrajectoryAware
LIBS += -L$(BUILD_ROOT)/lib/
LIBS += -lstdc++ -lm -lj2735-linux

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
