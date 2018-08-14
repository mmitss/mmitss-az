BUILD_ROOT=../../..
MAKE_ROOT=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC += M_PrioritySolver.cpp Array.cpp Config.cpp ReqEntry.cpp Signal.cpp PositionLocal3D.cpp GetInfo.cpp EVLS.cpp
TGT:=M_PrioritySolver
LIBS += -L$(BUILD_ROOT)/3rdparty/net-snmp/lib/
LIBS += -L$(BUILD_ROOT)/3rdparty/glpk/lib/
LIBS += -lnetsnmp -lstdc++ -lglpk -lm
INCLUDES += -I$(BUILD_ROOT)/3rdparty/
INCLUDES += -I$(BUILD_ROOT)/3rdparty/glpk/include

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
