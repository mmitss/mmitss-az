BUILD_ROOT=../../..
MAKE_ROOT=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC += M_PriorityRequestServer.cpp ReqEntry.cpp  PriorityConfig.cpp IntLanePhase.cpp ManageReqList.cpp PositionLocal3D.cpp BasicVehicle.cpp  
TGT:=M_PriorityRequestServer
LIBS += -L$(BUILD_ROOT)/lib/
LIBS += -L$(BUILD_ROOT)/3rdparty/net-snmp/lib/
LIBS+= -lnetsnmp -lstdc++ -lj2735-linux
INCLUDES += -I$(BUILD_ROOT)/3rdparty/

OBJ = $(patsubst %.cpp, %.o, ${SRC})
include $(MAKE_ROOT)/rules.mk
$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)

