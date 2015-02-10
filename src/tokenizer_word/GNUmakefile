TOPDIR=../..
include $(TOPDIR)/variables.mk

CPP_OBJS=\
	libstrus_tokenizer_word.o

CPP_INCLUDE=-I$(TOPDIR)/include -I$(TOPDIR)/include/private -I. -I$(TOPDIR)/3rdParty/textwolf/include
CXXFLAGS=-g -Werror -Wall -pedantic $(OPTIMIZE) -std=c++98
LDFLAGS=
LIBS=

all: libstrus_tokenizer_word.so

-include $(CPP_OBJS:.o=.d)

%.o : %.cpp GNUmakefile $(HFILES)
	$(CXX) -fPIC $(VISIBILITY) -c -o $@ $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp
	$(CXX) -MM $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp > $*.d

libstrus_tokenizer_word.so: $(CPP_OBJS)
	$(CXX) -shared -o $@ $(CPP_OBJS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(CPP_OBJS) $(CPP_OBJS:.o=.d) libstrus_tokenizer_word.so

.PHONY: install
install:
	@mkdir -p $(INSTALL_LIBDIR)
	@cp libstrus_tokenizer_word.so $(INSTALL_LIBDIR)

.PHONY: uninstall
uninstall:
	@rm $(INSTALL_LIBDIR)/libstrus_tokenizer_word.so

