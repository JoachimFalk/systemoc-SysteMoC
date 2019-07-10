# vim: set noet ts=8 sts=0 sw=2:

RESDIR=$(shell while [ ! -d HscdTeXRes -a x`pwd` != x'/' ]; do cd ..; done; cd HscdTeXRes; pwd)

.PHONY: smoc-docu

docu: smoc-docu.pdf

TEX_SOURCES=smoc-docu.tex

TEXINPUTS:=figs:$(TEXINPUTS)
TEXCLEANDIRS:= figs .

include $(RESDIR)/Rules-TeX-PDF.mk
