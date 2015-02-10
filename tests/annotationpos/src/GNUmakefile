TOPDIR=../../..

CPP_OBJS=\
	testAnnotationPos.o

CPP_INCLUDE=-I$(TOPDIR)/include -I$(TOPDIR)/src/segmenter_textwolf -I$(TOPDIR)/include/private -I$(TOPDIR)/include/tests
CXXFLAGS=-g -Werror -Wall -Wfatal-errors -pedantic -std=c++98
LDFLAGS=-L$(TOPDIR)/src/segmenter_textwolf
LIBS=-lstrus_segmenter_textwolf

all: testAnnotationPos

-include $(CPP_OBJS:.o=.d)

%.o : %.cpp GNUmakefile
	$(CXX) -c -o $@ $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp
	$(CXX) -MM $(CXXFLAGS) $(CPP_INCLUDE) $*.cpp > $*.d

testAnnotationPos: $(CPP_OBJS)
	$(CXX) -o $@ $(CPP_OBJS) $(LDFLAGS) $(LIBS)

.PHONY: clean
clean:
	rm -f $(CPP_OBJS) $(CPP_OBJS:.o=.d) testAnnotationPos

