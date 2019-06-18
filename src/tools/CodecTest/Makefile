# Makefile for 'tools' directory

# define 3rdparty directories
THIRDPARTY_ROOT := ../../../3rdparty
J2735_DIR     := $(THIRDPARTY_ROOT)/asn1j2735
JSONCPP_DIR   := $(THIRDPARTY_ROOT)/jsoncpp
JSONCPP_INCL  := $(JSONCPP_DIR)/include
J2735_INCL    := $(J2735_DIR)/include
JSONCPP_LIB   := $(JSONCPP_DIR)/lib
J2735_SO_DIR  := $(J2735_DIR)/lib

OBJ_DIR       := obj
LIB_DIR       := lib

# compiler and linker options
C++         := g++
LOCAL_INCL  := -I/usr/include -Iinclude
DEBUG_FLAGS := -O0 -g
C++FLAGS    := -Wall -Wextra -W -Wshadow -Wcast-qual -Wwrite-strings -Wconversion $(DEBUG_FLAGS) $(LOCAL_INCL)
ADD_INCL    := -I$(J2735_INCL) -I$(JSONCPP_INCL)
LINKLIBS    := -Wl,-rpath=../$(J2735_SO_DIR) -L$(J2735_SO_DIR) -L$(JSONCPP_LIB) -ldsrc -lasn -ljsoncpp1_8_4

# to be built
OBJS        := $(patsubst src/%.cpp,obj/%.o,$(wildcard src/*.cpp))
TARGET      := $(OBJ_DIR)/BSMDecoder $(OBJ_DIR)/BSMEncoder

# what to build in the form of ....
#target: prerequisites …
#(tab)  recipe         ...there must be a tab before the recipe or it will not work
# 
#the recipe can be a compiler call or a shell command

all: $(OBJ_DIR) $(OBJS) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: src/%.cpp
	$(C++) $(C++FLAGS) $(ADD_INCL) -c -o $@ $<

$(OBJ_DIR)/BSMEncoder: $(OBJ_DIR)/BSMEncoder.o
	$(C++) $(C++FLAGS) -o $(OBJ_DIR)/BSMEncoder $(OBJ_DIR)/BSMEncoder.o $(LINKLIBS) 
 
$(OBJ_DIR)/BSMDecoder: $(OBJ_DIR)/BSMDecoder.o
	$(C++) $(C++FLAGS) -o $(OBJ_DIR)/BSMDecoder $(OBJ_DIR)/BSMDecoder.o $(LINKLIBS)
  
clean:
	rm -f $(OBJS) $(TARGET)
