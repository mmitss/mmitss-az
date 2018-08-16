BUILD_ROOT=../../..
MAKE_ROOT=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC  =  M_PriorityRequestGenerator.cpp NMAP.cpp BasicVehicle.cpp PositionLocal3D.cpp ReqEntry.cpp MAPEntry.cpp
TGT  = M_PriorityRequestGenerator
LIBS += -L$(BUILD_ROOT)/lib/
LIBS += -L$(BUILD_ROOT)/3rdparty/net-snmp/lib/
LIBS+= -lnetsnmp -lstdc++ -lm -lj2735-linux

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	@echo $(VAR)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
