BUILD_ROOT=../../..
MAKE_ROOT=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC += M_PerformanceObserver.cpp AccelerationSet4Way.cpp BasicVehicle.cpp Brakes.cpp GeoCoord.cpp NMAP.cpp PositionLocal3D.cpp Array.cpp Signal.cpp Config.cpp
TGT:= M_PerformanceObserver

INCLUDES += -I$(BUILD_ROOT)/3rdparty/

LIBS += -L$(BUILD_ROOT)/3rdparty/net-snmp/lib/
LIBS += -lnetsnmp -lstdc++ -lm

//LIBS+= -lstdc++ -luClibc++ -leloop -lnetsnmp -lgps -lgpsapi

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
