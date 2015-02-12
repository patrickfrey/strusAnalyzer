TOPDIR=../..

all:
	cd src; make all; cd ..

test: all
	LD_LIBRARY_PATH=$(LD_LIBRARY_PATH):$(TOPDIR)/src/segmenter_textwolf src/testAnnotationPos

clean:
	cd src; make clean; cd ..


