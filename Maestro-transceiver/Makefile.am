
include $(auxdir)/jfrules.mk

SUBDIRS=


AM_LDFLAGS = @AM_LDFLAGS@ @SYSTEMOC_LDFLAGS@ @MAESTROIL_LDFLAGS@ @MAESTROML_LDFLAGS@ @BOOST_LDFLAGS@
AM_CPPFLAGS= @AM_CPPFLAGS@ @SYSTEMOC_INCLUDE@ @MAESTROIL_INCLUDE@ @MAESTROML_INCLUDE@ @BOOST_INCLUDE@ -Wno-deprecated

noinst_PROGRAMS= TransceiverM

TransceiverM_SOURCES= SystemModel.cpp

TransceiverM_LDADD =
if PKG_SYSTEMOC_USE_SRCDIR_VERSION
TransceiverM_LDADD += $(SYSTEMOC_LIBPATH)/libsystemoc.la
TransceiverM_LDADD += -lboost_filesystem
else
# SYSTEMOC_LIBPATH may not be defined for external libraries
TransceiverM_LDADD += -lsystemoc
TransceiverM_LDADD += -lboost_filesystem
endif

if PKG_MAESTROMM_USE_SRCDIR_VERSION
TransceiverM_LDADD += $(MAESTROMM_LIBPATH)/libmaestro_mm.la
else
# SYSTEMOC_LIBPATH may not be defined for external libraries
TransceiverM_LDADD += -lmaestro_mm
endif


#socmodel_LDADD += ../../InterfaceLibrary/libdigitalinterfaces.la
#socmodel_LDADD += ../../ModelLibrary/libarchmodels.la
#socmodel_LDADD += ../FunctionalModel/libfunctionalmodel.la
#socmodel_LDADD += ../GSM_Paging/libgsmpaging.la
#socmodel_LDADD += ../IMC_BasebandProc_XGOLD/libimcbaseband.la