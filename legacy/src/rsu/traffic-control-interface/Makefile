# Traffic Control Interface Makefile
BUILD_ROOT=../../..
MAKE_ROOT :=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/newheader.mk


INCLUDES    := -I. -I$(COMMON_INCL) -I$(BUILD_ROOT)/3rdparty/ -I$(NETSNMP_INCL)   # tell the compiler where to find the j2735 and json include directories
LDFLAGS     := -Wl,-rpath=$(J2735_SO_DIR) # tell the linker to add a path to the j2735 shared lib 
#-Wl is used to pass the option(rpath) to the linker --- we probably want to move this into a default rather than a hard coded location
LIBS        := -L$(MMITSS_COMMON_LIB) -L$(NETSNMP_LIB)  -lnetsnmp -lmmitss-common -lstdc++ -lm  #tell the linker where to find the libraries

#cpp's go here
SRC += M_TrafficControllerInterface.cpp Signal.cpp Array.cpp Config.cpp

#the name of the executable goes here
TARGET = M_TrafficControllerInterface

# list the .o's corresponding to .cpp's
# see: https://www.gnu.org/software/make/manual/html_node/Text-Functions.html
OBJECTS = $(patsubst %.cpp, %.o, $(SRC))

include $(MAKE_ROOT)/newrules.mk




#BUILD_ROOT=../../..
#MAKE_ROOT=$(BUILD_ROOT)/make
#include $(MAKE_ROOT)/header.mk

#SRC += M_TrafficControllerInterface.cpp Signal.cpp Array.cpp Config.cpp
#TGT:=M_TrafficControllerInterface
#LIBS += -L$(BUILD_ROOT)/3rdparty/net-snmp/lib/
#LIBS += -lnetsnmp -lstdc++ -lm
#INCLUDES += -I$(BUILD_ROOT)/3rdparty/

#OBJ = $(patsubst %.cpp, %.o, ${SRC})

#include $(MAKE_ROOT)/rules.mk

#$(TGT): $(OBJ)
#	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
