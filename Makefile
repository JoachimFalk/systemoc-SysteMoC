# vim: set noet ts=8 sts=0 sw=2:

RESDIR=$(shell while [ ! -d HscdTeXRes -a x`pwd` != x'/' ]; do cd ..; done; cd HscdTeXRes; pwd)

.PHONY: docu

douc: docu.pdf

TEX_SOURCES=report.tex docu.tex

TEXINPUTS:=figs:$(RESDIR):$(TEXINPUTS)
TEXCLEANDIRS:=figs $(RESDIR) .

include $(RESDIR)/Rules-TeX.mk
