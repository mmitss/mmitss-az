BUILD_ROOT=../../..
MAKE_ROOT=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC  =  M_PriorityRequestGenerator.cpp  GeoCoord.cpp NMAP.cpp AccelerationSet4Way.cpp BasicVehicle.cpp Motion.cpp PositionLocal3D.cpp Brakes.cpp ReqEntry.cpp MAPEntry.cpp
TGT  = M_PriorityRequestGenerator
LIBS += -L$(BUILD_ROOT)/3rdparty/net-snmp/lib/
LIBS+= -lnetsnmp -lstdc++ -lm 

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	@echo $(VAR)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
