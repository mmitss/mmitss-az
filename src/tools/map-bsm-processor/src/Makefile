# PRG Makefile 
BUILD_ROOT= ../../../..
MAKE_ROOT :=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/newheader.mk

INCLUDES    := -I$(J2735_INCL) -I$(JSONCPP_INCL) -I$(COMMON_INCL) -I$(BUILD_ROOT)/3rdparty/ -I$(MAPENGINE_INCL)  # tell the compiler where to find theinclude directories
CPPFLAGS += -DLOGGING
#LDFLAGS     := -fPIC -Wl,-rpath=$(J2735_SO_DIR) # tell the linker to add a path to the j2735 shared lib 
#LDFLAGS     := -Wl,-rpath=$(J2735_SO_DIR) # tell the linker to add a path to the j2735 shared lib 
#-Wl is used to pass the option(rpath) to the linker --- we probably want to move this into a default rather than a hard coded location
LIBS        := -L$(J2735_LIB) -L$(MMITSS_COMMON_LIB) -L$(MAPENGINE_LIB) -L$(NETSNMP_LIB) -llocAware -ldsrc -lasn -lmmitss-common -lnetsnmp 
#cpp's go here
SRC += MapBsmProcessor.cpp MapEngine.cpp
#the name of the executable goes here
TARGET = MapBsmProcessor

# list the .o's corresponding to .cpp's
# see: https://www.gnu.org/software/make/manual/html_node/Text-Functions.html
OBJECTS = $(patsubst %.cpp, %.o, $(SRC))

include $(MAKE_ROOT)/newrules.mk


