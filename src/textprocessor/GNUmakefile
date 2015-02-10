TOPDIR=../..
include $(TOPDIR)/variables.mk

CPP_OBJS=\
	libstrus_textprocessor.o \
	textProcessor.o

CPP_INCLUDE=-I$(TOPDIR)/include -I$(TOPDIR)/include/private -I.
CXXFLAGS=-g -Werror -Wall -pedantic $(OPTIMIZE) -std=c++98
LDFLAGS=-L/usr/lib -L$(TOPDIR)/src/normalizer_snowball -L$(TOPDIR)/src/tokenizer_punctuation -L$(TOPDIR)/src/tokenizer_word
LIBS=-lstrus_normalizer_snowball -lstrus_tokenizer_punctuation -lstrus_tokenizer_word

all: libstrus_textprocessor.so

-include $(CPP_OBJS:.o=.d)

%.o : %.cpp GNUmakefile $(HFILES)
	$(CXX) -fPIC $(VISIBILITY) -c -o $@ $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp
	$(CXX) -MM $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp > $*.d

libstrus_textprocessor.so: $(CPP_OBJS)
	$(CXX) -shared -o $@ $(CPP_OBJS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(CPP_OBJS) $(CPP_OBJS:.o=.d) libstrus_textprocessor.so

.PHONY: install
install:
	@mkdir -p $(INSTALL_LIBDIR)
	@cp libstrus_textprocessor.so $(INSTALL_LIBDIR)

.PHONY: uninstall
uninstall:
	@rm $(INSTALL_LIBDIR)/libstrus_textprocessor.so

