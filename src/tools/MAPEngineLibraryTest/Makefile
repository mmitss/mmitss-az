# Sample Makefile for building with the uper codec and json libraries
BUILD_ROOT=../../..
MAKE_ROOT :=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/newheader.mk

INCLUDES    := -I$(J2735_INCL) -I$(COMMON_INCL) # tell the compiler where to find the include directories
#LDFLAGS     := -Wl,-rpath=$(J2735_SO_DIR) # tell the linker to add a path to the json shared lib 
#-Wl is used to pass the option(rpath) to the linker --- we probably want to move this into a default rather than a hard coded location
LIBS        := -L$(J2735_LIB) -L$(MMITSS_COMMON_LIB) -ldsrc -lasn -lmmitss-common  #tell the linker where to find the libraries

#cpp's go here
SRC += testMEL.cpp GeoCoord.cpp locAware.cpp geoUtils.cpp
#the name of the executable goes here
TARGET = M_testMEL 

# list the .o's corresponding to .cpp's
# see: https://www.gnu.org/software/make/manual/html_node/Text-Functions.html
OBJECTS = $(patsubst %.cpp, %.o, $(SRC))

include $(MAKE_ROOT)/newrules.mk

