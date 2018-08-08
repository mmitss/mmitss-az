BUILD_ROOT=../../..
MAKE_ROOT=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC += M_TrafficControllerInterface.cpp Signal.cpp Array.cpp Config.cpp
TGT:=M_TrafficControllerInterface
LIBS += -L$(BUILD_ROOT)/3rdparty/net-snmp/lib/
LIBS += -lnetsnmp -lstdc++ -lm
INCLUDES += -I$(BUILD_ROOT)/3rdparty/

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
