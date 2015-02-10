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
	@mkdir -p $(INSTALL_INCDIR)/analyzer
	@mkdir -p $(INSTALL_INCDIR)/tokenizer
	@mkdir -p $(INSTALL_INCDIR)/lib
	@cp include/strus/analyzer/*.hpp $(INSTALL_INCDIR)/analyzer
	@cp include/strus/tokenizer/*.hpp $(INSTALL_INCDIR)/tokenizer
	@cp include/strus/lib/*.hpp $(INSTALL_INCDIR)/lib
	@cp include/strus/*.hpp $(INSTALL_INCDIR)

uninstall:
	cd 3rdParty; make uninstall; cd -
	cd src; make uninstall; cd ..
	@-rm $(INSTALL_INCDIR)/tokenizer/token.hpp
	@-rm $(INSTALL_INCDIR)/segmenterInstanceInterface.hpp
	@-rm $(INSTALL_INCDIR)/tokenizerInterface.hpp
	@-rm $(INSTALL_INCDIR)/normalizerConfig.hpp
	@-rm $(INSTALL_INCDIR)/segmenterInterface.hpp
	@-rm $(INSTALL_INCDIR)/tokenizerConfig.hpp
	@-rm $(INSTALL_INCDIR)/queryAnalyzerInterface.hpp
	@-rm $(INSTALL_INCDIR)/textProcessorInterface.hpp
	@-rm $(INSTALL_INCDIR)/normalizerInterface.hpp
	@-rm $(INSTALL_INCDIR)/documentAnalyzerInterface.hpp
	@-rm $(INSTALL_INCDIR)/analyzer/attribute.hpp
	@-rm $(INSTALL_INCDIR)/analyzer/document.hpp
	@-rm $(INSTALL_INCDIR)/analyzer/metaData.hpp
	@-rm $(INSTALL_INCDIR)/analyzer/query.hpp
	@-rm $(INSTALL_INCDIR)/analyzer/term.hpp
	@-rm $(INSTALL_INCDIR)/lib/analyzer.hpp
	@-rm $(INSTALL_INCDIR)/lib/normalizer_snowball.hpp
	@-rm $(INSTALL_INCDIR)/lib/tokenizer_word.hpp
	@-rm $(INSTALL_INCDIR)/lib/textprocessor.hpp
	@-rm $(INSTALL_INCDIR)/lib/tokenizer_punctuation.hpp
	@-rm $(INSTALL_INCDIR)/lib/segmenter_textwolf.hpp
	@rmdir $(INSTALL_INCDIR)/analyzer
	@rmdir $(INSTALL_INCDIR)/tokenizer
	@rmdir $(INSTALL_INCDIR)/lib
	@rmdir $(INSTALL_INCDIR)
	@rmdir $(INSTALL_LIBDIR)
	@rmdir $(INSTALL_BINDIR)
