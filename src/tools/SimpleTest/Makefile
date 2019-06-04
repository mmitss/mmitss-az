# Sample Makefile for building with the uper codec and json libraries
BUILD_ROOT=../../..
MAKE_ROOT :=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/newheader.mk

INCLUDES    := -I$(J2735_INCL) -I$(JSONCPP_INCL) # tell the compiler where to find the j2735 and json include directories
LDFLAGS     := -Wl,-rpath=$(J2735_SO_DIR) # tell the linker to add a path to the json shared lib --- we probably want to move this into a default rather than a hard coded location
LIBS        := -L$(J2735_SO_DIR) -L$(JSONCPP_LIB) -ldsrc -lasn -ljsoncpp1_8_4 #tell the linker where to find the j2735 and json libraries

#cpp's go here
SRC += BSMEncoder.cpp
#the name of the executable goes here
TARGET = BSMEncoder

# list the .o's corresponding to .cpp's
OBJECTS = $(patsubst %.cpp, %.o, ${SRC})

include $(MAKE_ROOT)/newrules.mk

#the executable depends on the .o's so we link
$(TARGET): $(OBJECTS)
	$(CPP) -o $(TARGET) $(OBJECTS) $(LDFLAGS) $(LIBS)
