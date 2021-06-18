# -*- tab-width:8; indent-tabs-mode:true; -*-
# vim: set noet sw=2 ts=8 sts=0:
#
# Copyright (c)
#   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
#   2011 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2012 FAU -- Liyuan Zhang <liyuan.zhang@cs.fau.de>
#   2014 FAU -- Tobias Schwarzer <tobias.schwarzer@fau.de>
#   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2017 FAU -- Martin Letras <martin.letras@fau.de>
#   2017 FAU -- Matthias Schid <matthias.schid@fau.de>
#   2017 FAU -- Simone Müller <simone.mueller@fau.de>
#   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
#   2018 FAU -- Stefan Wildermann <stefan.wildermann@cs.fau.de>
#   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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

nobase_include_HEADERS = \
	smoc.hpp \
	smoc/smoc_action.hpp \
	smoc/smoc_actor.hpp \
	smoc/smoc_and_state.hpp \
	smoc/smoc_base_state.hpp \
	smoc/smoc_event.hpp \
	smoc/smoc_fifo.hpp \
	smoc/smoc_firing_rule.hpp \
	smoc/smoc_firing_state.hpp \
	smoc/smoc_graph.hpp \
	smoc/smoc_guard.hpp \
	smoc/smoc_hooking.hpp \
	smoc/smoc_junction_state.hpp \
	smoc/smoc_multi_state.hpp \
	smoc/smoc_periodic_actor.hpp \
	smoc/smoc_port_in.hpp \
	smoc/smoc_port_out.hpp \
	smoc/smoc_register.hpp \
	smoc/smoc_reset_port.hpp \
	smoc/smoc_scheduler_top.hpp \
	smoc/smoc_state.hpp \
	smoc/smoc_token_traits.hpp \
	smoc/smoc_transition.hpp \
	smoc/smoc_xor_state.hpp \
	smoc/SimulatorAPI/ChannelInterfaces.hpp \
	smoc/SimulatorAPI/FiringRuleInterface.hpp \
	smoc/SimulatorAPI/PortInterfaces.hpp \
	smoc/SimulatorAPI/SchedulerInterface.hpp \
	smoc/SimulatorAPI/SimulatorInterface.hpp \
	smoc/SimulatorAPI/TaskInterface.hpp \
	smoc/SimulatorAPI/TransitionInterface.hpp \
	smoc/detail/ChanBase.hpp \
	smoc/detail/ConnectProvider.hpp \
	smoc/detail/DebugOStream.hpp \
	smoc/detail/DumpingInterfaces.hpp \
	smoc/detail/EventMapManager.hpp \
	smoc/detail/export_config.h \
	smoc/detail/FifoChanBase.hpp \
	smoc/detail/FifoChan.hpp \
	smoc/detail/FifoEntry.hpp \
	smoc/detail/FifoOutlet.hpp \
	smoc/detail/FifoStorage.hpp \
	smoc/detail/Functor.hpp \
	smoc/detail/GraphBase.hpp \
	smoc/detail/IdedObj.hpp \
	smoc/detail/IdTAllocRequest.hpp \
	smoc/detail/MemFuncCallIf.hpp \
	smoc/detail/NamedIdedObj.hpp \
	smoc/detail/NodeBase.hpp \
	smoc/detail/NodeQueue.hpp \
	smoc/detail/PortBase.hpp \
	smoc/detail/PortBaseIf.hpp \
	smoc/detail/PortCommon.hpp \
	smoc/detail/PortIOBaseIf.hpp \
	smoc/detail/QueueFRVWPtr.hpp \
	smoc/detail/QueueRVWPtr.hpp \
	smoc/detail/QueueRWPtr.hpp \
	smoc/detail/QueueWithStorage.hpp \
	smoc/detail/RegisterChanBase.hpp \
	smoc/detail/RegisterChan.hpp \
	smoc/detail/RegisterEntry.hpp \
	smoc/detail/RegisterOutlet.hpp \
	smoc/detail/RegisterStorage.hpp \
	smoc/detail/RingAccess.hpp \
	smoc/detail/SimCTXBase.hpp \
	smoc/detail/Storage.hpp \
	smoc/detail/TraceLog.hpp \
	\
	systemoc/smoc_moc.hpp \
	systemoc/smoc_actor.hpp \
	systemoc/smoc_chan_adapter.hpp \
	systemoc/smoc_config.h \
	systemoc/smoc_elab_and_sim.hpp \
	systemoc/smoc_event.hpp \
	systemoc/smoc_expr.hpp \
	systemoc/smoc_fifo.hpp \
	systemoc/smoc_firing_rule.hpp \
	systemoc/smoc_firing_rules.hpp \
	systemoc/smoc_graph.hpp \
	systemoc/smoc_graph_tt.hpp \
	systemoc/smoc_graph_type.hpp \
	systemoc/smoc_multiplex_fifo.hpp \
	systemoc/smoc_multireader_fifo.hpp \
	systemoc/smoc_node_types.hpp \
	systemoc/smoc_periodic_actor.hpp \
	systemoc/smoc_port.hpp \
	systemoc/smoc_register.hpp \
	systemoc/smoc_reset.hpp \
	systemoc/smoc_scheduler_top.hpp \
	systemoc/smoc_tt.hpp \
	systemoc/detail/smoc_chan_if.hpp \
	systemoc/detail/smoc_port_registry.hpp

