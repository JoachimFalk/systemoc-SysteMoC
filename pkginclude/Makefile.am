## Process this file with automake to produce Makefile.in
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (C) Joachim Falk <joachim.falk@gmx.de> $Date: 2003/01/15 16:43:20 $
#
# Makefile.am is part of the jflib distribution of Joachim Falk;
# you can redistribute it and/or modify it under the terms of the
# GNU General Public License as published by the Free Software Foundation;
# either version 2 of the License, or (at your option) any later version.
#
# The jflib distributio is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; If not, write to
# the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# $Log$

include $(auxdir)/jfrules.mk

SUBDIRS=

nobase_include_HEADERS = \
	systemoc/smoc_config.h \
	\
	systemoc/detail/smoc_event_decls.hpp \
	systemoc/detail/smoc_event_expr.hpp \
	systemoc/detail/smoc_sysc_port.hpp \
	systemoc/detail/smoc_root_port.hpp \
	systemoc/detail/EventMapManager.hpp \
	systemoc/detail/smoc_latency_queues.hpp \
	systemoc/detail/smoc_ring_access.hpp \
	systemoc/detail/smoc_fifo_storage.hpp \
	systemoc/detail/smoc_port_registry.hpp \
	systemoc/detail/QueueRWPtr.hpp \
	systemoc/detail/QueueRVWPtr.hpp \
	systemoc/detail/QueueFRVWPtr.hpp \
	systemoc/smoc_event.hpp \
	systemoc/smoc_expr.hpp \
	systemoc/smoc_elab_and_sim.hpp \
	systemoc/smoc_ast_common.hpp \
	systemoc/hscd_tdsim_TraceLog.hpp \
	systemoc/smoc_ast_systemoc.hpp \
	systemoc/smoc_chan_if.hpp \
	systemoc/smoc_chan_adapter.hpp \
	systemoc/detail/smoc_root_chan.hpp \
	systemoc/smoc_port.hpp \
	systemoc/smoc_multiplex_fifo.hpp \
	systemoc/smoc_multireader_fifo.hpp \
	systemoc/smoc_fifo.hpp \
	systemoc/smoc_firing_rules.hpp \
	systemoc/smoc_graph_synth.hpp \
	systemoc/smoc_graph_type.hpp \
	systemoc/smoc_guard.hpp \
	systemoc/smoc_md_array.hpp \
	systemoc/smoc_md_array_access.hpp \
	systemoc/smoc_md_ba_basic_tree.hpp \
	systemoc/smoc_md_ba_linearized_buffer.hpp \
	systemoc/smoc_md_ba_linearized_buffer_schedule.hpp \
	systemoc/smoc_md_ba_linearized_buffer_size.hpp \
	systemoc/smoc_md_buffer.hpp \
	systemoc/smoc_md_buffer_analysis.hpp \
	systemoc/smoc_md_chan_if.hpp \
	systemoc/smoc_md_fifo.hpp \
	systemoc/smoc_md_loop.hpp \
	systemoc/smoc_md_port.hpp \
	systemoc/smoc_moc.hpp \
	systemoc/smoc_multicast_sr_signal.hpp \
	systemoc/smoc_node_types.hpp \
	systemoc/smoc_pggen.hpp \
	systemoc/smoc_root_node.hpp \
	systemoc/smoc_sr_signal.hpp \
	systemoc/smoc_storage.hpp \
	systemoc/smoc_vector.hpp \
	systemoc/smoc_wsdf_edge.hpp \
	systemoc/smoc_ngx_sync.hpp

if SYSTEMOC_ENABLE_WSDF
nobase_include_HEADERS += \
	systemoc/smoc_md_array_access.hpp \
	systemoc/smoc_md_array.hpp \
	systemoc/smoc_md_chan_if.hpp \
	systemoc/smoc_md_buffer.hpp \
	systemoc/smoc_md_fifo.hpp \
	systemoc/smoc_md_loop.hpp \
	systemoc/smoc_md_port.hpp \
	systemoc/smoc_vector.hpp \
	systemoc/smoc_wsdf_edge.hpp \
	systemoc/smoc_md_ba_basic_tree.hpp \
	systemoc/smoc_md_buffer_analysis.hpp \
	systemoc/smoc_md_ba_linearized_buffer.hpp \
	systemoc/smoc_md_ba_linearized_buffer_size.hpp \
	systemoc/smoc_md_ba_linearized_buffer_schedule.hpp
endif

