TOPDIR=../..
include $(TOPDIR)/variables.mk

CPP_OBJS=\
	chartable.o \
	snowball.o \
	punctuation.o \
	tokenMinerLib.o

CPP_INCLUDE=-I$(TOPDIR)/include -I$(TOPDIR)/include/private -I$(TOPDIR)/3rdParty/snowball/libstemmer_c/include/ -I$(TOPDIR)/3rdParty/textwolf/include -I/usr/include -I.
CXXFLAGS=-g -Werror -Wall -pedantic -std=c++98
LDFLAGS=-L/usr/lib -L$(TOPDIR)/3rdParty/snowball/libstemmer_c/
LIBS=-lstemmer

all: libstrus_tokenminer.so

-include $(CPP_OBJS:.o=.d)

%.o : %.cpp GNUmakefile $(HFILES)
	g++ -fPIC -fvisibility=hidden -c -o $@ $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp
	g++ -MM $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp > $*.d

libstrus_tokenminer.so: $(CPP_OBJS)
	g++  -shared -o $@ $(CPP_OBJS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(CPP_OBJS) $(CPP_OBJS:.o=.d) libstrus_tokenminer.so

.PHONY: install
install:
	@mkdir -p $(INSTALL_LIBDIR)
	@cp libstrus_tokenminer.so $(INSTALL_LIBDIR)

.PHONY: uninstall
uninstall:
	@rm $(INSTALL_LIBDIR)/libstrus_tokenminer.so

