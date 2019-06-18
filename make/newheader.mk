#this file contains the common definitions to be used in module specific makefiles

#paths are initally null and filled in by module makefiles
INCLUDES =  
LDFLAGS  =  
LIBS     =  


# define 3rdparty directories
# ':=' vs '=" usage is explained in https://www.gnu.org/software/make/manual/html_node/Flavors.html#Flavors
THIRDPARTY_ROOT := $(BUILD_ROOT)/3rdparty
J2735_DIR       := $(THIRDPARTY_ROOT)/asn1j2735
JSONCPP_DIR     := $(THIRDPARTY_ROOT)/jsoncpp
MAPENGINE_DIR   := $(THIRDPARTY_ROOT)/mapengine
NETSNMP_DIR     := $(THIRDPARTY_ROOT)/net-snmp
GLPK_DIR        := $(THIRDPARTY_ROOT)/glpk
JSONCPP_INCL    := $(JSONCPP_DIR)/include
J2735_INCL      := $(J2735_DIR)/include
MAPENGINE_INCL  := $(MAPENGINE_DIR)/include
NETSNMP_INCL    := $(NETSNMP_DIR)
GLPK_INCL       := $(GLPK_DIR)/include
JSONCPP_LIB     := $(JSONCPP_DIR)/lib
J2735_SO_DIR    := $(J2735_DIR)/lib
MAPENGINE_LIB  := $(MAPENGINE_DIR)/lib
NETSNMP_LIB    := $(NETSNMP_DIR)/lib
GLPK_LIB       := $(GLPK_DIR)/lib


# compiler and linker options 
CPP         := g++
DEBUG_FLAGS := -O0 -g #do not optimize, add info for gdb
CPPFLAGS    := -Wall -Wextra -W -Wshadow -Wcast-qual -Wwrite-strings -Wconversion $(DEBUG_FLAGS) # turn on compiler warnings with -W options

