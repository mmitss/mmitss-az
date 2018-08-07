SDK_ROOT=../../../sdk
MAKE_ROOT=$(SDK_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC += MMITSS_MRP_TrafficControllerInterface.cpp Signal.cpp Array.cpp Config.cpp
TGT:=MMITSS_MRP_TrafficControllerInterface
LIBS+= -lnetsnmp -lstdc++ -lm

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
