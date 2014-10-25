TOPDIR=..

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



