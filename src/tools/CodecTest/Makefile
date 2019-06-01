# Makefile for 'tools' directory

THIRDPARTY_ROOT := ../../../3rdparty
J2735_DIR     := $(THIRDPARTY_ROOT)/asn1j2735
JSONCPP_DIR   := $(THIRDPARTY_ROOT)/jsoncpp
JSONCPP_LIB   := $(JSONCPP_DIR)/lib
J2735_SO_DIR  := $(J2735_DIR)/lib

OBJ_DIR       := obj
LIB_DIR       := lib

# compiler flags
C++        := g++
LOCAL_INCL := -I/usr/include -Iinclude
CFLAGS     := -Wall -W -Wshadow -Wcast-qual -Wwrite-strings -O3 -fno-strict-aliasing -fPIC $(LOCAL_INCL)
C++FLAGS   := -std=c++11 -Wconversion $(CFLAGS)

OBJS   := $(patsubst src/%.cpp,obj/%.o,$(wildcard src/*.cpp))
TARGET := $(OBJ_DIR)/BSMDecoder $(OBJ_DIR)/BSMEncoder
ADDINC := -I$(J2735_DIR)/include -I$(JSONCPP_DIR)/include
LINKLIBS := -Wl,-rpath=../$(J2735_SO_DIR) -L$(J2735_SO_DIR) -L$(JSONCPP_LIB) -ldsrc -lasn -ljsoncpp

all: $(OBJ_DIR) $(OBJS) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: src/%.cpp
	$(C++) $(C++FLAGS) $(ADDINC) -c -o $@ $<

$(OBJ_DIR)/BSMEncoder: $(OBJ_DIR)/BSMEncoder.o
	$(C++) $(C++FLAGS) -o $(OBJ_DIR)/BSMEncoder $(OBJ_DIR)/BSMEncoder.o $(LINKLIBS) 
 
$(OBJ_DIR)/BSMDecoder: $(OBJ_DIR)/BSMDecoder.o
	$(C++) $(C++FLAGS) -o $(OBJ_DIR)/BSMDecoder $(OBJ_DIR)/BSMDecoder.o $(LINKLIBS)  
clean:
	rm -f $(OBJS) $(TARGET)
