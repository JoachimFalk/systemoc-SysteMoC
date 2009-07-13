# vim: set noet ts=8 sts=0 sw=2:

RESDIR=$(shell while [ ! -d HscdTeXRes -a x`pwd` != x'/' ]; do cd ..; done; cd HscdTeXRes; pwd)

TEX_SOURCES=SysteMoC-Tutorial-slides.tex SysteMoC-Tutorial-report.tex

doc: tex-all

all:

SUBDIRS=

TEXCLEANDIRS=. $(srcdir) pics
TEXINPUTS:=$(srcdir):$(RESDIR):$(RESDIR)/beamer-latex-styles:$(TEXINPUTS):pics:philosophers

include $(RESDIR)/Rules-TeX.mk

todo:
	grep -n todo *.tex

force:
	$(MAKE) clean
	$(MAKE) tex-all

.PHONY: todo force all doc
