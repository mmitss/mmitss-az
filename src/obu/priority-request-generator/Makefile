SDK_ROOT=../../../../sdk
MAKE_ROOT=$(SDK_ROOT)/make
include $(MAKE_ROOT)/header.mk



SRC  =  MMITSS_OBE_PriorityRequestGenerator_ASN.cpp  GeoCoord.cpp NMAP.cpp AccelerationSet4Way.cpp BasicVehicle.cpp Motion.cpp PositionLocal3D.cpp Brakes.cpp ReqEntry.cpp MAPEntry.cpp
TGT  = MMITSS_OBE_PriorityRequestGenerator_ASN
LIBS+= -lnetsnmp -lstdc++ -lm 


OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	@echo $(VAR)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
