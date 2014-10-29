include variables.mk

all:
	cd 3rdParty; make all; cd -
	cd src; make all; cd ..

test: all

clean:
	cd 3rdParty; make clean; cd -
	cd src; make clean; cd ..

install:
	cd 3rdParty; make install; cd -
	cd src; make install; cd ..
	@mkdir -p $(INSTALL_INCDIR)
	@cp include/strus/*.hpp $(INSTALL_INCDIR)

uninstall:
	cd 3rdParty; make uninstall; cd -
	cd src; make uninstall; cd ..
	@rmdir $(INSTALL_INCDIR)
	@rmdir $(INSTALL_LIBDIR)
	@rmdir $(INSTALL_BINDIR)

