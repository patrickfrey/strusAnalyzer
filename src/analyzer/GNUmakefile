TOPDIR=../..
include $(TOPDIR)/variables.mk

CPP_OBJS=\
	parser/lexems.o \
	analyzer.o \
	analyzerLib.o

CPP_INCLUDE=-I$(TOPDIR)/include -I$(TOPDIR)/include/private -I$(TOPDIR)/3rdParty/textwolf/include/ -I/usr/include -I.
CXXFLAGS=-g -Werror -Wall -pedantic -O3 -std=c++98
LDFLAGS=-L/usr/lib -L$(TOPDIR)/src/tokenminer
LIBS=-lstrus_tokenminer

all: libstrus_analyzer.so

-include $(CPP_OBJS:.o=.d)

%.o : %.cpp GNUmakefile $(HFILES)
	g++ -fPIC -fvisibility=hidden -c -o $@ $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp
	g++ -MM $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp > $*.d

libstrus_analyzer.so: $(CPP_OBJS)
	g++  -shared -o $@ $(CPP_OBJS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(CPP_OBJS) $(CPP_OBJS:.o=.d) libstrus_analyzer.so

.PHONY: install
install:
	@mkdir -p $(INSTALL_LIBDIR)
	@cp libstrus_analyzer.so $(INSTALL_LIBDIR)

.PHONY: uninstall
uninstall:
	@rm $(INSTALL_LIBDIR)/libstrus_analyzer.so

