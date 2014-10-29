TOPDIR=../../..
include $(TOPDIR)/variables.mk
include mkinc.mak

CFLAGS=-Iinclude
all: libstemmer.so libstemmer.o stemwords
libstemmer.o: $(snowball_sources:.c=.o)
	$(AR) -cru $@ $^

libstemmer.so: $(snowball_sources:.c=.o)
	g++  -shared -o $@ $(snowball_sources:.c=.o)

%.o : %.c Makefile
	g++ -fPIC -c -o $@ $(CFLAGS) $*.c

stemwords: examples/stemwords.o libstemmer.o
	$(CC) -o $@ $^
clean:
	rm -f stemwords *.o libstemmer.so src_c/*.o runtime/*.o libstemmer/*.o

install:
	@-cp libstemmer.so $(INSTALL_LIBDIR)

uninstall:
	@-rm $(INSTALL_LIBDIR)/libstemmer.so

