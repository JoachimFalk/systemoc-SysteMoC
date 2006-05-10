## Process this file with automake to produce Makefile.in
# vim: set noet sw=2 ts=8 sts=0:
root_srcdir=$(top_srcdir)/..
root_builddir=$(top_builddir)/..

SYSTEMC_VPC_BUILDDIR=$(root_builddir)/SystemC-VPC

SUBDIRS=

INCLUDES = @INCLUDES@ @SYSTEMC_VPC_INCLUDE@

AM_LDFLAGS = @AM_LDFLAGS@ @SYSTEMC_VPC_LDFLAGS@
AM_CPPFLAGS=-Wno-deprecated

LIBS=-lsystemcvpc

noinst_PROGRAMS= simulation #bb_test dummy_sim test_tq

simulation_DEPENDENCIES= $(SYSTEMC_VPC_BUILDDIR)/libsystemcvpc.la
simulation_SOURCES= \
    rlc_id106011.cpp c_in_diff_id106018.cpp c_q_iq_id106022.cpp \
    sf_id106012.cpp c_rf_bm_id106025.cpp in_id106005.cpp \
    dct_id106002.cpp c_rec_sf_id106024.cpp c_sf_rlc_id106027.cpp \
    diff_id106003.cpp c_idct_rec_id106016.cpp q_id106008.cpp \
    c_dct_q_id106014.cpp c_q_rlc_id106023.cpp idct_id106004.cpp \
    c_diff_dct_id106015.cpp c_in_bm_id106017.cpp lf_id106007.cpp \
    c_in_rf_id106026.cpp bm_id106001.cpp iq_id106006.cpp \
    c_iq_idct_id106019.cpp rec_id106009.cpp \
    c_lf_rec_id106021.cpp c_bm_lf_id106013.cpp \
    rf_id106010.cpp c_lf_diff_id106020.cpp sc_main.cpp monitor.cpp
