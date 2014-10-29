TOPDIR=../..
include $(TOPDIR)/variables.mk

CPP_OBJS=

CPP_INCLUDE=-I$(TOPDIR)/include -I$(TOPDIR)/include/private -I$(INSTALL_INCDIR)
CXXFLAGS=-g -Werror -Wall -pedantic -std=c++98
LDFLAGS=-L$(TOPDIR)/src/analyzer -L$(TOPDIR)/src/tokenminer -L$(TOPDIR)/3rdParty/snowball/libstemmer_c -L$(TOPDIR)/src/system -L/usr/lib -L/usr/local/lib -L$(INSTALL_LIBDIR)
LIBS=-lstrus_analyzer -lstrus_tokenminer -lstemmer -lstrus_system -lstrus_lvdbstorage -lboost_system

all: strusInserter

-include $(CPP_OBJS:.o=.d)

%.o : %.cpp GNUmakefile
	g++ -c -o $@ $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp
	g++ -MM $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp > $*.d

strusInserter: $(CPP_OBJS) strusInserter.o
	g++ -o $@ strusInserter.o $(CPP_OBJS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f ${CPP_OBJS} ${CPP_OBJS:.o=.d} strusInserter strusInserter.o strusInserter.d

.PHONY: install
install:
	@mkdir -p $(INSTALL_BINDIR)
	@cp strusInserter $(INSTALL_BINDIR)

.PHONY: uninstall
uninstall:
	@rm $(INSTALL_BINDIR)/strusInserter



