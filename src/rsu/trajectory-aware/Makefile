SDK_ROOT=../../../sdk
MAKE_ROOT=$(SDK_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC += MMITSS_MRP_EquippedVehicleTrajectoryAware_ASN.cpp GeoCoord.cpp NMAP.cpp AccelerationSet4Way.cpp BasicVehicle.cpp Brakes.cpp PositionLocal3D.cpp Motion.cpp
TGT:=MMITSS_MRP_EquippedVehicleTrajectoryAware_ASN
LIBS+= -lnetsnmp -lstdc++ -lm

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
