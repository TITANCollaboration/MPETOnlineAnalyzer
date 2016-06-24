# Makefile

# ROOTANA library
OBJS = TMidasFile.o TMidasEvent.o

CXXFLAGS = -g -O2 -std=gnu++0x -Wall -Wuninitialized -I$(HOME)/packages/rootana -lrt

# optional ROOT libraries

ifdef ROOTSYS
ROOTLIBS  = -L$(ROOTSYS)/lib $(shell $(ROOTSYS)/bin/root-config --libs)  -lXMLParser -lThread -lRHTTP
ROOTGLIBS = -L$(ROOTSYS)/lib $(shell $(ROOTSYS)/bin/root-config --glibs) -lXMLParser -lThread -lRHTTP
RPATH    += -Wl,-rpath,$(ROOTSYS)/lib
CXXFLAGS += -DHAVE_ROOT -I$(ROOTSYS)/include -DHAVE_THTTP_SERVER
endif

# optional MIDAS libraries

ifdef MIDASSYS
MIDASLIBS = $(MIDASSYS)/linux/lib/libmidas.a -lutil 
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

clean:
	-rm -f *.o *.exe $(ALL)

rebuild: clean all

# end
