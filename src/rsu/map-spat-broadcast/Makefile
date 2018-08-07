MAKE_ROOT=../make
include $(MAKE_ROOT)/header.mk

SRC  = M_MapSpatBroadcast.cpp Signal.cpp SignalController.cpp NMAP.cpp GeoCoord.cpp
TGT  = M_MapSpatBroadcast
LIBS+= -lnetsnmp -lstdc++ -lm

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
