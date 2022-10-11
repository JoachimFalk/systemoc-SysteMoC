# vim: set noet ts=8 sts=0 sw=2:

TEX_SOURCES ?= $(TEXSOURCES)
TEX_TARGETS := $(TEX_SOURCES:.tex=.pdf) $(TEX_SOURCES:.tex=-2x1.pdf)

PS_OR_PDF=pdf

include $(RESDIR)/Rules-TeX-Common.mk

PDFLATEX=pdflatex --file-line-error-style

.PRECIOUS: %.aux

%.aux %.idx: %.tex %.tex-dep
	@{ TEXINPUTS="$(TEXINPUTS)" $(PDFLATEX)						\
	     -output-directory $(dir $@) $< </dev/null || 				\
	     { $(RM_F) $*.pdf $*.aux $*.idx; exit 1; } } |				\
	  sed -e 's/^\(.*\.tex:[0-9].*\)$$/[31m\1[39m/';				\
	$(RM_F) $*.pdf;									\
	[ -f $*.aux ]

ifeq ($(BUILD_TEX_AUX_DEPS),yes)
%.pdf: %.tex %.tex-dep %.aux-dep
	$(MV_F) $*.idx $*.idx-old && $(CP_F) $*.idx-old $*.idx;				\
	$(MV_F) $*.aux $*.aux-old && $(CP_F) $*.aux-old $*.aux;				\
	TEXINPUTS="$(TEXINPUTS)" $(PDFLATEX)						\
	    -output-directory $(dir $@) $< </dev/null || 				\
	  { $(RM_F) $*.pdf $*.aux $*.aux-old $*.idx $*.idx-old; exit 1; };		\
	diff $*.aux-old $*.aux >/dev/null && {						\
	    $(MV_F) $*.idx-old $*.idx;							\
	    $(MV_F) $*.aux-old $*.aux && exit 0 || exit 1;				\
	  };										\
	$(RM_F) $@ && $(MAKE) $@ && exit 0 || exit 1
else
%.pdf: %.tex tex-aux-deps.stamp
	$(MAKE) BUILD_TEX_AUX_DEPS=yes $@
endif
