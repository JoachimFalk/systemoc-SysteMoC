//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2016 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SMOC_SMOC_GRAPH_HPP
#define _INCLUDED_SMOC_SMOC_GRAPH_HPP

#include "detail/GraphBase.hpp"

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_MAESTRO
# include <Maestro/MetaMap/SMoCGraph.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO

namespace smoc {

/**
 * graph with FSM which schedules children by selecting
 * any executable transition
 */
class smoc_graph : public Detail::GraphBase
#ifdef SYSTEMOC_ENABLE_MAESTRO
, public MetaMap::SMoCGraph
#endif //SYSTEMOC_ENABLE_MAESTRO
{
public:
  // construct graph with name
  explicit smoc_graph(const sc_core::sc_module_name& name);

  // construct graph with generated name
  smoc_graph();
  
protected:
  /// @brief See GraphBase
  virtual void before_end_of_elaboration();

private:
  // graph scheduler FSM state
  smoc_firing_state run;

  // common constructor code
  void constructor();

  void initDDF();

  // schedule children of this graph
  void scheduleDDF();

  // a list containing the transitions of the graph's children
  // that may be executed

  typedef CoSupport::SystemC::EventOrList<smoc_root_node>
          smoc_node_ready_list;

  smoc_node_ready_list ol;
};

} // namespace smoc

#endif // _INCLUDED_SMOC_SMOC_GRAPH_HPP