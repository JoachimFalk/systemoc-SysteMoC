# -*- tab-width:8; indent-tabs-mode:true; -*-
# vim: set noet sw=2 ts=8 sts=0:
# 
# Copyright (c)
#   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
#   2010 FAU -- Sebastian Graf <sebastian.graf@fau.de>
#   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2012 FAU -- Rafael Rosales <rafael.rosales@fau.de>
#   2013 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2014 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2016 Adnane Tayachi <adnane.tayachi@gmx.de>
#   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2017 FAU -- Franz-Josef Streit <franz-josef.streit@fau.de>
#   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2017 FAU -- Matthias Schid <matthias.schid@fau.de>
#   2017 FAU -- Simone Müller <simone.mueller@fau.de>
#   2017 FAU -- Matthias Schid <matthias.schid@fau.de>
#   2018 FAU -- Franz-Josef Streit <franz-josef.streit@fau.de>
#   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
# 
# This library is free software; you can redistribute it and/or modify it under
# the terms of the GNU Lesser General Public License as published by the Free
# Software Foundation; either version 2 of the License, or (at your option) any
# later version.
# 
# This library is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
# details.
# 
# You should have received a copy of the GNU Lesser General Public License
# along with this library; if not, write to the Free Software Foundation, Inc.,
# 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

## Process this file with automake to produce Makefile.in

include $(auxdir)/jfrules.mk

SUBDIRS=

AUTOMAKE_OPTIONS = nostdinc

if SGXUTILS
  NGX_TO_DOT=$(pkg_sgxutils_builddir)/ngx-to-dot
endif

SYSTEMOC_INCLUDE_	:= $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)"; export PKG_CONFIG_PATH; $(PKG_CONFIG) --cflags-only-I libsystemoc)
SYSTEMOC_LDFLAGS_	:= $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)"; export PKG_CONFIG_PATH; $(PKG_CONFIG) --libs-only-L   libsystemoc)
SYSTEMOC_LIBS_		:= $(filter %.la -l%, $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)"; export PKG_CONFIG_PATH; $(PKG_CONFIG) --libs libsystemoc))
SYSTEMOC_DEPENDENCIES_	:= $(filter %.la,     $(shell PKG_CONFIG_PATH="$(PKG_CONFIG_PATH)"; export PKG_CONFIG_PATH; $(PKG_CONFIG) --libs libsystemoc))
#"

#dummy:
#	@echo "SYSTEMOC_INCLUDE_:      $(SYSTEMOC_INCLUDE_)"
#	@echo "SYSTEMOC_LDFLAGS_:      $(SYSTEMOC_LDFLAGS_)"
#	@echo "SYSTEMOC_LIBS_:         $(SYSTEMOC_LIBS_)"
#	@echo "SYSTEMOC_DEPENDENCIES_: $(SYSTEMOC_DEPENDENCIES_)"

AM_CPPFLAGS= @AM_CPPFLAGS@ $(SYSTEMOC_INCLUDE_) $(BOOST_INCLUDE)
AM_LDFLAGS = @AM_LDFLAGS@  $(SYSTEMOC_LDFLAGS_) $(BOOST_LDFLAGS)

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

if SYSTEMOC_ENABLE_HOOKING
noinst_PROGRAMS+= \
	test-fsm-hooking
endif

if SGXUTILS
%.smx: %
	./'$<' --systemoc-export-smx='$@' || \
	  { rm -f "$@"; exit 1; }

%-fsms.dot: %.smx
	$(NGX_TO_DOT) \
	    --fsm-of-proc-name '(top|sqrroot|g|graph)\..*' \
	    --fsm-dump-state-names yes \
	    --ngx '$<' --dot '$@' || \
	  { rm -f "$@"; exit 1; }
endif

%.log: %
	$(srcdir)/scripts/runtest.py --log '$@' --filter '^((sqrroot|top)\.[^:]*|src|sink|A|B|Src|Snk):' -- ./'$<' 100
	
TEST_LOGS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/golden%.log,$(TEST)%.log,$(wildcard $(srcdir)/src/$(TEST)/golden*.log)))
TEST_FSMS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/fsms.dot,$(TEST)-fsms.dot,$(wildcard $(srcdir)/src/$(TEST)/fsms.dot)))

test_cluster_example_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_cluster_example_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_cluster_example_SOURCES			= \
	src/test-cluster-example/cpp/cluster_ex.cpp

test-cluster-example.log: test-cluster-example
	$(srcdir)/scripts/runtest.py --log '$@' -- ./'$<' 100

test_event_LDFLAGS				= $(SYSTEMOC_LIBS_)
test_event_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES_)
test_event_SOURCES				= \
	src/test-event/cpp/design-event.cpp

test-event.log: test-event
	$(srcdir)/scripts/runtest.py --log '$@' -- ./'$<' 100

# More integration required! check and synthesis
test_fir_LDFLAGS				= $(SYSTEMOC_LIBS_)
test_fir_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES_)
test_fir_SOURCES				= \
	src/test-fir/cpp/design-fir.cpp

# Needed by all synthesis targets for sqr
test-fir.cpp: $(test_fir_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@

# More integration required! check and synthesis
test_fir_nostl_LDFLAGS				= $(SYSTEMOC_LIBS_)
test_fir_nostl_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES_)
test_fir_nostl_SOURCES				= \
	src/test-fir-nostl/cpp/design-fir-nostl.cpp

# Needed by all synthesis targets for sqr
test-fir-nostl.cpp: $(test_fir_nostl_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@

test_fsm_action_parameters_LDFLAGS		= $(SYSTEMOC_LIBS_)
test_fsm_action_parameters_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_fsm_action_parameters_SOURCES		= \
	src/test-fsm-action-parameters/cpp/design-fsm-action-parameters.cpp

test-fsm-action-parameters.log: test-fsm-action-parameters
	$(srcdir)/scripts/runtest.py --log '$@' -- ./'$<' 100

test_fsm_action_sequences_LDFLAGS		= $(SYSTEMOC_LIBS_)
test_fsm_action_sequences_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_fsm_action_sequences_SOURCES		= \
	src/test-fsm-action-sequences/cpp/design-fsm-action-sequences.cpp

test-fsm-action-sequences.log: test-fsm-action-sequences
	$(srcdir)/scripts/runtest.py --log '$@' -- ./'$<' 100

test_fsm_dynamic_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_fsm_dynamic_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES_)
test_fsm_dynamic_SOURCES			= \
	src/test-fsm-dynamic/cpp/design-fsm-dynamic.cpp

# More integration required! check
test_fsm_hierarchy_and_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_fsm_hierarchy_and_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_fsm_hierarchy_and_SOURCES			= \
	src/test-fsm-hierarchy-and/cpp/design-fsm-hierarchy-and.cpp

test_fsm_hierarchy_and_diss_falk_LDFLAGS	= $(SYSTEMOC_LIBS_) -lboost_random
test_fsm_hierarchy_and_diss_falk_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES_)
test_fsm_hierarchy_and_diss_falk_SOURCES	= \
	src/test-fsm-hierarchy-and-diss-falk/cpp/design-fsm-hierarchy-and.cpp

# More integration required! check
test_fsm_hierarchy_xor_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_fsm_hierarchy_xor_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_fsm_hierarchy_xor_SOURCES			= \
	src/test-fsm-hierarchy-xor/cpp/design-fsm-hierarchy-xor.cpp

test_fsm_hierarchy_xor_diss_falk_LDFLAGS	= $(SYSTEMOC_LIBS_) -lboost_random
test_fsm_hierarchy_xor_diss_falk_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES_)
test_fsm_hierarchy_xor_diss_falk_SOURCES	= \
	src/test-fsm-hierarchy-xor-diss-falk/cpp/design-fsm-hierarchy-xor.cpp

test_fsm_hooking_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_fsm_hooking_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES_)
test_fsm_hooking_SOURCES			= \
	src/test-fsm-hooking/cpp/design-fsm-hooking.cpp

test-fsm-hooking.log: test-fsm-hooking
	$(srcdir)/scripts/runtest.py --log '$@' --filter '^(top.[^[]*)\[' -- ./'$<' 100

test_fsm_junction_state_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_fsm_junction_state_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_fsm_junction_state_SOURCES			= \
	src/test-fsm-junction-state/cpp/design-junction-state.cpp

test_getValueAt_LDFLAGS				= $(SYSTEMOC_LIBS_)
test_getValueAt_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES_)
test_getValueAt_SOURCES				= \
	src/test-getValueAt/cpp/design-getValueAt.cpp

test_periodic_LDFLAGS				= $(SYSTEMOC_LIBS_)
test_periodic_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES_)
test_periodic_SOURCES				= \
	src/test-periodic/cpp/design-periodic.cpp

test-periodic.log: test-periodic
	$(srcdir)/scripts/runtest.py --log '$@' -- ./'$<' 100

test_sdf_LDFLAGS				= $(SYSTEMOC_LIBS_)
test_sdf_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES_)
test_sdf_SOURCES				= \
	src/test-sdf/cpp/design-sdf.cpp

test_select_LDFLAGS				= $(SYSTEMOC_LIBS_)
test_select_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES_)
test_select_SOURCES				= \
	src/test-select/cpp/design-select.cpp

# More integration required! check and synthesis
test_sequence_alttoksize_LDFLAGS		= $(SYSTEMOC_LIBS_)
test_sequence_alttoksize_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_sequence_alttoksize_SOURCES		= \
	src/test-sequence-alttoksize/cpp/design-test-sequence-alttoksize.cpp

test-sequence-alttoksize.cpp: $(test_sequence_alttoksize_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@

# More integration required! check and synthesis
test_sqr_LDFLAGS				= $(SYSTEMOC_LIBS_)
test_sqr_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES_)
test_sqr_SOURCES				= \
	src/test-sqr/cpp/design-sqr.cpp

# Needed by all synthesis targets for sqr
test-sqr.cpp: $(test_sqr_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@

# More integration required! check and synthesis
test_sqr_int_LDFLAGS				= $(SYSTEMOC_LIBS_)
test_sqr_int_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES_)
test_sqr_int_SOURCES				= \
	src/test-sqr-int/cpp/design-sqr.cpp

# Needed by all synthesis targets for sqr-int
test-sqr-int.cpp: $(test_sqr_int_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@

test_src_snk_hierarchy_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_src_snk_hierarchy_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_src_snk_hierarchy_SOURCES			= \
	src/test-src-snk-hierarchy/cpp/design-src-snk-hierarchy.cpp

test_src_snk_multicast_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_src_snk_multicast_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_src_snk_multicast_SOURCES			= \
	src/test-src-snk-multicast/cpp/design-src-snk-multicast.cpp

# More integration required! check
test_src_snk_multiplex_LDFLAGS			= $(SYSTEMOC_LIBS_) -lboost_random
test_src_snk_multiplex_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_src_snk_multiplex_SOURCES			= \
	src/test-src-snk-multiplex/cpp/design-src-snk-multiplex.cpp

test-src-snk-multiplex.log: test-src-snk-multiplex
	$(srcdir)/scripts/runtest.py --log '$@' --filter '^(top\.[^:]*):' -- ./'$<' 1000 10 20 5 4711

test_src_snk_multirate_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_src_snk_multirate_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_src_snk_multirate_SOURCES			= \
	src/test-src-snk-multirate/cpp/design-src-snk-multirate.cpp

test_src_snk_multireader_LDFLAGS		= $(SYSTEMOC_LIBS_)
test_src_snk_multireader_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_src_snk_multireader_SOURCES		= \
	src/test-src-snk-multireader/cpp/design-src-snk-multireader.cpp

test_src_snk_multiwriter_LDFLAGS		= $(SYSTEMOC_LIBS_)
test_src_snk_multiwriter_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_src_snk_multiwriter_SOURCES		= \
	src/test-src-snk-multiwriter/cpp/design-src-snk-multiwriter.cpp

# More integration required! check
test_src_snk_noconsume_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_src_snk_noconsume_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_src_snk_noconsume_SOURCES			= \
	src/test-src-snk-noconsume/cpp/design-src-snk-noconsume.cpp

test_src_snk_plain_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_src_snk_plain_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES_)
test_src_snk_plain_SOURCES			= \
	src/test-src-snk-plain/cpp/design-src-snk-plain.cpp

test_src_snk_references_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_src_snk_references_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_src_snk_references_SOURCES			= \
	src/test-src-snk-references/cpp/design-src-snk-references.cpp 

# Needed by all synthesis targets for src-snk-ref
test-src-snk-references.cpp: $(test_src_snk_references_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@

test_src_snk_register_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_src_snk_register_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES_)
test_src_snk_register_SOURCES			= \
	src/test-src-snk-register/cpp/design-src-snk-register.cpp

test_src_snk_void_LDFLAGS			= $(SYSTEMOC_LIBS_)
test_src_snk_void_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES_)
test_src_snk_void_SOURCES			= \
	src/test-src-snk-void/cpp/design-src-snk-void.cpp

test-src-snk-void.log: test-src-snk-void
	$(srcdir)/scripts/runtest.py --log '$@' -- ./'$<' 100

test_systemc_blocking_cosim_LDFLAGS		= $(SYSTEMOC_LIBS_)
test_systemc_blocking_cosim_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES_)
test_systemc_blocking_cosim_SOURCES		= \
	src/test-systemc-blocking-cosim/cpp/design-cosim-blocking.cpp

test_systemc_nonblocking_cosim_LDFLAGS		= $(SYSTEMOC_LIBS_)
test_systemc_nonblocking_cosim_DEPENDENCIES	= $(SYSTEMOC_DEPENDENCIES_)
test_systemc_nonblocking_cosim_SOURCES		= \
	src/test-systemc-nonblocking-cosim/cpp/design-cosim-nonblocking.cpp

test_systemc_sqr_LDFLAGS			= $(SYSTEMC_LIBS)
test_systemc_sqr_DEPENDENCIES			= $(SYSTEMC_DEPENDENCIES)
test_systemc_sqr_SOURCES			= \
	src/test-systemc-sqr/cpp/systemc-sqr.cpp

clean-local:
	rm -f $(addsuffix .smx,$(noinst_PROGRAMS))
	rm -f $(TEST_LOGS) $(TEST_FSMS)

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
