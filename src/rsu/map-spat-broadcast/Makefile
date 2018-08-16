BUILD_ROOT=../../..
MAKE_ROOT=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC  = M_MapSpatBroadcast.cpp Signal.cpp SignalController.cpp NMAP.cpp
TGT  = M_MapSpatBroadcast
LIBS += -L$(BUILD_ROOT)/lib/
LIBS += -L$(BUILD_ROOT)/3rdparty/net-snmp/lib/
LIBS+= -lnetsnmp -lstdc++ -lm -lj2735-linux
INCLUDES += -I$(BUILD_ROOT)/3rdparty/
 
OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
