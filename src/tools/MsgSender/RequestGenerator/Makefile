# Sample Makefile for building with the uper codec and json libraries
BUILD_ROOT=../../../..
MAKE_ROOT :=$(BUILD_ROOT)/make
include $(MAKE_ROOT)/newheader.mk

INCLUDES    := -I$(BUILD_ROOT)/3rdparty/ -I$(COMMON_INCL) -I$(GLPK_INCL)  # tell the compiler where to find the include directories
#LDFLAGS     := -Wl,-rpath=$(J2735_SO_DIR) # tell the linker to add a path to the j2735 shared lib 
#-Wl is used to pass the option(rpath) to the linker --- we probably want to move this into a default rather than a hard coded location
LIBS        := -L$(MMITSS_COMMON_LIB) -L$(GLPK_LIB) -lmmitss-common -lglpk   #tell the linker where to find the libraries

#cpp's go here
SRC += requestGenerator.cpp
#the name of the executable goes here
#the name of the executable goes here
TARGET = M_requestGenerator

# list the .o's corresponding to .cpp's
# see: https://www.gnu.org/software/make/manual/html_node/Text-Functions.html
OBJECTS = $(patsubst %.cpp, %.o, $(SRC))

include $(MAKE_ROOT)/newrules.mk

