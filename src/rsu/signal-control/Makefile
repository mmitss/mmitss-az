MAKE_ROOT=../make
include $(MAKE_ROOT)/header.mk

SRC += M_Signal_Control.cpp Signal.cpp Array.cpp Config.cpp PositionLocal3D.cpp
TGT:=M_Signal_Control
LIBS+= -lnetsnmp -lstdc++ -lm

OBJ = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/rules.mk

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
