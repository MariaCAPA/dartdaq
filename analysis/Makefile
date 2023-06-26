#
# Example Makefile for ROOTANA-based projects
#

CXXFLAGS = -g -O2 -Wall -Wuninitialized -fPIC

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
OBJS += TDartAnaManager.o 
OBJS += TEventProcessor.o 
#OBJS += TDartEventDict.o 
OBJS += TChannelHistograms.o 
OBJS += TDartReadRun.o 
OBJS += TDartVisu.o 
OBJS += TEventMusic.o

all: $(OBJS) anaMusic.exe wfViewer.exe midas2root.exe wfAverage.exe dartlib.so

lib: dartlib.so

anaMusic.exe: anaMusic.cxx TDartEventDict.cxx $(OBJS) 
	$(CXX) -o $@ $(CXXFLAGS) $(INCFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lutil

wfViewer.exe: wfViewer.cxx TDartEventDict.cxx $(OBJS) 
	$(CXX) -o $@ $(CXXFLAGS) $(INCFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lutil

wfAverage.exe: wfAverage.cxx TDartEventDict.cxx $(OBJS) 
	$(CXX) -o $@ $(CXXFLAGS) $(INCFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lutil

midas2root.exe: midas2root.cxx TDartEventDict.cxx $(OBJS) 
	$(CXX) -o $@ $(CXXFLAGS) $(INCFLAGS) $^ $(LIBS) $(ROOTGLIBS) -lm -lz -lpthread -lutil

%Dict.o: %Dict.cxx
	$(CXX) -o $@ $(CXXFLAGS) $(INCFLAGS) -c -I. $<

%Dict.cxx: TV1730Waveform.hxx TDartAnaManager.hxx  TDartReadRun.hxx TDartVisu.hxx
	#rootcint -f $@ -c $(CXXFLAGS) -I. $*.hxx $^ $*_LinkDef.h
	rootcint -f $@ -c -I. $(INCFLAGS) $*.hxx $^ $*_LinkDef.h

%.o: %.cxx
	$(CXX) -o $@ $(CXXFLAGS) $(INCFLAGS) -c $<

#dartlib.so: TDartEventDict.cxx TDartEvent.hxx 
dartlib.so: *.cxx *.hxx
	$(CXX) $(CXXFLAGS) $(INCFLAGS) -shared -o $@  TDartEventDict.cxx $(OBJS) $(LIBS) $(ROOTGLIBS) 
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
