BUILD_ROOT=../../..
MAKE_ROOT=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC += M_TrajectoryAware.cpp GeoCoord.cpp NMAP.cpp AccelerationSet4Way.cpp BasicVehicle.cpp Brakes.cpp PositionLocal3D.cpp Motion.cpp
TGT:=M_TrajectoryAware
LIBS += -L$(BUILD_ROOT)/3rdparty/net-snmp/lib/
LIBS += -lnetsnmp -lstdc++ -lm

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
