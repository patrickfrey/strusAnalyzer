TOPDIR=../..
include $(TOPDIR)/variables.mk

CPP_OBJS=\
	snowball.o \
	libstrus_normalizer_snowball.o

CPP_INCLUDE=-I$(TOPDIR)/include -I$(TOPDIR)/include/private -I$(TOPDIR)/3rdParty/snowball/libstemmer_c/include/ -I. -I$(TOPDIR)/3rdParty/textwolf/include
CXXFLAGS=-g -Werror -Wall -pedantic $(OPTIMIZE) -std=c++98
LDFLAGS=-L/usr/lib -L$(TOPDIR)/3rdParty/snowball/libstemmer_c/
LIBS=-lstemmer

all: libstrus_normalizer_snowball.so

-include $(CPP_OBJS:.o=.d)

%.o : %.cpp GNUmakefile $(HFILES)
	$(CXX) -fPIC $(VISIBILITY) -c -o $@ $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp
	$(CXX) -MM $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp > $*.d

libstrus_normalizer_snowball.so: $(CPP_OBJS)
	$(CXX) -shared -o $@ $(CPP_OBJS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(CPP_OBJS) $(CPP_OBJS:.o=.d) libstrus_normalizer_snowball.so

.PHONY: install
install:
	@mkdir -p $(INSTALL_LIBDIR)
	@cp libstrus_normalizer_snowball.so $(INSTALL_LIBDIR)

.PHONY: uninstall
uninstall:
	@rm $(INSTALL_LIBDIR)/libstrus_normalizer_snowball.so

