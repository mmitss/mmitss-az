SDK_ROOT=../../../../sdk
MAKE_ROOT=$(SDK_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC += MMITSS_MRP_PriorityRequestServer_ASN.cpp ReqEntry.cpp  PriorityConfig.cpp IntLanePhase.cpp ReqEntryListHandle.cpp AccelerationSet4Way.cpp PositionLocal3D.cpp BasicVehicle.cpp Brakes.cpp Motion.cpp
TGT:=MMITSS_MRP_PriorityRequestServer_ASN
LIBS+= -lnetsnmp -lstdc++ 
OBJ = $(patsubst %.cpp, %.o, ${SRC})
include $(MAKE_ROOT)/rules.mk
$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)

