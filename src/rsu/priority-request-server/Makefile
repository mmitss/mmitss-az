BUILD_ROOT=../../..
MAKE_ROOT=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC += M_PriorityRequestServer.cpp ReqEntry.cpp IntLanePhase.cpp ManageRequestList.cpp PositionLocal3D.cpp BasicVehicle.cpp SignalRequest.cpp IntersectionAccessPoint.cpp ExpectedTimeOfArrival.cpp 
TGT:=M_PriorityRequestServer
LIBS += -L$(BUILD_ROOT)/lib/
LIBS += -L$(BUILD_ROOT)/3rdparty/net-snmp/lib/
LIBS += -L$(BUILD_ROOT)/3rdparty/jsoncpp/lib/
LIBS+= -lnetsnmp -lstdc++ -lj2735-linux -ljsoncpp1_8_4
INCLUDES += -I$(BUILD_ROOT)/3rdparty/
INCLUDES += -I$(BUILD_ROOT)/3rdparty/jsoncpp/include
INCLUDES += -I$(BUILD_ROOT)/3rdparty/asn1j2735/include
INCLUDES += -I$(BUILD_ROOT)/3rdparty/mapengine/include

OBJ = $(patsubst %.cpp, %.o, ${SRC})
include $(MAKE_ROOT)/rules.mk
$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)

