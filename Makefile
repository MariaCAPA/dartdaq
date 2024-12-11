#####################################################################
#
#  Name:         Makefile
#  Created by:   Maria 260619
#
#####################################################################

# get OS type from shell
OSTYPE = $(shell uname)

#####################################################################
# Nothing needs to be modified after this line 
#####################################################################

#-----------------------
# Common flags
#
CC = gcc $(USERFLAGS)
CXX = g++ $(USERFLAGS)
CFLAGS = -O2 -Wall -Wno-strict-aliasing -Wuninitialized -I$(INC_DIR)  -I$(INC_CAEN)  -I$(INC_ROOT) -DLINUX
#CFLAGS = -g -Wall -Wno-strict-aliasing -Wuninitialized -I$(INC_DIR)  -I$(INC_CAEN)  -I$(INC_ROOT) -DLINUX


#-----------------------
# This is for Linux
#
ifeq ($(OSTYPE),Linux)
OSTYPE = linux
endif

ifeq ($(OSTYPE),linux)

# >2GB file support
CFLAGS += -D_LARGEFILE64_SOURCE

OS_DIR = linux
OSFLAGS += -DOS_LINUX -fPIC -Wno-unused-function
LIBS = -lutil -lpthread -lrt -ldl -lrt

# add OpenSSL
ifndef NO_SSL
SSL_LIBS += -lssl -lcrypto
endif

endif

ifndef OS_DIR
OS_DIR = unknown
endif

#####################################################################
# end of conditional code
#####################################################################

#
# Midas directories
#
MIDAS=$(MIDASSYS)
INC_DIR  = $(MIDAS)/include
SRC_DIR  = $(MIDAS)/src
LIB_DIR   = $(MIDAS)/lib

#
#CAEN LIBRARY
#LIB_CAEN = -lCAENVME -lCAENDigitizer -lCAENComm # en /usr/lib
#INC_CAEN = /usr/include # /home/daquser/CAEN/CAENVMELib-2.50/include
LIB_CAEN = -lCAEN_FELib



#
# targets
#

PROGS_DIR = .
PROGS = $(PROGS_DIR)/fe2730Th $(PROGS_DIR)/enableTrigger $(PROGS_DIR)/disableTrigger

LIBNAME = $(LIB_DIR)/libmidas.a
LIB     = $(LIBNAME)

ALL:=
ALL+= $(LIB_DIR)/mfe.o 
ALL+= $(PROGS)

all: $(ALL)

#####################################################################

#
# create library and binary directories
#


#
# main binaries
#

ifndef NO_ROOT
HAVE_ROOT := $(shell root-config --version 2> /dev/null)
endif


ifdef HAVE_ROOT
ROOTLIBS    := $(shell root-config --libs)
ROOTGLIBS   := $(shell root-config --glibs)
ROOTCFLAGS  := $(shell root-config --cflags)
INC_ROOT  := $(shell  $(ROOTSYS)/bin/root-config --incdir)
ROOTCFLAGS  += -DHAVE_ROOT
endif


#$(PROGS_DIR)/%:$(SRC_DIR)/%.cxx
	#$(CXX) $(CFLAGS) $(OSFLAGS) -o $@ $< $(LIB) $(LIBS)

CFLAGS      += -DHAVE_MONGOOSE6
CFLAGS      += -DMG_ENABLE_THREADS
CFLAGS      += -DMG_DISABLE_CGI
ifndef NO_SSL
CFLAGS      += -DMG_ENABLE_SSL
endif

$(PROGS): $(LIBNAME)

#
# utilities
#
$(PROGS_DIR)/fe2730Th: $(PROGS_DIR)/fe2730Th.cxx $(LIB_DIR)/mfe.o
	$(CXX) $(CFLAGS) $(OSFLAGS) -o $@ $^ $(LIB) $(LIBS) $(LIB_CAEN) $(ROOTLIBS)

$(PROGS_DIR)/enableTrigger: $(PROGS_DIR)/enableTrigger.cxx 
	$(CXX) $(CFLAGS) $(OSFLAGS) -o $@ $^ $(LIB) $(LIBS) $(LIB_CAEN) $(ROOTLIBS)

$(PROGS_DIR)/disableTrigger: $(PROGS_DIR)/disableTrigger.cxx 
	$(CXX) $(CFLAGS) $(OSFLAGS) -o $@ $^ $(LIB) $(LIBS) $(LIB_CAEN) $(ROOTLIBS)


clean::
	rm -f *.o *~ \#*
	rm -f fe2730Th enableTrigger disableTrigger

