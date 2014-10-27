TOPDIR=..

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



