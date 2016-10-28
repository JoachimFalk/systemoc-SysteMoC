## Process this file with automake to produce Makefile.in
#
# Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
#
#   This library is free software; you can redistribute it and/or modify it under
#   the terms of the GNU Lesser General Public License as published by the Free
#   Software Foundation; either version 2 of the License, or (at your option) any
#   later version.
#
#   This library is distributed in the hope that it will be useful, but WITHOUT
#   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
#   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
#   details.
#
#   You should have received a copy of the GNU Lesser General Public License
#   along with this library; if not, write to the Free Software Foundation, Inc.,
#   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
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

include $(auxdir)/jfrules.mk

# This is only needed if there is a configure.in in this directory
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

SUBDIRS= @SUBDIRS@

if SYNTH_SW
  SW_SYNTH_SUBDIR = synthesis-sw
  include $(pkg_synthesis_sw_builddir)/mk/Rules-Synth-II.mk
endif
if SYNTH_PTHREAD
  PTHREAD_SYNTH_SUBDIR = synthesis-pthread
  include $(pkg_synthesis_pthread_builddir)/mk/Rules-Synth-II.mk
endif
if SYNTH_CYN
  CYN_SYNTH_SUBDIR = synthesis-cyn
  include $(pkg_synthesis_cynthesizer_builddir)/mk/Rules-Synth-II.mk
endif
if SYNTH_TLM
  TLM_SYNTH_SUBDIR = synthesis-tlm
  include $(pkg_synthesis_tlm_builddir)/mk/Rules-Synth-II.mk
endif

AM_CPPFLAGS= @AM_CPPFLAGS@ @SYSTEMOC_INCLUDE@
AM_LDFLAGS = @AM_LDFLAGS@  @SYSTEMOC_LDFLAGS@

noinst_PROGRAMS= \
	test-cluster-example \
	test-dynamic-fsm \
	test-event \
	test-fir \
	test-fsm-and-composition \
	test-fsm-and-composition-diss-falk \
	test-fsm-junction-state \
	test-fsm-xor-composition \
	test-fsm-xor-composition-diss-falk \
	test-getValueAt \
	test-periodic \
	test-sdf \
	test-select \
	test-sqr \
	test-sqr-int \
	test-src-snk \
	test-src-snk-hierarchy \
	test-src-snk-multicast \
	test-src-snk-multiplex \
	test-src-snk-multireader \
	test-src-snk-multireader-sched \
	test-src-snk-multiwriter \
	test-src-snk-noconsume \
	test-src-snk-void \
	test-systemc-blocking-cosim \
	test-systemc-nonblocking-cosim

test_cluster_example_LDADD			= $(SYSTEMOC_LIBS)
test_cluster_example_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_cluster_example_SOURCES			= \
	src/test-cluster-example/cpp/cluster_ex.cpp

test_dynamic_fsm_LDADD				= $(SYSTEMOC_LIBS)
test_dynamic_fsm_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_dynamic_fsm_SOURCES			= \
	src/test-dynamic-fsm/cpp/design-dynamic-fsm.cpp

test_event_LDADD				= $(SYSTEMOC_LIBS)
test_event_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES)
test_event_SOURCES				= \
	src/test-event/cpp/design-event.cpp

# More integration required! check and synthesis
test_fir_LDADD					= $(SYSTEMOC_LIBS)
test_fir_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES)
test_fir_SOURCES				= \
	src/test-fir/cpp/design-fir.cpp

# More integration required! check
test_fsm_and_composition_LDADD			= $(SYSTEMOC_LIBS)
test_fsm_and_composition_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_fsm_and_composition_SOURCES		= \
	src/test-fsm-and-composition/cpp/design-and-comp.cpp

# More integration required! check
test_fsm_and_composition_diss_falk_LDADD	= $(SYSTEMOC_LIBS)
test_fsm_and_composition_diss_falk_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_fsm_and_composition_diss_falk_SOURCES	= \
	src/test-fsm-and-composition-diss-falk/cpp/design-and-comp.cpp

# More integration required! check
test_fsm_junction_state_LDADD			= $(SYSTEMOC_LIBS)
test_fsm_junction_state_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_fsm_junction_state_SOURCES			= \
	src/test-fsm-junction-state/cpp/design-junction-state.cpp

# More integration required! check
test_fsm_xor_composition_LDADD			= $(SYSTEMOC_LIBS)
test_fsm_xor_composition_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_fsm_xor_composition_SOURCES		= \
	src/test-fsm-xor-composition/cpp/design-xor-comp.cpp

# More integration required! check
test_fsm_xor_composition_diss_falk_LDADD	= $(SYSTEMOC_LIBS)
test_fsm_xor_composition_diss_falk_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_fsm_xor_composition_diss_falk_SOURCES	= \
	src/test-fsm-xor-composition-diss-falk/cpp/design-xor-comp.cpp

test_getValueAt_LDADD				= $(SYSTEMOC_LIBS)
test_getValueAt_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_getValueAt_SOURCES				= \
	src/test-getValueAt/cpp/design-getValueAt.cpp

test_periodic_LDADD				= $(SYSTEMOC_LIBS)
test_periodic_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_periodic_SOURCES				= \
	src/test-periodic/cpp/design-periodic.cpp

test_sdf_LDADD					= $(SYSTEMOC_LIBS)
test_sdf_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES)
test_sdf_SOURCES				= \
	src/test-sdf/cpp/design-sdf.cpp

test_select_LDADD				= $(SYSTEMOC_LIBS)
test_select_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_select_SOURCES				= \
	src/test-select/cpp/design-select.cpp

# More integration required! check and synthesis
test_sqr_LDADD					= $(SYSTEMOC_LIBS)
test_sqr_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES)
test_sqr_SOURCES				= \
	src/test-sqr/cpp/design-sqr.cpp

# More integration required! check and synthesis
test_sqr_int_LDADD				= $(SYSTEMOC_LIBS)
test_sqr_int_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_sqr_int_SOURCES				= \
	src/test-sqr-int/cpp/design-sqr.cpp

# More integration required! Maybe check for VPC?
test_src_snk_LDADD				= $(SYSTEMOC_LIBS)
test_src_snk_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_SOURCES				= \
	src/test-src-snk/cpp/design-src-snk.cpp

test_src_snk_hierarchy_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_hierarchy_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_hierarchy_SOURCES			= \
	src/test-src-snk-hierarchy/cpp/design-src-snk-hierarchy.cpp

test_src_snk_multicast_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multicast_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multicast_SOURCES			= \
	src/test-src-snk-multicast/cpp/design-src-snk-multicast.cpp

# More integration required! check
test_src_snk_multiplex_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multiplex_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multiplex_SOURCES			= \
	src/test-src-snk-multiplex/cpp/design-src-snk-multiplex.cpp

test_src_snk_multireader_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multireader_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multireader_SOURCES		= \
	src/test-src-snk-multireader/cpp/design-src-snk-multireader.cpp

test_src_snk_multireader_sched_LDADD		= $(SYSTEMOC_LIBS)
test_src_snk_multireader_sched_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multireader_sched_SOURCES		= \
	src/test-src-snk-multireader-sched/cpp/design-src-snk-multireader-sched.cpp

test_src_snk_multiwriter_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multiwriter_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multiwriter_SOURCES		= \
	src/test-src-snk-multiwriter/cpp/design-src-snk-multiwriter.cpp

# More integration required! check
test_src_snk_noconsume_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_noconsume_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_noconsume_SOURCES			= \
	src/test-src-snk-noconsume/cpp/design-src-snk-noconsume.cpp

test_src_snk_void_LDADD				= $(SYSTEMOC_LIBS)
test_src_snk_void_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_void_SOURCES			= \
	src/test-src-snk-void/cpp/design-src-snk-void.cpp

test_systemc_blocking_cosim_LDADD		= $(SYSTEMOC_LIBS)
test_systemc_blocking_cosim_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_systemc_blocking_cosim_SOURCES		= \
	src/test-systemc-blocking-cosim/cpp/design-cosim-blocking.cpp

test_systemc_nonblocking_cosim_LDADD		= $(SYSTEMOC_LIBS)
test_systemc_nonblocking_cosim_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_systemc_nonblocking_cosim_SOURCES		= \
	src/test-systemc-nonblocking-cosim/cpp/design-cosim-nonblocking.cpp
