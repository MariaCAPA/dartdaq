#
# Example Makefile for ROOTANA-based projects
#

CXXFLAGS = -g  -Wall -Wuninitialized -fPIC
#CXXFLAGS =  -O2 -Wall -Wuninitialized -fPIC

# required ZLIB library

CXXFLAGS += -DHAVE_LIBZ

# required ROOTANA library

ifndef ROOTANASYS
ROOTANASYS=../../rootana
endif

INCFLAGS = -I$(ROOTANASYS)/include
LIBS += -L$(ROOTANASYS)/lib -lrootana

# ROOT libraries

ifdef ROOTSYS

ROOTCFLAGS   := $(shell root-config --cflags)
ROOTFEATURES := $(shell root-config --features)
ROOTGLIBS    := $(shell root-config --glibs) -lThread
HAVE_ROOT_HTTP := $(findstring http,$(ROOTFEATURES))
HAVE_ROOT_XML  := $(findstring xml,$(ROOTFEATURES))

CXXFLAGS  += -DHAVE_ROOT $(ROOTCFLAGS)

ifdef HAVE_ROOT_XML
CXXFLAGS  += -DHAVE_ROOT_XML
ROOTGLIBS += -lXMLParser
endif

ifdef HAVE_ROOT_HTTP
CXXFLAGS  += -DHAVE_ROOT_HTTP -DHAVE_THTTP_SERVER
ROOTGLIBS += -lRHTTP
endif

endif # ROOTSYS

# optional MIDAS library

ifdef MIDASSYS

MIDASLIBS = $(MIDASSYS)/lib/libmidas.a -lutil -lrt 
CXXFLAGS += -DHAVE_MIDAS -DOS_LINUX -Dextname -I$(MIDASSYS)/include

UNAME := $(shell uname)
ifeq ($(UNAME),Darwin)
MIDASLIBS = $(MIDASSYS)/lib/libmidas.a
endif

LIBS += $(MIDASLIBS)

endif # MIDASSYS

#OBJS += TEvaluationData.o 
#OBJS += TEvaluationHistograms.o 
OBJS += TV1730RawData.o 
OBJS += TV1730Waveform.o 
OBJS += TAAnaManager.o 
OBJS += TEventProcessor.o 
#OBJS += TAEventDict.o 
OBJS += TChannelHistograms.o 
OBJS += TAReadRun.o 
OBJS += TAVisu.o 

all: $(OBJS) wfViewer.exe midas2root.exe midas2Anais.exe wfAverage.exe libAnod.so

lib: libAnod.so

wfViewer.exe: wfViewer.cxx TAEventDict.cxx $(OBJS) 
	$(CXX) -o $@ $(CXXFLAGS) $(INCFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lutil

wfAverage.exe: wfAverage.cxx TAEventDict.cxx $(OBJS) 
	$(CXX) -o $@ $(CXXFLAGS) $(INCFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lutil

midas2root.exe: midas2root.cxx TAEventDict.cxx $(OBJS) 
	$(CXX) -o $@ $(CXXFLAGS) $(INCFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lutil

midas2Anais.exe: midas2Anais.cxx TAEventDict.cxx $(OBJS) 
	$(CXX) -o $@ $(CXXFLAGS) $(INCFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lutil

%Dict.o: %Dict.cxx
	$(CXX) -o $@ $(CXXFLAGS) $(INCFLAGS) -c -I. $<

%Dict.cxx: TV1730Waveform.hxx TAAnaManager.hxx  TAReadRun.hxx TAVisu.hxx
	#rootcint -f $@ -c $(CXXFLAGS) -I. $*.hxx $^ $*_LinkDef.h
	rootcling -f $@ -c -I. $(INCFLAGS) $*.hxx $^ $*_LinkDef.h

%.o: %.cxx
	$(CXX) -o $@ $(CXXFLAGS) $(INCFLAGS) -c $<

libAnod.so: *.cxx *.hxx
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -shared -o $@  TAEventDict.cxx $(OBJS) $(LIBS) $(ROOTGLIBS) 
	#$(CXX) $(CXXFLAGS) $(INCFLAGS) -shared -o $@  $^



dox:
	doxygen

clean::
	-rm -f *.o *.a *.so
	-rm -f *.exe
	-rm -f *Dict.cxx
	-rm -rf *.exe.dSYM
	-rm -rf *.pcm

# end
