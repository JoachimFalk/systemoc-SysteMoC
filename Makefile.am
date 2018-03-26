## Process this file with automake to produce Makefile.in
#
# Copyright (c) 2004-2018 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

SUBDIRS= @SUBDIRS@

AUTOMAKE_OPTIONS = nostdinc
# This is only needed if there is a configure.ac in this directory
ACLOCAL_AMFLAGS=-I ../BuildSystem/m4

if SYNTH_SW
  SW_SYNTH_SUBDIR     = synthesis-sw
  SW_SYNTH_INST_FLAGS = --no-preprocess-macros
  -include $(pkg_synthesis_sw_builddir)/mk/Rules-Synth-II.mk
endif
if SYNTH_PTHREAD
  PTHREAD_SYNTH_SUBDIR     = synthesis-pthread
  PTHREAD_SYNTH_INST_FLAGS = --no-preprocess-macros
  -include $(pkg_synthesis_pthread_builddir)/mk/Rules-Synth-II.mk
endif
if SYNTH_TLM
  TLM_SYNTH_SUBDIR     = synthesis-tlm
  TLM_SYNTH_INST_FLAGS = --no-preprocess-macros
  -include $(pkg_synthesis_tlm_builddir)/mk/Rules-Synth-II.mk
endif
if SYNTH_VIVADOHLS
  VIVADOHLS_SYNTH_SUBDIR     = synthesis-vivadohls
  VIVADOHLS_SYNTH_INST_FLAGS = --no-preprocess-macros
  -include $(pkg_synthesis_vivadohls_builddir)/mk/Rules-Synth-II.mk
endif
if CLUSTERING
  -include $(pkg_clustering_builddir)/mk/Rules-Synth-II.mk
endif
if SGXUTILS
  NGX_TO_DOT=$(pkg_sgxutils_builddir)/ngx-to-dot
endif

AM_CPPFLAGS= @AM_CPPFLAGS@ $(SYSTEMOC_INCLUDE) $(BOOST_INCLUDE)
AM_LDFLAGS = @AM_LDFLAGS@  $(SYSTEMOC_LDFLAGS) $(BOOST_LDFLAGS)

noinst_PROGRAMS= \
	test-cluster-example \
	test-event \
	test-fir \
	test-fir-nostl \
	test-fsm-action-parameters \
	test-fsm-action-sequences \
	test-fsm-dynamic \
	test-fsm-hierarchy-and \
	test-fsm-hierarchy-and-diss-falk \
	test-fsm-hierarchy-xor \
	test-fsm-hierarchy-xor-diss-falk \
	test-fsm-junction-state \
	test-getValueAt \
	test-periodic \
	test-sdf \
	test-select \
	test-sequence-alttoksize \
	test-sqr \
	test-sqr-int \
	test-src-snk-hierarchy \
	test-src-snk-multicast \
	test-src-snk-multiplex \
	test-src-snk-multirate \
	test-src-snk-multireader \
	test-src-snk-multiwriter \
	test-src-snk-noconsume \
	test-src-snk-plain \
	test-src-snk-references \
	test-src-snk-register \
	test-src-snk-void \
	test-systemc-blocking-cosim \
	test-systemc-nonblocking-cosim \
	test-systemc-sqr

%.smx: %
	./'$<' --systemoc-export-smx='$@' || \
	  { rm -f "$@"; exit 1; }
if SGXUTILS
%-fsms.dot: %.smx
	$(NGX_TO_DOT) \
	    --fsm-of-proc-name '(top|sqrroot|g|graph)\..*' \
	    --fsm-dump-state-names yes \
	    --ngx '$<' --dot '$@' || \
	  { rm -f "$@"; exit 1; }
endif
	
if SYNTH_SW
if CLUSTERING
test-%.clu: src/test-%/clustering.clu
	ln -sf '$<' '$@'
endif
endif
if SYNTH_PTHREAD
test-%-pthread.map: src/test-%/synth-pthread.map
	ln -sf '$<' '$@'
if CLUSTERING
test-%-after-binding.clu: src/test-%/clustering-after-binding.clu
	ln -sf '$<' '$@'
endif
endif

TEST_LOGS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/golden%.log,$(TEST)%.log,$(wildcard $(srcdir)/src/$(TEST)/golden*.log)))
TEST_FSMS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/fsms.dot,$(TEST)-fsms.dot,$(wildcard $(srcdir)/src/$(TEST)/fsms.dot)))
TEST_VPC_LOGS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/vpc/%.vpc.xml,$(TEST)-%.log,$(wildcard $(srcdir)/src/$(TEST)/vpc/*.vpc.xml)))
TEST_VPC_VCDS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/vpc/%-golden.vcd,$(TEST)-%.vcd,$(wildcard $(srcdir)/src/$(TEST)/vpc/*-golden.vcd)))

test_cluster_example_LDADD			= $(SYSTEMOC_LIBS)
test_cluster_example_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_cluster_example_SOURCES			= \
	src/test-cluster-example/cpp/cluster_ex.cpp

test-cluster-example.log: test-cluster-example
	./'$<' > "$@" || { rm -f "$@"; exit 1; }

test_event_LDADD				= $(SYSTEMOC_LIBS)
test_event_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES)
test_event_SOURCES				= \
	src/test-event/cpp/design-event.cpp

test-event.log: test-event
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

# More integration required! check and synthesis
test_fir_LDADD					= $(SYSTEMOC_LIBS)
test_fir_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES)
test_fir_SOURCES				= \
	src/test-fir/cpp/design-fir.cpp

test-fir.log: test-fir
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

# Needed by all synthesis targets for sqr
test-fir.cpp: $(test_fir_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@
# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-fir synth-pthread-fir synth-vivadohls-fir
if SYNTH_SW
SW_SYNTH_TEST_FIR_CPPFLAGS = -I$(srcdir)/src/test-fir/cpp
synth-sw-fir: test-fir-$(SW_SYNTH_SUBDIR)/top
endif
if SYNTH_PTHREAD
PTHREAD_SYNTH_TEST_FIR_CPPFLAGS = -I$(srcdir)/src/test-fir/cpp
synth-pthread-fir: test-fir-$(PTHREAD_SYNTH_SUBDIR)/top
endif
if SYNTH_VIVADOHLS
VIVADOHLS_SYNTH_TEST_FIR_CPPFLAGS = -I$(srcdir)/src/test-fir/cpp
synth-vivadohls-fir: test-fir-$(VIVADOHLS_SYNTH_SUBDIR)/top
endif

# More integration required! check and synthesis
test_fir_nostl_LDADD				= $(SYSTEMOC_LIBS)
test_fir_nostl_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_fir_nostl_SOURCES				= \
	src/test-fir-nostl/cpp/design-fir-nostl.cpp

test-fir-nostl.log: test-fir-nostl
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

# Needed by all synthesis targets for sqr
test-fir-nostl.cpp: $(test_fir_nostl_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@
# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-fir-nostl synth-vivadohls-fir-nostl synth-pthread-fir-nostl
if SYNTH_SW
SW_SYNTH_TEST_FIR_NOSTL_CPPFLAGS = -I$(srcdir)/src/test-fir-nostl/cpp
synth-sw-fir-nostl: test-fir-nostl-$(SW_SYNTH_SUBDIR)/top
endif

if SYNTH_VIVADOHLS
VIVADOHLS_SYNTH_TEST_FIR_NOSTL_CPPFLAGS = -I$(srcdir)/src/test-fir-nostl/cpp
synth-vivadohls-fir-nostl: test-fir-nostl-$(VIVADOHLS_SYNTH_SUBDIR)/top
endif

if SYNTH_PTHREAD
PTHREAD_SYNTH_TEST_FIR_NOSTL_CPPFLAGS = -I$(srcdir)/src/test-fir-nostl/cpp
synth-pthread-fir-nostl: test-fir-nostl-$(PTHREAD_SYNTH_SUBDIR)/top
endif

test_fsm_action_parameters_LDADD		= $(SYSTEMOC_LIBS)
test_fsm_action_parameters_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_fsm_action_parameters_SOURCES		= \
	src/test-fsm-action-parameters/cpp/design-fsm-action-parameters.cpp

test-fsm-action-parameters.log: test-fsm-action-parameters
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-fsm-action-parameters-%.log: test-fsm-action-parameters.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

test_fsm_action_sequences_LDADD			= $(SYSTEMOC_LIBS)
test_fsm_action_sequences_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_fsm_action_sequences_SOURCES		= \
	src/test-fsm-action-sequences/cpp/design-fsm-action-sequences.cpp

test-fsm-action-sequences.log: test-fsm-action-sequences
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-fsm-action-sequences-%.log: test-fsm-action-sequences.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

test_fsm_dynamic_LDADD				= $(SYSTEMOC_LIBS)
test_fsm_dynamic_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_fsm_dynamic_SOURCES			= \
	src/test-fsm-dynamic/cpp/design-fsm-dynamic.cpp

test-fsm-dynamic.log: test-fsm-dynamic
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-fsm-dynamic-%.log: test-fsm-dynamic.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

# More integration required! check
test_fsm_hierarchy_and_LDADD			= $(SYSTEMOC_LIBS)
test_fsm_hierarchy_and_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_fsm_hierarchy_and_SOURCES			= \
	src/test-fsm-hierarchy-and/cpp/design-fsm-hierarchy-and.cpp

test-fsm-hierarchy-and.log: test-fsm-hierarchy-and
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-fsm-hierarchy-and-top.%.log: test-fsm-hierarchy-and.log
	grep 'top.$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

test_fsm_hierarchy_and_diss_falk_LDADD		= $(SYSTEMOC_LIBS) -lboost_random
test_fsm_hierarchy_and_diss_falk_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_fsm_hierarchy_and_diss_falk_SOURCES	= \
	src/test-fsm-hierarchy-and-diss-falk/cpp/design-fsm-hierarchy-and.cpp

test-fsm-hierarchy-and-diss-falk.log: test-fsm-hierarchy-and-diss-falk
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-fsm-hierarchy-and-diss-falk-top.%.log: test-fsm-hierarchy-and-diss-falk.log
	grep 'top.$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

# More integration required! check
test_fsm_hierarchy_xor_LDADD			= $(SYSTEMOC_LIBS)
test_fsm_hierarchy_xor_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_fsm_hierarchy_xor_SOURCES			= \
	src/test-fsm-hierarchy-xor/cpp/design-fsm-hierarchy-xor.cpp

test-fsm-hierarchy-xor.log: test-fsm-hierarchy-xor
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-fsm-hierarchy-xor-top.%.log: test-fsm-hierarchy-xor.log
	grep 'top.$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

test_fsm_hierarchy_xor_diss_falk_LDADD		= $(SYSTEMOC_LIBS) -lboost_random
test_fsm_hierarchy_xor_diss_falk_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_fsm_hierarchy_xor_diss_falk_SOURCES	= \
	src/test-fsm-hierarchy-xor-diss-falk/cpp/design-fsm-hierarchy-xor.cpp

test-fsm-hierarchy-xor-diss-falk.log: test-fsm-hierarchy-xor-diss-falk
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-fsm-hierarchy-xor-diss-falk-top.%.log: test-fsm-hierarchy-xor-diss-falk.log
	grep 'top.$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

test_fsm_junction_state_LDADD			= $(SYSTEMOC_LIBS)
test_fsm_junction_state_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_fsm_junction_state_SOURCES			= \
	src/test-fsm-junction-state/cpp/design-junction-state.cpp

test-fsm-junction-state.log: test-fsm-junction-state
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

test_getValueAt_LDADD				= $(SYSTEMOC_LIBS)
test_getValueAt_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_getValueAt_SOURCES				= \
	src/test-getValueAt/cpp/design-getValueAt.cpp

test-getValueAt.log: test-getValueAt
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-getValueAt-%.log: test-getValueAt.log
	grep '^$* ' '$<' > '$@' || { rm -f "$@"; exit 1; }

test_periodic_LDADD				= $(SYSTEMOC_LIBS)
test_periodic_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_periodic_SOURCES				= \
	src/test-periodic/cpp/design-periodic.cpp

test-periodic.log: test-periodic
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

test_sdf_LDADD					= $(SYSTEMOC_LIBS)
test_sdf_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES)
test_sdf_SOURCES				= \
	src/test-sdf/cpp/design-sdf.cpp

test-sdf.log: test-sdf
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-sdf-%.log: test-sdf.log
	grep '$* ' '$<' > '$@' || { rm -f "$@"; exit 1; }

test_select_LDADD				= $(SYSTEMOC_LIBS)
test_select_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_select_SOURCES				= \
	src/test-select/cpp/design-select.cpp

test-select.log: test-select
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-select-%.log: test-select.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

# More integration required! check and synthesis
test_sequence_alttoksize_LDADD			= $(SYSTEMOC_LIBS)
test_sequence_alttoksize_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_sequence_alttoksize_SOURCES		= \
	src/test-sequence-alttoksize/cpp/design-test-sequence-alttoksize.cpp

test-sequence-alttoksize.log: test-sequence-alttoksize
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

test-sequence-alttoksize.cpp: $(test_sequence_alttoksize_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@

# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-sequence-alttoksize synth-vivadohls-sequence-alttoksize
if SYNTH_SW
SW_SYNTH_TEST_SEQUENCE_ALTTOKSIZE_CPPFLAGS = -I$(srcdir)/src/test-sequence-alttoksize/cpp
synth-sw-sequence-alttoksize: test-sequence-alttoksize-$(SW_SYNTH_SUBDIR)/top
endif

if SYNTH_VIVADOHLS
VIVADOHLS_SYNTH_TEST_SEQUENCE_ALTTOKSIZE_CPPFLAGS = -I$(srcdir)/src/test-sequence-alttoksize/cpp
synth-vivadohls-sequence-alttoksize: test-sequence-alttoksize-$(VIVADOHLS_SYNTH_SUBDIR)/top
endif

# More integration required! check and synthesis
test_sqr_LDADD					= $(SYSTEMOC_LIBS)
test_sqr_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES)
test_sqr_SOURCES				= \
	src/test-sqr/cpp/design-sqr.cpp

test-sqr.log: test-sqr
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-sqr-sqrroot.%.log: test-sqr.log
	grep 'sqrroot.$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

# Needed by all synthesis targets for sqr
test-sqr.cpp: $(test_sqr_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@
# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-sqr synth-sw-sqr-clu synth-vivadohls-sqr synth-pthread-sqr synth-pthread-sqr-clu
if SYNTH_SW
SW_SYNTH_TEST_SQR_CPPFLAGS = -I$(srcdir)/src/test-sqr/cpp
synth-sw-sqr: test-sqr-$(SW_SYNTH_SUBDIR)/top
if CLUSTERING
synth-sw-sqr-clu: test-sqr-fsm-sched-$(SW_SYNTH_SUBDIR)/top
endif
endif

if SYNTH_VIVADOHLS
VIVADOHLS_SYNTH_TEST_SQR_CPPFLAGS = -I$(srcdir)/src/test-sqr/cpp
synth-vivadohls-sqr: test-sqr-$(VIVADOHLS_SYNTH_SUBDIR)/top
endif

if SYNTH_PTHREAD
PTHREAD_SYNTH_TEST_SQR_CPPFLAGS = -I$(srcdir)/src/test-sqr/cpp
synth-pthread-sqr: test-sqr-$(PTHREAD_SYNTH_SUBDIR)/top
if CLUSTERING
synth-pthread-sqr-clu: test-sqr-after-binding-fsm-sched-$(PTHREAD_SYNTH_SUBDIR)/top
endif
endif

# More integration required! check and synthesis
test_sqr_int_LDADD				= $(SYSTEMOC_LIBS)
test_sqr_int_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_sqr_int_SOURCES				= \
	src/test-sqr-int/cpp/design-sqr.cpp

test-sqr-int.log: test-sqr-int
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-sqr-int-%.log: test-sqr-int.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

# Needed by all synthesis targets for sqr-int
test-sqr-int.cpp: $(test_sqr_int_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@
# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-sqr-int synth-sw-sqr-int-clu synth-vivadohls-sqr-int synth-pthread-sqr-int synth-pthread-sqr-int-clu
if SYNTH_SW
SW_SYNTH_TEST_SQR_INT_CPPFLAGS = -I$(srcdir)/src/test-sqr-int/cpp
synth-sw-sqr-int: test-sqr-int-$(SW_SYNTH_SUBDIR)/top
if CLUSTERING
synth-sw-sqr-int-clu: test-sqr-int-fsm-sched-$(SW_SYNTH_SUBDIR)/top
endif
endif

if SYNTH_VIVADOHLS
VIVADOHLS_SYNTH_TEST_SQR_INT_CPPFLAGS = -I$(srcdir)/src/test-sqr-int/cpp
synth-vivadohls-sqr-int: test-sqr-int-$(VIVADOHLS_SYNTH_SUBDIR)/top
endif

if SYNTH_PTHREAD
PTHREAD_SYNTH_TEST_SQR_INT_CPPFLAGS = -I$(srcdir)/src/test-sqr-int/cpp
synth-pthread-sqr-int: test-sqr-int-$(PTHREAD_SYNTH_SUBDIR)/top
if CLUSTERING
synth-pthread-sqr-int-clu: test-sqr-int-after-binding-fsm-sched-$(PTHREAD_SYNTH_SUBDIR)/top
endif
endif

test_src_snk_hierarchy_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_hierarchy_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_hierarchy_SOURCES			= \
	src/test-src-snk-hierarchy/cpp/design-src-snk-hierarchy.cpp

test-src-snk-hierarchy.log: test-src-snk-hierarchy
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

test_src_snk_multicast_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multicast_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multicast_SOURCES			= \
	src/test-src-snk-multicast/cpp/design-src-snk-multicast.cpp

test-src-snk-multicast.log: test-src-snk-multicast
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-src-snk-multicast-%.log: test-src-snk-multicast.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

# More integration required! check
test_src_snk_multiplex_LDADD			= $(SYSTEMOC_LIBS) -lboost_random
test_src_snk_multiplex_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multiplex_SOURCES			= \
	src/test-src-snk-multiplex/cpp/design-src-snk-multiplex.cpp

test-src-snk-multiplex.log: test-src-snk-multiplex
	./'$<' 1000 10 20 5 4711 > "$@" || { rm -f "$@"; exit 1; }
test-src-snk-multiplex-%.log: test-src-snk-multiplex.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

# More integration required! Maybe check for VPC?
test_src_snk_multirate_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multirate_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multirate_SOURCES			= \
	src/test-src-snk-multirate/cpp/design-src-snk-multirate.cpp

test-src-snk-multirate.log: test-src-snk-multirate
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

test_src_snk_multireader_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multireader_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multireader_SOURCES		= \
	src/test-src-snk-multireader/cpp/design-src-snk-multireader.cpp

test-src-snk-multireader.log: test-src-snk-multireader
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

test_src_snk_multiwriter_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multiwriter_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multiwriter_SOURCES		= \
	src/test-src-snk-multiwriter/cpp/design-src-snk-multiwriter.cpp

test-src-snk-multiwriter.log: test-src-snk-multiwriter
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-src-snk-multiwriter-%.log: test-src-snk-multiwriter.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

# More integration required! check
test_src_snk_noconsume_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_noconsume_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_noconsume_SOURCES			= \
	src/test-src-snk-noconsume/cpp/design-src-snk-noconsume.cpp

test-src-snk-noconsume.log: test-src-snk-noconsume
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

# More integration required! Maybe check for VPC?
test_src_snk_plain_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_plain_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_plain_SOURCES			= \
	src/test-src-snk-plain/cpp/design-src-snk-plain.cpp

test-src-snk-plain.log: test-src-snk-plain
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-src-snk-plain-%.log: test-src-snk-plain.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }
if VPC
test-src-snk-plain-%.log test-src-snk-plain-%-CPU.vcd: test-src-snk-plain src/test-src-snk-plain/vpc/%.vpc.xml
	set -- $^; VPCTRACEFILEPREFIX=test-src-snk-plain-$*- VPCCONFIGURATION=$$2 ./'$<' 100 > 'test-src-snk-plain-$*.log'
endif

# More integration required! Maybe check for VPC?
test_src_snk_references_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_references_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_references_SOURCES			= \
	src/test-src-snk-references/cpp/design-src-snk-references.cpp 

# Needed by all synthesis targets for src-snk-ref
test-src-snk-references.cpp: $(test_src_snk_references_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@
# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-src-snk-ref synth-pthread-src-snk-ref
if SYNTH_SW
SW_SYNTH_TEST_SRC_SNK_REFERENCES_CPPFLAGS = -I$(srcdir)/src/test-src-snk-references/cpp
synth-sw-src-snk-ref: test-src-snk-references-$(SW_SYNTH_SUBDIR)/top
endif
if SYNTH_PTHREAD
PTHREAD_SYNTH_TEST_SRC_SNK_REFERENCES_CPPFLAGS = -I$(srcdir)/src/test-src-snk-references/cpp
synth-pthread-src-snk-ref: test-src-snk-references-$(PTHREAD_SYNTH_SUBDIR)/top
endif

test_src_snk_register_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_register_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_register_SOURCES			= \
	src/test-src-snk-register/cpp/design-src-snk-register.cpp

test-src-snk-register.log: test-src-snk-register
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-src-snk-register-%.log: test-src-snk-register.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

test_src_snk_void_LDADD				= $(SYSTEMOC_LIBS)
test_src_snk_void_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_void_SOURCES			= \
	src/test-src-snk-void/cpp/design-src-snk-void.cpp

test-src-snk-void.log: test-src-snk-void
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

test_systemc_blocking_cosim_LDADD		= $(SYSTEMOC_LIBS)
test_systemc_blocking_cosim_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_systemc_blocking_cosim_SOURCES		= \
	src/test-systemc-blocking-cosim/cpp/design-cosim-blocking.cpp

test-systemc-blocking-cosim.log: test-systemc-blocking-cosim
	./'$<' > "$@" || { rm -f "$@"; exit 1; }
test-systemc-blocking-cosim-%.log: test-systemc-blocking-cosim.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

test_systemc_nonblocking_cosim_LDADD		= $(SYSTEMOC_LIBS)
test_systemc_nonblocking_cosim_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_systemc_nonblocking_cosim_SOURCES		= \
	src/test-systemc-nonblocking-cosim/cpp/design-cosim-nonblocking.cpp

test-systemc-nonblocking-cosim.log: test-systemc-nonblocking-cosim
	./'$<' > "$@" || { rm -f "$@"; exit 1; }
test-systemc-nonblocking-cosim-%.log: test-systemc-nonblocking-cosim.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }

test_systemc_sqr_LDADD				= $(SYSTEMC_LIBS)
test_systemc_sqr_DEPENDENCIES			= $(SYSTEMC_DEPENDENCIES)
test_systemc_sqr_SOURCES			= \
	src/test-systemc-sqr/cpp/systemc-sqr.cpp

test-systemc-sqr.log: test-systemc-sqr
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

clean-local:
	rm -f $(addsuffix -pthread.map,$(noinst_PROGRAMS))
	rm -f $(addsuffix -after-binding.clu,$(noinst_PROGRAMS))
	rm -f $(addsuffix .clu,$(noinst_PROGRAMS))
	rm -f $(addsuffix .log,$(noinst_PROGRAMS))
	rm -f $(addsuffix .cpp,$(noinst_PROGRAMS))
	rm -f $(addsuffix .smx,$(noinst_PROGRAMS))
	rm -f $(TEST_LOGS) $(TEST_FSMS) $(TEST_VPC_VCDS) $(TEST_VPC_LOGS)

.PHONY: test-logs check-logs
check-am: check-logs
test-logs: $(TEST_LOGS)
check-logs: $(TEST_LOGS)
	@error=0; \
	for test in $(noinst_PROGRAMS); do \
	  case "$$test" in \
	    test-*) \
	      if test -f $(srcdir)/src/$${test}/golden.log; then \
		if ! diff -u $${test}.log $(srcdir)/src/$${test}/golden.log > /dev/null; then \
		  echo "Check for $${test} failed!!!"; error=1; \
		  diff -u $${test}.log $(srcdir)/src/$${test}/golden.log; \
		fi; \
	      else \
		status=success; \
		shopt -s nullglob; \
		for log in $(srcdir)/src/$${test}/golden-*.log; do \
		  if ! diff -u $${test}-$${log##*/golden-} $${log} > /dev/null; then \
		    status=failed; \
		  fi; \
		done; \
		if test x"$$status" = x"failed"; then \
		  echo "Check for $${test} failed!!!"; error=1; \
		  for log in $(srcdir)/src/$${test}/golden-*.log; do \
		    diff -u $${test}-$${log##*/golden-} $${log}; \
		  done; \
		fi; \
	      fi; \
	      ;; \
 	  esac; \
	done; \
	exit $$error


# .PHONY is outside conditionals to make automake happy
.PHONY: test-fsms check-fsms
if SGXUTILS

check-am: check-fsms
test-fsms: $(TEST_FSMS)
check-fsms: $(TEST_FSMS)
	@error=0; \
	for test in $(noinst_PROGRAMS); do \
	  case "$$test" in \
	    test-*) \
	      test_fsms=$${test}-fsms.dot; \
	      golden_fsms=$(srcdir)/src/$${test}/fsms.dot; \
	      if test -f $${golden_fsms}; then \
		status=success; \
		if ! diff -u $${test_fsms} $${golden_fsms} > /dev/null; then \
		  status=failed; \
		fi; \
		if test x"$$status" = x"failed"; then \
		  echo "Check for $${test}-fsms failed!!!"; error=1; \
		  diff -u $${test_fsms} $${golden_fsms}; \
		fi; \
	      fi; \
	      ;; \
 	  esac; \
	done; \
	exit $$error
endif

# .PHONY is outside conditionals to make automake happy
.PHONY: test-vcds check-vcds
if VPC

diff_vcd_ignore_date=-I '[[:blank:]]*[[:alpha:]]\{3\}[[:blank:]][[:digit:]]\{2\},[[:blank:]][[:digit:]]\{4\}[[:blank:]]*[[:digit:]]\{1,2\}:[[:digit:]]\{2\}:[[:digit:]]\{2\}$$'
diff_vcd_ignore_systemc_version=-I 'SystemC[[:blank:]][[:digit:].]\+\(-Accellera\)\?[[:blank:]]---[[:blank:]]'
diff_vcd_ignore=$(diff_vcd_ignore_date) $(diff_vcd_ignore_systemc_version)

check-am: check-vcds
test-vcds: $(TEST_VPC_VCDS)
check-vcds: $(TEST_VPC_VCDS)
	@error=0; \
	for test in $(noinst_PROGRAMS); do \
	  case "$$test" in \
	    test-*) \
	      for cmx in $(srcdir)/src/$${test}/vpc/*.vpc.xml; do \
		cmx=$${cmx##*/}; cmx=$${cmx%%.vpc.xml}; \
		status=success; \
		for golden_vcd in $(srcdir)/src/$${test}/vpc/$${cmx}-*-golden.vcd; do \
		  test_vcd=$${golden_vcd##*/}; test_vcd=$${test}-$${test_vcd%%-golden.vcd}.vcd; \
		  if test -f $${golden_vcd} && ! diff -u $(diff_vcd_ignore) $${test_vcd} $${golden_vcd} > /dev/null; then \
		    status=failed; \
		  fi; \
		done; \
		if test x"$$status" = x"failed"; then \
		  echo "Check for $${test}-$${cmx} failed!!!"; error=1; \
		  for golden_vcd in $(srcdir)/src/$${test}/vpc/$${cmx}-*-golden.vcd; do \
		    test_vcd=$${golden_vcd##*/}; test_vcd=$${test}-$${test_vcd%%-golden.vcd}.vcd; \
		    diff -u $(diff_vcd_ignore) $${test_vcd} $${golden_vcd}; \
		  done; \
		fi; \
	      done; \
	      ;; \
 	  esac; \
	done; \
	exit $$error
endif

# .PHONY is outside conditionals to make automake happy
.PHONY: synth
.PHONY: synth-sw synth-sw-clu
if SYNTH_SW
synth-sw: synth-sw-fir synth-sw-sqr synth-sw-sqr-int synth-sw-src-snk-ref
synth: synth-sw
if CLUSTERING
synth-sw-clu: synth-sw-sqr-clu synth-sw-sqr-int-clu
synth: synth-sw-clu
endif
endif
# .PHONY is outside conditionals to make automake happy
.PHONY: synth
.PHONY: synth-vivadohls
if SYNTH_VIVADOHLS
synth-vivadohls: synth-vivadohls-fir synth-vivadohls-fir-nostl synth-vivadohls-sqr synth-vivadohls-sqr-int synth-vivadohls-sequence-alttoksize
synth: synth-vivadohls
endif

# .PHONY is outside conditionals to make automake happy
.PHONY: synth-pthread 
if SYNTH_PTHREAD
synth-pthread: synth-pthread-fir synth-pthread-sqr synth-pthread-sqr-int synth-pthread-src-snk-ref
synth: synth-pthread
if CLUSTERING
synth-pthread-clu: synth-pthread-sqr-clu synth-pthread-sqr-int-clu
synth: synth-pthread-clu
endif
endif
