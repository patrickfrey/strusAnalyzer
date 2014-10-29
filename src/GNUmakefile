TOPDIR=..
include $(TOPDIR)/variables.mk

CPP_OBJS=

all: subdirs

.PHONY: subdirs
subdirs:
	cd system; make all; cd -
	cd tokenminer; make all; cd -
	cd analyzer; make all; cd -
	cd strusAnalyzer; make all; cd -

.PHONY: clean
clean:
	cd system; make clean; cd -
	cd tokenminer; make clean; cd -
	cd analyzer; make clean; cd -
	cd strusAnalyzer; make clean; cd -

.PHONY: install
install:
	cd system; make install; cd -
	cd tokenminer; make install; cd -
	cd analyzer; make install; cd -
	cd strusAnalyzer; make install; cd -

.PHONY: uninstall
uninstall:
	cd system; make uninstall; cd -
	cd tokenminer; make uninstall; cd -
	cd analyzer; make uninstall; cd -
	cd strusAnalyzer; make uninstall; cd -



