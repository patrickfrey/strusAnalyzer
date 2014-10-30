TOPDIR=../..
include $(TOPDIR)/variables.mk

CPP_OBJS=

CPP_INCLUDE=-I$(TOPDIR)/include -I$(TOPDIR)/include/private
CXXFLAGS=-g -Werror -Wall -pedantic $(OPTIMIZE) -std=c++98
LDFLAGS=-L$(TOPDIR)/src/analyzer -L$(TOPDIR)/src/tokenminer -L$(TOPDIR)/3rdParty/snowball/libstemmer_c -L$(TOPDIR)/src/system -L/usr/lib
LIBS=-lstrus_analyzer -lstrus_tokenminer -lstemmer -lstrus_system -lboost_system

all: strusAnalyze

-include $(CPP_OBJS:.o=.d)

%.o : %.cpp GNUmakefile
	g++ -c -o $@ $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp
	g++ -MM $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp > $*.d

strusAnalyze: $(CPP_OBJS) strusAnalyze.o
	g++ -o $@ strusAnalyze.o $(CPP_OBJS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f ${CPP_OBJS} ${CPP_OBJS:.o=.d} strusAnalyze strusAnalyze.o strusAnalyze.d


.PHONY: install
install:
	@mkdir -p $(INSTALL_BINDIR)
	@cp strusAnalyze $(INSTALL_BINDIR)

.PHONY: uninstall
uninstall:
	@rm $(INSTALL_BINDIR)/strusAnalyze



