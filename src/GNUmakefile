TOPDIR=..
include $(TOPDIR)/variables.mk

CPP_OBJS=

all: subdirs

.PHONY: subdirs
subdirs:
	cd normalizer_snowball; make all; cd -
	cd tokenizer_punctuation; make all; cd -
	cd tokenizer_word; make all; cd -
	cd segmenter_textwolf; make all; cd -
	cd textproc; make all; cd -
	cd analyzer; make all; cd -

.PHONY: clean
clean:
	cd normalizer_snowball; make clean; cd -
	cd tokenizer_punctuation; make clean; cd -
	cd tokenizer_word; make clean; cd -
	cd segmenter_textwolf; make clean; cd -
	cd textproc; make clean; cd -
	cd analyzer; make clean; cd -

.PHONY: install
install:
	cd normalizer_snowball; make install; cd -
	cd tokenizer_punctuation; make install; cd -
	cd tokenizer_word; make install; cd -
	cd segmenter_textwolf; make install; cd -
	cd textproc; make install; cd -
	cd analyzer; make install; cd -

.PHONY: uninstall
uninstall:
	cd normalizer_snowball; make uninstall; cd -
	cd tokenizer_punctuation; make uninstall; cd -
	cd tokenizer_word; make uninstall; cd -
	cd segmenter_textwolf; make uninstall; cd -
	cd textproc; make uninstall; cd -
	cd analyzer; make uninstall; cd -


