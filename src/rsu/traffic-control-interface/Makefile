MAKE_ROOT=../../../make
include $(MAKE_ROOT)/header.mk

SRC += M_TrafficControllerInterface.cpp Signal.cpp Array.cpp Config.cpp
TGT:=M_TrafficControllerInterface
THIRDPARTYLIBS+= -lnetsnmp -lstdc++ -lm

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
