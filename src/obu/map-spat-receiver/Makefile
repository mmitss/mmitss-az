SDK_ROOT=../../../../sdk
MAKE_ROOT=$(SDK_ROOT)/make
include $(MAKE_ROOT)/header.mk

SRC  = MMITSS_OBE_MAP_SPAT_Receiver_ASN.cpp NMAP.cpp GeoCoord.cpp
TGT  = MMITSS_OBE_MAP_SPAT_Receiver_ASN
LIBS+= -lstdc++ -lm -luClibc++ -lj2735-asd

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
