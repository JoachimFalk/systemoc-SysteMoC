//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#ifndef _INCLUDED_SMOC_MOC_HPP
#define _INCLUDED_SMOC_MOC_HPP

#include <list>

#include <systemc.h>

#include <systemoc/smoc_config.h>

#include "smoc_firing_rules.hpp"
#include "smoc_root_node.hpp"

class smoc_graph_base;

class smoc_scheduler_top : public sc_module {
public:
  smoc_scheduler_top(smoc_graph_base* g);
  smoc_scheduler_top(smoc_graph_base& g);
  ~smoc_scheduler_top();

protected:
  void start_of_simulation();
  void end_of_simulation();
  void end_of_elaboration();

private:
  typedef CoSupport::SystemC::EventOrList
    <ExpandedTransition> smoc_transition_ready_list;

  smoc_graph_base* g;
  bool simulation_running;
  SC_HAS_PROCESS(smoc_scheduler_top);
  void schedule();

  // FIXME: move into smoc_graph_sr
  //size_t countDefinedInports(smoc_root_node & n);
  // void getLeafNodes(smoc_node_list &nodes, smoc_graph_base *node);
  //size_t countDefinedOutports(smoc_root_node & n);

  void dump();
};

// for compatibility...
typedef smoc_scheduler_top smoc_top;

// for the bold ones...
namespace SysteMoC {
  typedef smoc_scheduler_top Scheduler;
}

template <typename Graph>
class smoc_top_moc : public Graph {
public:

  // -jens-
  // FIXME: this copies given parameter and makes it impossible to use
  //   references in constructor of smoc_graph_base! Copying is evil anyway (much
  //   data, pointers, references, copy constructor has to exist, ...)

  smoc_top_moc()
    : Graph(), s(this) {}
  explicit smoc_top_moc(sc_module_name name)
    : Graph(name), s(this) {}
  template <typename T1>
  explicit smoc_top_moc(sc_module_name name, T1 p1)
    : Graph(name, p1), s(this) {}
  template <typename T1, typename T2>
  explicit smoc_top_moc(sc_module_name name, T1 p1, T2 p2)
    : Graph(name, p1, p2), s(this) {}
  template <typename T1, typename T2, typename T3>
  explicit smoc_top_moc(sc_module_name name, T1 p1, T2 p2, T3 p3)
    : Graph(name, p1, p2, p3), s(this) {}
  template <typename T1, typename T2, typename T3, typename T4>
  explicit smoc_top_moc(sc_module_name name, T1 p1, T2 p2, T3 p3, T4 p4)
    : Graph(name, p1, p2, p3, p4), s(this) {}
  template <typename T1, typename T2, typename T3, typename T4, typename T5>
  explicit smoc_top_moc(sc_module_name name, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5)
    : Graph(name, p1, p2, p3, p4, p5), s(this) {}

private:
  smoc_scheduler_top s;
};

#endif // _INCLUDED_SMOC_MOC_HPP

#include "smoc_graph_type.hpp"
