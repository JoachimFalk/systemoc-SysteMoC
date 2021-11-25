# vim: set noet ts=8 sts=0 sw=2:

SHELL=/bin/bash

RM_F ?= rm -f
MV_F ?= mv -f
CP_F ?= cp -f
MKDIR_P ?= mkdir -p

TEX_STDINCLUDES:=$(shell find /usr/share/texmf/tex -type d)
OOFFICE:=$(shell which ooffice)
ifeq ($(OOFFICE),)
OOFFICE:=$(shell which libreoffice)
endif

ifndef BIB_SOURCES
BIB_SOURCES = literature.bib
endif

ifndef TEX_NO_ALL_TARGET
.PHONY: all
all: tex-all
endif
ifndef TEX_NO_CLEAN_TARGET
.PHONY: clean
clean: tex-clean
endif

.PHONY: tex-all
tex-all: $(TEX_TARGETS)

.PHONY: tex-clean
tex-clean:
	@$(RM_F) tex-deps.stamp tex-aux-deps.stamp \
		$(TEX_TARGETS) \
		$(TEX_SOURCES:.tex=.dvi) \
		$(TEX_SOURCES:.tex=.nav) \
		$(TEX_SOURCES:.tex=.snm) \
		$(TEX_SOURCES:.tex=.vrb) \
		$(TEX_SOURCES:.tex=.out) \
		$(TEX_SOURCES:.tex=.log) \
		$(TEX_SOURCES:.tex=.toc) \
		$(TEX_SOURCES:.tex=.aux) \
		$(TEX_SOURCES:.tex=.aux-old) \
		$(TEX_SOURCES:.tex=.aux-dep) \
		$(TEX_SOURCES:.tex=.idx) \
		$(TEX_SOURCES:.tex=.idx-old) \
		$(TEX_SOURCES:.tex=.bbl) \
		$(TEX_SOURCES:.tex=.blg) \
		$(TEX_SOURCES:.tex=.ind) \
		$(TEX_SOURCES:.tex=.ilg)
	@find $(TEXCLEANDIRS) \( \
		-name "*.tex-dep" \
	  \) -delete
	@find $(TEXCLEANDIRS) -name "*.pdf" | \
	  while read i; do \
	    for srcformat in eps ps; do \
	      if test \( x"$(VPATH)" != x"" -a -f "$(VPATH)/$${i%-*}.$${srcformat}" \) \
					    -o -f "$${i%-*}.$${srcformat}"; then \
		$(RM_F) $$i; \
	      fi; \
	    done; \
	  done
	@find $(TEXCLEANDIRS) -name "*.pdf" -o -name "*.ps" -o -name "*.eps" | \
	  while read i; do \
	    for srcformat in fig dia svg odg plt; do \
	      if test \( x"$(VPATH)" != x"" -a -f "$(VPATH)/$${i%-*}.$${srcformat}" \) \
					    -o -f "$${i%-*}.$${srcformat}"; then \
		$(RM_F) $$i; \
	      fi; \
	    done; \
	  done
	@find $(TEXCLEANDIRS) -name "*-fig.tex" -o -name "*-tex.pdf" -o -name "*-tex.ps" | \
	  while read i; do \
	    if test \( x"$(VPATH)" != x"" -a -f "$(VPATH)/$${i%-*}.fig" \) \
					  -o -f "$${i%-*}.fig"; then \
	      $(RM_F) $$i; \
	    fi; \
	  done

ifeq ($(BUILD_TEX_AUX_DEPS)$(BUILD_TEX_DEPS),yes)
%.tex-dep: %.tex
	@$(MKDIR_P) $(dir $@) && {							\
	  TEXINPUTS="$(TEXINPUTS)";		export TEXINPUTS;			\
	  TEX_STDINCLUDES="$(TEX_STDINCLUDES)";	export TEX_STDINCLUDES;			\
	  RESDIR="$(RESDIR)";			export RESDIR;				\
	  SRCDIR="$(srcdir)";			export SRCDIR;				\
	  PS_OR_PDF="$(PS_OR_PDF)";		export PS_OR_PDF;			\
	  echo -n "tex-aux-deps.stamp $*.tex-dep: $*.tex" && "$(RESDIR)"/Dep-TeX.perl;	\
	} < $< > $@
endif

ifeq ($(BUILD_TEX_AUX_DEPS),yes)
%.aux-dep: %.aux
	@$(MKDIR_P) $(dir $@) && {							\
	  STEM="$*"; 				export STEM;				\
	  PS_OR_PDF="$(PS_OR_PDF)";		export PS_OR_PDF;			\
	  "$(RESDIR)"/Cite-TeX.perl;							\
	  grep "makeidx" "$*.log" > /dev/null && echo " $*.ind" || echo;		\
	} < $< > $@
endif

TEX_DEPS=$(TEX_SOURCES:.tex=.tex-dep)
TEX_AUX_DEPS=$(TEX_SOURCES:.tex=.aux-dep)

tex-deps.stamp:
	$(MAKE) BUILD_TEX_DEPS=yes $(TEX_DEPS) && touch $@
tex-aux-deps.stamp: tex-deps.stamp
	$(MAKE) BUILD_TEX_AUX_DEPS=yes $(TEX_AUX_DEPS) && touch $@

-include $(TEX_DEPS) $(TEX_AUX_DEPS)

%.bbl: %.aux $(BIB_SOURCES)
	set $(dir $^); if test x"$$1" != x; then cd $$1; fi && BIBINPUTS=`cd $$2; pwd`:"$$BIBINPUTS" bibtex $(notdir $(basename $<))

%-fig.tex: %.fig
	@$(MKDIR_P) $(dir $@) && {							\
	  echo -e '\\begin{picture}(0,0)%' &&						\
	  echo -e '\\includegraphics[]{$*-tex}%' &&					\
	  echo -e '\\end{picture}%' &&							\
	  ( cd $(dir $<) && fig2dev -L pstex_t $(notdir $<) ); } > $@

%-tex.pdf: %.fig
	@$(MKDIR_P) $(dir $@) &&							\
	  ( cd $(dir $<) && fig2dev -L pdftex  $(notdir $<) )    > $@

%-tex.ps: %.fig
	@$(MKDIR_P) $(dir $@) &&							\
	  ( cd $(dir $<) && fig2dev -L pstex   $(notdir $<) )    > $@

%.eps: %.fig
	@$(MKDIR_P) $(dir $@) && 							\
	  ( cd $(dir $<) && fig2dev -L eps     $(notdir $<) )    > $@

%.pdf: %.fig
	@$(MKDIR_P) $(dir $@) &&							\
	  ( cd $(dir $<) && fig2dev -L pdf    $(notdir $<) )     > $@

%.eps: %.odg
	cd $(dir $<) && $(OOFFICE) --headless --convert-to eps $(notdir $<)

%.pdf: %.odg
	cd $(dir $<) && $(OOFFICE) --headless --convert-to pdf $(notdir $<)

%.eps: %.dot
	@$(MKDIR_P) $(dir $@) && dot -Tps  $< > $@

%.fig: %.dot
	@$(MKDIR_P) $(dir $@) && dot -Tfig $< > $@

%.eps: %.dia
	cd `dirname $<` && dia --nosplash --filter=eps `basename $<` --export `basename $@`

%.eps: %.svg
	cd `dirname $<` && inkscape --export-eps `basename $@` `basename $<`

%.pdf: %.svg
	cd `dirname $<` && inkscape --export-pdf `basename $@` `basename $<`

%.eps: %.plt
	cd `dirname $<` && (\
		echo "set output \""`basename $@`"\"";\
		echo 'set terminal postscript eps enhanced color';\
		sed -e '/^set terminal /d' `basename $<` ) | gnuplot

%.pdf: %.ps
	@$(MKDIR_P) $(dir $@) &&							\
	  epstopdf $< --outfile=$@

%.pdf: %.eps
	@$(MKDIR_P) $(dir $@) &&							\
	  epstopdf $< --outfile=$@

%.ps: %.dvi
	@$(MKDIR_P) $(dir $@) &&							\
	  dvips $< -o $@

%-2x1.pdf: %.pdf
	@$(MKDIR_P) $(dir $@) &&							\
	  pdfjam --suffix 2x1 --nup '2x1' --landscape --outfile $@ -- $<

# ps & pdf to (color) png conversion
%.png: %.ps
	@$(MKDIR_P) $(dir $@) &&							\
	  gs -r300 -sDEVICE=pngalpha -dNOPAUSE -dBATCH -sOutputFile="$@" "$<"

%.png: %.pdf
	@$(MKDIR_P) $(dir $@) &&							\
	  gs -r300 -sDEVICE=pngalpha -dNOPAUSE -dBATCH -sOutputFile="$@" "$<"

%.ind: %.idx
	makeindex "$<"
