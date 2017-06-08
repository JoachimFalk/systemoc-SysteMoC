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
if CLUSTERING
  -include $(pkg_clustering_builddir)/mk/Rules-Synth-II.mk
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
	test-systemc-sqr \
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

%.smx: %
	./'$<' --systemoc-export-smx='$@'

TEST_LOGS = $(addsuffix .log, $(noinst_PROGRAMS))
TEST_ACTOR_LOGS =

test_cluster_example_LDADD			= $(SYSTEMOC_LIBS)
test_cluster_example_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_cluster_example_SOURCES			= \
	src/test-cluster-example/cpp/cluster_ex.cpp

test-cluster-example.log: test-cluster-example
	./'$<' > "$@" || { rm -f "$@"; exit 1; }

test_dynamic_fsm_LDADD				= $(SYSTEMOC_LIBS)
test_dynamic_fsm_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_dynamic_fsm_SOURCES			= \
	src/test-dynamic-fsm/cpp/design-dynamic-fsm.cpp

test-dynamic-fsm.log: test-dynamic-fsm
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

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
.PHONY: synth-sw-fir synth-pthread-fir
if SYNTH_SW
SW_SYNTH_TEST_FIR_CPPFLAGS = -I$(srcdir)/src/test-fir/cpp
synth-sw-fir: test-fir-$(SW_SYNTH_SUBDIR)/top
endif
if SYNTH_PTHREAD
PTHREAD_SYNTH_TEST_FIR_CPPFLAGS = -I$(srcdir)/src/test-fir/cpp
synth-pthread-fir: test-fir-$(PTHREAD_SYNTH_SUBDIR)/top
endif

# More integration required! check
test_fsm_and_composition_LDADD			= $(SYSTEMOC_LIBS)
test_fsm_and_composition_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_fsm_and_composition_SOURCES		= \
	src/test-fsm-and-composition/cpp/design-and-comp.cpp

test-fsm-and-composition.log: test-fsm-and-composition
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-fsm-and-composition-%.log: test-fsm-and-composition.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }
TEST_ACTOR_LOGS += \
	test-fsm-and-composition-top.src.log \
	test-fsm-and-composition-top.snk.log \
	test-fsm-and-composition-top.transform.log

# More integration required! check
test_fsm_and_composition_diss_falk_LDADD	= $(SYSTEMOC_LIBS)
test_fsm_and_composition_diss_falk_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_fsm_and_composition_diss_falk_SOURCES	= \
	src/test-fsm-and-composition-diss-falk/cpp/design-and-comp.cpp

test-fsm-and-composition-diss-falk.log: test-fsm-and-composition-diss-falk
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

# More integration required! check
test_fsm_junction_state_LDADD			= $(SYSTEMOC_LIBS)
test_fsm_junction_state_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_fsm_junction_state_SOURCES			= \
	src/test-fsm-junction-state/cpp/design-junction-state.cpp

test-fsm-junction-state.log: test-fsm-junction-state
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

# More integration required! check
test_fsm_xor_composition_LDADD			= $(SYSTEMOC_LIBS)
test_fsm_xor_composition_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_fsm_xor_composition_SOURCES		= \
	src/test-fsm-xor-composition/cpp/design-xor-comp.cpp

test-fsm-xor-composition.log: test-fsm-xor-composition
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-fsm-xor-composition-%.log: test-fsm-xor-composition.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }
TEST_ACTOR_LOGS += \
	test-fsm-xor-composition-top.snk.log \
	test-fsm-xor-composition-top.src.log \
	test-fsm-xor-composition-top.transform.log

# More integration required! check
test_fsm_xor_composition_diss_falk_LDADD	= $(SYSTEMOC_LIBS)
test_fsm_xor_composition_diss_falk_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_fsm_xor_composition_diss_falk_SOURCES	= \
	src/test-fsm-xor-composition-diss-falk/cpp/design-xor-comp.cpp

test-fsm-xor-composition-diss-falk.log: test-fsm-xor-composition-diss-falk
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

test_getValueAt_LDADD				= $(SYSTEMOC_LIBS)
test_getValueAt_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_getValueAt_SOURCES				= \
	src/test-getValueAt/cpp/design-getValueAt.cpp

test-getValueAt.log: test-getValueAt
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

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

test_select_LDADD				= $(SYSTEMOC_LIBS)
test_select_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_select_SOURCES				= \
	src/test-select/cpp/design-select.cpp

test-select.log: test-select
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }
test-select-%.log: test-select.log
	grep '$*:' '$<' > '$@' || { rm -f "$@"; exit 1; }
TEST_ACTOR_LOGS += \
	test-select-top.select.log \
	test-select-top.sink.log \
	test-select-top.src1.log \
	test-select-top.src2.log \
	test-select-top.srcbool.log

# More integration required! check and synthesis
test_sqr_LDADD					= $(SYSTEMOC_LIBS)
test_sqr_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES)
test_sqr_SOURCES				= \
	src/test-sqr/cpp/design-sqr.cpp

test-sqr.log: test-sqr
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

# Needed by all synthesis targets for sqr
test-sqr.cpp: $(test_sqr_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@
# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-sqr synth-pthread-sqr
if SYNTH_SW
SW_SYNTH_TEST_SQR_CPPFLAGS = -I$(srcdir)/src/test-sqr/cpp
synth-sw-sqr: test-sqr-$(SW_SYNTH_SUBDIR)/top
endif
if SYNTH_PTHREAD
PTHREAD_SYNTH_TEST_SQR_CPPFLAGS = -I$(srcdir)/src/test-sqr/cpp
synth-pthread-sqr: test-sqr-$(PTHREAD_SYNTH_SUBDIR)/top
endif

test_systemc_sqr_LDADD				= $(SYSTEMC_LIBS)
test_systemc_sqr_DEPENDENCIES			= $(SYSTEMC_DEPENDENCIES)
test_systemc_sqr_SOURCES			= \
	src/test-systemc-sqr/cpp/systemc-sqr.cpp

test-systemc-sqr.log: test-systemc-sqr
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

# More integration required! check and synthesis
test_sqr_int_LDADD				= $(SYSTEMOC_LIBS)
test_sqr_int_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_sqr_int_SOURCES				= \
	src/test-sqr-int/cpp/design-sqr.cpp

test-sqr-int.log: test-sqr-int
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

# Needed by all synthesis targets for sqr-int
test-sqr-int.cpp: $(test_sqr_int_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@
# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-sqr-int synth-pthread-sqr-int
if SYNTH_SW
SW_SYNTH_TEST_SQR_INT_CPPFLAGS = -I$(srcdir)/src/test-sqr-int/cpp
synth-sw-sqr-int: test-sqr-int-$(SW_SYNTH_SUBDIR)/top
endif
if SYNTH_PTHREAD
PTHREAD_SYNTH_TEST_SQR_INT_CPPFLAGS = -I$(srcdir)/src/test-sqr-int/cpp
synth-pthread-sqr-int: test-sqr-int-$(PTHREAD_SYNTH_SUBDIR)/top
endif

# More integration required! Maybe check for VPC?
test_src_snk_LDADD				= $(SYSTEMOC_LIBS)
test_src_snk_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_SOURCES				= \
	src/test-src-snk/cpp/design-src-snk.cpp

test-src-snk.log: test-src-snk
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

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

# More integration required! check
test_src_snk_multiplex_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multiplex_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multiplex_SOURCES			= \
	src/test-src-snk-multiplex/cpp/design-src-snk-multiplex.cpp

test-src-snk-multiplex.log: test-src-snk-multiplex
	./'$<' 1000 10 20 5 0 2 > "$@" || { rm -f "$@"; exit 1; }

test_src_snk_multireader_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multireader_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multireader_SOURCES		= \
	src/test-src-snk-multireader/cpp/design-src-snk-multireader.cpp

test-src-snk-multireader.log: test-src-snk-multireader
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

test_src_snk_multireader_sched_LDADD		= $(SYSTEMOC_LIBS)
test_src_snk_multireader_sched_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multireader_sched_SOURCES		= \
	src/test-src-snk-multireader-sched/cpp/design-src-snk-multireader-sched.cpp

test-src-snk-multireader-sched.log: test-src-snk-multireader-sched
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

test_src_snk_multiwriter_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multiwriter_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multiwriter_SOURCES		= \
	src/test-src-snk-multiwriter/cpp/design-src-snk-multiwriter.cpp

test-src-snk-multiwriter.log: test-src-snk-multiwriter
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

# More integration required! check
test_src_snk_noconsume_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_noconsume_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_noconsume_SOURCES			= \
	src/test-src-snk-noconsume/cpp/design-src-snk-noconsume.cpp

test-src-snk-noconsume.log: test-src-snk-noconsume
	./'$<' 100 > "$@" || { rm -f "$@"; exit 1; }

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

test_systemc_nonblocking_cosim_LDADD		= $(SYSTEMOC_LIBS)
test_systemc_nonblocking_cosim_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES)
test_systemc_nonblocking_cosim_SOURCES		= \
	src/test-systemc-nonblocking-cosim/cpp/design-cosim-nonblocking.cpp

test-systemc-nonblocking-cosim.log: test-systemc-nonblocking-cosim
	./'$<' > "$@" || { rm -f "$@"; exit 1; }

clean-local:
	for i in $(noinst_PROGRAMS); do \
	  rm -f $${i}.cpp $${i}.smx $${i}.log; \
	done

.PHONY: logs
logs: $(TEST_LOGS) $(TEST_ACTOR_LOGS)
check-am: $(TEST_LOGS) $(TEST_ACTOR_LOGS)
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
.PHONY: synth-sw synth-pthread synth
if SYNTH_SW
synth-sw: synth-sw-fir synth-sw-sqr synth-sw-sqr-int
synth: synth-sw
endif
if SYNTH_PTHREAD
synth-pthread: synth-pthread-fir synth-pthread-sqr synth-pthread-sqr-int
.PHONY: synth-pthread
synth: synth-pthread
endif
