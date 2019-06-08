# Makefile for 'tools' directory

BUILD_ROOT       := ../../../3rdparty
J2735_DIR     := $(BUILD_ROOT)/asn1j2735
MAPENGINE_DIR := $(BUILD_ROOT)/mapengine
J2735_SO_DIR    := $(J2735_DIR)/lib
MAPENGINE_SO_DIR := $(MAPENGINE_DIR)/lib
#EXEC_DIR  := $(BUILD_ROOT)/bin

OBJ_DIR       := obj
LIB_DIR       := lib

# compiler flags
C++       := g++
LOCAL_INCL    := -I/usr/include -Iinclude
CFLAGS    := -Wall -W -Wshadow -Wcast-qual -Wwrite-strings -O3 -fno-strict-aliasing -fPIC $(LOCAL_INCL)
C++FLAGS  := -std=c++11 -Wconversion $(CFLAGS)

OBJS   := $(patsubst src/%.cpp,obj/%.o,$(wildcard src/*.cpp))
TARGET := $(OBJ_DIR)/testDecoder $(OBJ_DIR)/testMapData
ADDINC := -I$(J2735_DIR)/include -I$(MAPENGINE_DIR)/include
LINKSO := -Wl,-rpath=../$(J2735_SO_DIR):../$(MAPENGINE_SO_DIR) -L$(J2735_SO_DIR) -L$(MAPENGINE_SO_DIR) -llocAware -ldsrc -lasn

all: $(OBJ_DIR) $(OBJS) $(TARGET)

$(OBJ_DIR):
	mkdir -p $(OBJ_DIR)

$(OBJ_DIR)/%.o: src/%.cpp
	$(C++) $(C++FLAGS) $(ADDINC) -c -o $@ $<

$(OBJ_DIR)/testDecoder: $(OBJ_DIR)/testDecoder.o
	$(C++) $(C++FLAGS) -o $(OBJ_DIR)/testDecoder $(OBJ_DIR)/testDecoder.o $(LINKSO)

$(OBJ_DIR)/testMapData: $(OBJ_DIR)/testMapData.o
	$(C++) $(C++FLAGS) -o $(OBJ_DIR)/testMapData $(OBJ_DIR)/testMapData.o $(LINKSO)

install:
	(mkdir -p $(EXEC_DIR))
	(cp $(TARGET) $(EXEC_DIR))
	(cd $(EXEC_DIR); chmod +x *)

clean:
	rm -f $(OBJS) $(TARGET)
