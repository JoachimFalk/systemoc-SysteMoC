# vim: set noet ts=8 sts=0 sw=2:

RESDIR=$(shell while [ ! -d HscdTeXRes -a x`pwd` != x'/' ]; do cd ..; done; cd HscdTeXRes; pwd)

TEX_SOURCES=SysteMoC-Tutorial-slides.tex SysteMoC-Tutorial-report.tex

doc: tex-all

all:

SUBDIRS=

TEXINPUTS:=../figs:$(RESDIR):$(RESDIR)/beamer_fau_logo/4-3:$(TEXINPUTS)
TEXCLEANDIRS=../figs .

include $(RESDIR)/Rules-TeX-PDF.mk

todo:
	grep -n todo *.tex

force:
	$(MAKE) clean
	$(MAKE) tex-all

.PHONY: todo force all doc
