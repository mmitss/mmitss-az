# Sample Makefile for building with the uper codec and json libraries
BUILD_ROOT=../..
MAKE_ROOT :=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/newheader.mk

INCLUDES    := -I$(J2735_INCL) -I$(COMMON_INCL)# tell the compiler where to find the include directories
CPPFLAGS	:= $(CPPFLAGS) -fPIC
LDFLAGS     := -shared 
#LDFLAGS     := -shared -Wl,-rpath=$(J2735_SO_DIR) # tell the linker to add a path to the json shared lib 
#-Wl is used to pass the option(rpath) to the linker --- we probably want to move this into a default rather than a hard coded location
LIBS        := -L$(J2735_LIB) -ldsrc -lasn #tell the linker where to find the libraries


#cpp's go here
SRC += AccelerationSet4Way.cpp Brakes.cpp GeoCoord.cpp Motion.cpp PriorityRequest.cpp BasicVehicle.cpp Position3D.cpp UdpSocket.cpp SignalRequest.cpp SignalStatus.cpp IntersectionAccessPoint.cpp ExpectedTimeOfArrival.cpp jsoncpp.cpp Timestamp.cpp

#the name of the executable goes here
TARGET = $(MMITSS_COMMON_LIB)/libmmitss-common.so


# list the .o's corresponding to .cpp's
# see: https://www.gnu.org/software/make/manual/html_node/Text-Functions.html
OBJECTS = $(patsubst %.cpp, %.o, $(SRC))

include $(MAKE_ROOT)/newrules.mk

