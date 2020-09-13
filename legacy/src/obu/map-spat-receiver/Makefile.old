SRC =   
SRC += MMITSS_OBE_MAP_SPAT_Receiver_ASN.cpp NMAP.cpp GeoCoord.cpp

TGT:=MMITSS_OBE_MAP_SPAT_Receiver_ASN
TOOLCHAIN=../../../../sdk/asd/toolchain
MMITSS=../../../sdk/mmitss
CFLAGS:=-DPRINT_LOG -g
INCLUDES:=-I. -I$(MMITSS)/include/libj2735
LIBS+=-L$(MMITSS)/lib

all:TOOLCHAIN=../../../sdk/rse/toolchain
all:CC=$(TOOLCHAIN)/bin/i386-linux-uclibc-g++
all:LD=$(TOOLCHAIN)/bin/i386-linux-uclibc-ld
all:INCLUDES+=-I$(TOOLCHAIN)/include -I$(TOOLCHAIN)/usr/include
all:LIBS+=-L$(TOOLCHAIN)/lib -L$(TOOLCHAIN)/usr/lib -lstdc++ -lm -luClibc++ -lj2735-asd

linux:TOOLCHAIN=
linux:CC=g++
linux:LD=ld
linux:LIBS+=-lj2735-linux -lnetsnmp -lstdc++ 


LIBS +=  -lm
OBJ = $(patsubst %.cpp, %.o, ${SRC})

all:	$(TGT)
linux:  $(TGT)

$(TGT): $(OBJ)
	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)

%.o: %.cpp
	${CC} ${CFLAGS} $(INCLUDES) $(CFLAGS) -c -o $@ $<

clean:
	-rm -f $(TGT) $(OBJ) *~ *.o
