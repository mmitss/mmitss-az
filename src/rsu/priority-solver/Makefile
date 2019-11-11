# Solver Makefile
BUILD_ROOT=../../..
MAKE_ROOT :=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/newheader.mk


INCLUDES    := -I$(BUILD_ROOT)/3rdparty/ -I$(COMMON_INCL) -I$(NETSNMP_INCL) -I$(GLPK_INCL)  # tell the compiler where to find the include directories
#LDFLAGS     := -Wl,-rpath=$(J2735_SO_DIR) # tell the linker to add a path to the j2735 shared lib 
#-Wl is used to pass the option(rpath) to the linker --- we probably want to move this into a default rather than a hard coded location
LIBS        := -L$(MMITSS_COMMON_LIB) -L$(GLPK_LIB) -L$(NETSNMP_LIB) -lmmitss-common -lglpk -lnetsnmp   #tell the linker where to find the libraries

#cpp's go here
SRC += M_PrioritySolver.cpp Array.cpp Config.cpp ReqEntry.cpp Signal.cpp PositionLocal3D.cpp GetInfo.cpp EVLS.cpp

#the name of the executable goes here
TARGET = M_PrioritySolver

# list the .o's corresponding to .cpp's
# see: https://www.gnu.org/software/make/manual/html_node/Text-Functions.html
OBJECTS = $(patsubst %.cpp, %.o, $(SRC))

include $(MAKE_ROOT)/newrules.mk



#BUILD_ROOT=../../..
#MAKE_ROOT=$(BUILD_ROOT)/make
#include $(MAKE_ROOT)/header.mk

#SRC += M_PrioritySolver.cpp Array.cpp Config.cpp ReqEntry.cpp Signal.cpp PositionLocal3D.cpp GetInfo.cpp EVLS.cpp
#TGT:=M_PrioritySolver
#LIBS += -L$(BUILD_ROOT)/3rdparty/net-snmp/lib/
#LIBS += -L$(BUILD_ROOT)/3rdparty/glpk/lib/
#LIBS += -lstdc++ -lglpk -lnetsnmp -lm
#INCLUDES += -I$(BUILD_ROOT)/3rdparty/
#INCLUDES += -I$(BUILD_ROOT)/3rdparty/glpk/include

#OBJ = $(patsubst %.cpp, %.o, ${SRC})

#include $(MAKE_ROOT)/rules.mk

#$(TGT): $(OBJ)
#	${CC} ${CFLAGS} ${INCLUDES} $(LDFLAGS) $(OBJ) -o $(TGT) $(LIBS)
