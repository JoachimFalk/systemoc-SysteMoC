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

export SYSTEMOC_PLUGINPATH = @SYSTEMOC_PLUGINPATH@
SYSTEMOC_PLUGINVPC = libsystemcvpc.so

vpath $(SYSTEMOC_PLUGINVPC) $(SYSTEMOC_PLUGINPATH)

AM_CPPFLAGS= @AM_CPPFLAGS@ $(SYSTEMOC_INCLUDE) $(BOOST_INCLUDE)
AM_LDFLAGS = @AM_LDFLAGS@  $(SYSTEMOC_LDFLAGS) $(BOOST_LDFLAGS)

noinst_PROGRAMS= \
	test-fir \
	test-fir-nostl \
	test-sequence-alttoksize \
	test-sqr \
	test-sqr-int \
	test-src-snk-multicast \
	test-src-snk-multirate \
	test-src-snk-plain \
	test-src-snk-references

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

%.log: %
	$(srcdir)/scripts/runtest.py --log '$@' --filter '^((sqrroot|top)\.[^:]*|src|sink|A|B|Src|Snk):' -- ./'$<' 100
	
if SYNTH_SW
if CLUSTERING
test-%.clu: src/test-%/clustering.clu
	ln -sf '$<' '$@'
endif
endif

TEST_LOGS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/golden%.log,$(TEST)%.log,$(wildcard $(srcdir)/src/$(TEST)/golden*.log)))
TEST_FSMS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/fsms.dot,$(TEST)-fsms.dot,$(wildcard $(srcdir)/src/$(TEST)/fsms.dot)))
TEST_VPC_LOGS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/vpc/%.vpc.xml,$(TEST)-%.log,$(wildcard $(srcdir)/src/$(TEST)/vpc/*.vpc.xml)))
TEST_VPC_VCDS	= $(foreach TEST,$(noinst_PROGRAMS),$(patsubst $(srcdir)/src/$(TEST)/vpc/%-golden.vcd,$(TEST)-%.vcd,$(wildcard $(srcdir)/src/$(TEST)/vpc/*-golden.vcd)))

# More integration required! check and synthesis
test_fir_LDADD					= $(SYSTEMOC_LIBS)
test_fir_DEPENDENCIES				= $(SYSTEMOC_DEPENDENCIES)
test_fir_SOURCES				= \
	src/test-fir/cpp/design-fir.cpp

# Needed by all synthesis targets for sqr
test-fir.cpp: $(test_fir_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@
# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-fir synth-vivadohls-fir
if SYNTH_SW
SW_SYNTH_TEST_FIR_CPPFLAGS = -I$(srcdir)/src/test-fir/cpp
synth-sw-fir: test-fir-$(SW_SYNTH_SUBDIR)/top
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

# Needed by all synthesis targets for sqr
test-fir-nostl.cpp: $(test_fir_nostl_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@
# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-fir-nostl synth-vivadohls-fir-nostl
if SYNTH_SW
SW_SYNTH_TEST_FIR_NOSTL_CPPFLAGS = -I$(srcdir)/src/test-fir-nostl/cpp
synth-sw-fir-nostl: test-fir-nostl-$(SW_SYNTH_SUBDIR)/top
endif

if SYNTH_VIVADOHLS
VIVADOHLS_SYNTH_TEST_FIR_NOSTL_CPPFLAGS = -I$(srcdir)/src/test-fir-nostl/cpp
synth-vivadohls-fir-nostl: test-fir-nostl-$(VIVADOHLS_SYNTH_SUBDIR)/top
endif

# More integration required! check and synthesis
test_sequence_alttoksize_LDADD			= $(SYSTEMOC_LIBS)
test_sequence_alttoksize_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_sequence_alttoksize_SOURCES		= \
	src/test-sequence-alttoksize/cpp/design-test-sequence-alttoksize.cpp

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

# Needed by all synthesis targets for sqr
test-sqr.cpp: $(test_sqr_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@
# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-sqr synth-sw-sqr-clu synth-vivadohls-sqr
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

# More integration required! check and synthesis
test_sqr_int_LDADD				= $(SYSTEMOC_LIBS)
test_sqr_int_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_sqr_int_SOURCES				= \
	src/test-sqr-int/cpp/design-sqr.cpp

# Needed by all synthesis targets for sqr-int
test-sqr-int.cpp: $(test_sqr_int_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@
# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-sqr-int synth-sw-sqr-int-clu synth-vivadohls-sqr-int
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

test_src_snk_multicast_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multicast_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multicast_SOURCES			= \
	src/test-src-snk-multicast/cpp/design-src-snk-multicast.cpp

if VPC
test-src-snk-multicast-%.log test-src-snk-multicast-%-CPU.vcd test-src-snk-multicast-%-CPU1.vcd test-src-snk-multicast-%-CPU2.vcd test-src-snk-multicast-%-BUS.vcd test-src-snk-multicast-%-MEM.vcd test-src-snk-multicast-%-MEM1.vcd test-src-snk-multicast-%-MEM2.vcd: test-src-snk-multicast src/test-src-snk-multicast/vpc/%.vpc.xml $(SYSTEMOC_PLUGINVPC)
	set -- $^; \
	VPCTRACEFILEPREFIX=test-src-snk-multicast-$*- \
	$(srcdir)/scripts/runtest.py \
		--log 'test-src-snk-multicast-$*.log' \
		--clean 'test-src-snk-multicast-$*-*' \
		-- ./'$<' --systemoc-vpc-config "$$2" 100
endif

# More integration required! Maybe check for VPC?
test_src_snk_multirate_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_multirate_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_multirate_SOURCES			= \
	src/test-src-snk-multirate/cpp/design-src-snk-multirate.cpp

# More integration required! Maybe check for VPC?
test_src_snk_plain_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_plain_DEPENDENCIES			= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_plain_SOURCES			= \
	src/test-src-snk-plain/cpp/design-src-snk-plain.cpp

if VPC
test-src-snk-plain-%.log test-src-snk-plain-%-CPU.vcd test-src-snk-plain-%-BUS.vcd test-src-snk-plain-%-MEM.vcd: test-src-snk-plain src/test-src-snk-plain/vpc/%.vpc.xml $(SYSTEMOC_PLUGINVPC)
	set -- $^; \
	VPCTRACEFILEPREFIX=test-src-snk-plain-$*- \
	VPCCONFIGURATION="$$2" \
	$(srcdir)/scripts/runtest.py \
		--log 'test-src-snk-plain-$*.log' \
		--clean 'test-src-snk-plain-$*-*' \
		-- ./'$<' 100
endif

# More integration required! Maybe check for VPC?
test_src_snk_references_LDADD			= $(SYSTEMOC_LIBS)
test_src_snk_references_DEPENDENCIES		= $(SYSTEMOC_DEPENDENCIES)
test_src_snk_references_SOURCES			= \
	src/test-src-snk-references/cpp/design-src-snk-references.cpp 

# Needed by all synthesis targets for src-snk-ref
test-src-snk-references.cpp: $(test_src_snk_references_SOURCES)
	{ for i in $^; do echo "#include \"$$i\""; done; } > $@
# .PHONY is outside conditionals to make automake happy
.PHONY: synth-sw-src-snk-ref
if SYNTH_SW
SW_SYNTH_TEST_SRC_SNK_REFERENCES_CPPFLAGS = -I$(srcdir)/src/test-src-snk-references/cpp
synth-sw-src-snk-ref: test-src-snk-references-$(SW_SYNTH_SUBDIR)/top
endif

clean-local:
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
