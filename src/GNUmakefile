TOPDIR=..
include $(TOPDIR)/variables.mk

CPP_OBJS=

all: subdirs

.PHONY: subdirs
subdirs:
	cd tokenminer; make all; cd -
	cd analyzer; make all; cd -

.PHONY: clean
clean:
	cd tokenminer; make clean; cd -
	cd analyzer; make clean; cd -

.PHONY: install
install:
	cd tokenminer; make install; cd -
	cd analyzer; make install; cd -

.PHONY: uninstall
uninstall:
	cd tokenminer; make uninstall; cd -
	cd analyzer; make uninstall; cd -



