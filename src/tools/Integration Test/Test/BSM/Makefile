# Sample Makefile for building with the uper codec and json libraries
BUILD_ROOT=../../../../..
MAKE_ROOT :=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/newheader.mk

INCLUDES    := -I$(J2735_INCL) -I$(JSONCPP_INCL) -I$(BUILD_ROOT)/include/common # tell the compiler where to find the j2735 and json include directories
LDFLAGS     := -Wl,-rpath=$(J2735_SO_DIR) # tell the linker to add a path to the json shared lib 
#-Wl is used to pass the option(rpath) to the linker --- we probably want to move this into a default rather than a hard coded location
LIBS        := -L$(J2735_SO_DIR) -L$(JSONCPP_LIB) -L$(BUILD_ROOT)/3rdparty/jsoncpp -L$(BUILD_ROOT)/lib -ldsrc -lasn -ljsoncpp1_8_4 -lmmitss-common  #tell the linker where to find the j2735 and json libraries

#cpp's go here
SRC += bsmSender.cpp
#the name of the executable goes here
#the name of the executable goes here
TARGET = bsm

# list the .o's corresponding to .cpp's
# see: https://www.gnu.org/software/make/manual/html_node/Text-Functions.html
OBJECTS = $(patsubst %.cpp, %.o, $(SRC))

include $(MAKE_ROOT)/newrules.mk

