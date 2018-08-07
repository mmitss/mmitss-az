MAKE_ROOT=../../../make
include $(MAKE_ROOT)/header.mk

SRC += M_PriorityRequestServer.cpp ReqEntry.cpp  PriorityConfig.cpp IntLanePhase.cpp ReqEntryListHandle.cpp AccelerationSet4Way.cpp PositionLocal3D.cpp BasicVehicle.cpp Brakes.cpp Motion.cpp
TGT:=M_PriorityRequestServer
LIBS+= -lnetsnmp -lstdc++ 
OBJ = $(patsubst %.cpp, %.o, ${SRC})
include $(MAKE_ROOT)/rules.mk
$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)

