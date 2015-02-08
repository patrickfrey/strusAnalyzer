TOPDIR=..
include $(TOPDIR)/variables.mk

CPP_OBJS=

all: subdirs

.PHONY: subdirs
subdirs:
	cd segmenter_textwolf; make all; cd -
	cd tokenminer; make all; cd -
	cd analyzer; make all; cd -

.PHONY: clean
clean:
	cd segmenter_textwolf; make clean; cd -
	cd tokenminer; make clean; cd -
	cd analyzer; make clean; cd -

.PHONY: install
install:
	cd segmenter_textwolf; make install; cd -
	cd tokenminer; make install; cd -
	cd analyzer; make install; cd -

.PHONY: uninstall
uninstall:
	cd segmenter_textwolf; make uninstall; cd -
	cd tokenminer; make uninstall; cd -
	cd analyzer; make uninstall; cd -



