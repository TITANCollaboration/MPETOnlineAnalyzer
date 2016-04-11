# Makefile

# ROOTANA library
OBJS = TMidasFile.o TMidasEvent.o

# old
#CXXFLAGS = -g -O2 -Wall -Wuninitialized

CXXFLAGS = -g -O2 -std=gnu++0x -Wall -Wuninitialized -I$(HOME)/packages/rootana -lrt

# optional ROOT libraries

ifdef ROOTSYS
ROOTLIBS  = -L$(ROOTSYS)/lib $(shell $(ROOTSYS)/bin/root-config --libs)  -lXMLParser -lThread -lRHTTP
ROOTGLIBS = -L$(ROOTSYS)/lib $(shell $(ROOTSYS)/bin/root-config --glibs) -lXMLParser -lThread -lRHTTP
RPATH    += -Wl,-rpath,$(ROOTSYS)/lib
CXXFLAGS += -DHAVE_ROOT -I$(ROOTSYS)/include -DHAVE_THTTP_SERVER
#ROOTSYS1 = /home/mpet/packages/root-m32
#ROOTLIBS  = -L$(ROOTSYS1)/lib $(shell $(ROOTSYS1)/bin/root-config --libs)  -lXMLParser -lThread
#ROOTGLIBS = -L$(ROOTSYS1)/lib $(shell $(ROOTSYS1)/bin/root-config --glibs) -lXMLParser -lThread
#RPATH    += -Wl,-rpath,$(ROOTSYS1)/lib
#CXXFLAGS += -DHAVE_ROOT -I$(ROOTSYS1)/include
endif

# optional MIDAS libraries

ifdef MIDASSYS
MIDASLIBS = $(MIDASSYS)/linux/lib/libmidas.a -lutil 
#MIDASLIBS = $(MIDASSYS)/linux-m32/lib/libmidas.a -lutil 
MIDASVME  = $(MIDASSYS)/drivers/vme
CXXFLAGS += -DHAVE_MIDAS -DOS_LINUX -Dextname -I$(MIDASSYS)/include -I$(MIDASVME) -I$(HOME)/packages/rootana/include
endif

ALL:= analyzer.exe

all: $(ALL)

analyzer.exe: %.exe: %.o MCP_TDC.o $(HOME)/packages/rootana/lib/librootana.a
	$(CXX) -o $@ $(CXXFLAGS) $^ $(MIDASLIBS) $(ROOTGLIBS) -lm -lz -lpthread $(RPATH) $(MIDASLIBS)

%.o: %.cxx
	$(CXX) $(CXXFLAGS) -c $<

dox:
	doxygen

clean::
	-rm -f *.o *.exe $(ALL)

# end
