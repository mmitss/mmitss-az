SDK_ROOT=../../../../sdk
MAKE_ROOT=$(SDK_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC += MPR_Priority_Solver.cpp Array.cpp Config.cpp ReqEntry.cpp Signal.cpp PriorityRequest.cpp GetInfo.cpp EVLS.cpp
TGT:=MMITSS_MRP_Priority_Solver

LIBS+= -lnetsnmp -lstdc++ -lglpk -lm

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
