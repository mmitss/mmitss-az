INCLUDES =  
LDFLAGS  =  
LIBS     =  


# define 3rdparty directories
# ':=' vs '=" usage is explained in https://www.gnu.org/software/make/manual/html_node/Flavors.html#Flavors
THIRDPARTY_ROOT := $(BUILD_ROOT)/3rdparty
J2735_DIR       := $(THIRDPARTY_ROOT)/asn1j2735
JSONCPP_DIR     := $(THIRDPARTY_ROOT)/jsoncpp
JSONCPP_INCL    := $(JSONCPP_DIR)/include
J2735_INCL      := $(J2735_DIR)/include
JSONCPP_LIB     := $(JSONCPP_DIR)/lib
J2735_SO_DIR    := $(J2735_DIR)/lib


# compiler and linker options - these will need to be moved into a unique file in mmitss/make so all modules eventually use the same basic settings
CPP         := g++
DEBUG_FLAGS := -O0 -g #do not optimize, add info for gdb
CPPFLAGS    := -Wall -Wextra -W -Wshadow -Wcast-qual -Wwrite-strings -Wconversion $(DEBUG_FLAGS) # turn compiler warnings with -W options

