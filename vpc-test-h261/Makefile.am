## Process this file with automake to produce Makefile.in
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
# Erlangen-Nuremberg. All rights reserved.
# 
#   This program is free software; you can redistribute it and/or modify it under
#   the terms of the GNU General Public License as published by the Free Software
#   Foundation; either version 2 of the License, or (at your option) any later
#   version.
# 
#   This program is distributed in the hope that it will be useful, but WITHOUT
#   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
#   details.
# 
#   You should have received a copy of the GNU General Public License along with
#   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
#   Place, Suite 330, Boston, MA 02111-1307 USA.
# 
# --- This software and any associated documentation is provided "as is" 
# 
# IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
# BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
# CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
# DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
# NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
# 
# HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
# DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
# OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
# HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
# ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
# ENHANCEMENTS, OR MODIFICATIONS.

root_builddir := $(shell cd $(top_builddir)/.. && pwd)

include $(auxdir)/jfrules.mk

SUBDIRS=

AM_LDFLAGS = @AM_LDFLAGS@ @SYSTEMC_VPC_LDFLAGS@
AM_CPPFLAGS= @AM_CPPFLAGS@ @SYSTEMC_VPC_INCLUDE@ -Wno-deprecated

LIBS= @LIBS@ -lsystemcvpc

noinst_PROGRAMS= simulation #bb_test dummy_sim test_tq

simulation_DEPENDENCIES= @SYSTEMC_VPC_DEPENDENCIES@
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
