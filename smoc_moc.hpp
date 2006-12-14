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

#include <systemc.h>
#include <smoc_graph_type.hpp>

#include <list>

#include <hscd_tdsim_TraceLog.hpp>

/*
class smoc_scheduler_base {
protected:
};

class smoc_scheduler_sdf
  : public smoc_root_node,
    public smoc_scheduler_base {
public:
  typedef smoc_scheduler_sdf  this_type;
  typedef smoc_sdf_constraintset  cset_ty;
protected:
  cset_ty *c;
  
  void schedule();
private:
  smoc_firing_state s;
  
  void analyse();
public:
  smoc_scheduler_sdf( cset_ty *c );
};

typedef class smoc_scheduler_ndf smoc_scheduler_ddf;

class smoc_scheduler_ndf
  : public smoc_root_node,
    public smoc_firing_types,
    public smoc_scheduler_base {
public:
  typedef smoc_scheduler_ndf      this_type;
  typedef smoc_ndf_constraintset  cset_ty;
private:
  smoc_firing_state  s;
  cset_ty           *c;
protected:
  typedef std::pair<transition_ty *, smoc_root_node *>  transition_node_ty;
  typedef std::list<transition_node_ty>                 transition_node_list_ty;
  
  const smoc_firing_state &schedule();
 
  void finalise() {
    s = smoc_activation_pattern() >> diverge(&smoc_scheduler_ndf::schedule);
    smoc_root_node::finalise();
  }
public:
  smoc_scheduler_ndf( cset_ty *c );
};
*/

class smoc_graph
  : public smoc_ndf_constraintset,
    public smoc_root_node {
//  public smoc_scheduler_ndf {
public:
  typedef smoc_graph this_type;
private:
protected:
  void finalise() {
    smoc_ndf_constraintset::finalise();
    smoc_root_node::finalise();
//  smoc_scheduler_ndf::finalise();
  }

  smoc_firing_state dummy;
public:
  explicit smoc_graph( sc_module_name name )
    : smoc_ndf_constraintset(name),
      smoc_root_node(dummy) {
    dummy = CALL(smoc_graph::finalise) >> dummy;
  }
  smoc_graph()
    : smoc_ndf_constraintset(
        sc_gen_unique_name("smoc_graph") ),
      smoc_root_node(dummy) {
    dummy = CALL(smoc_graph::finalise) >> dummy;
  }

#ifndef __SCFE__
  sc_module *myModule() { return this; }
  
  void pgAssemble( smoc_modes::PGWriter &pgw, const smoc_root_node *n ) const
    { return smoc_ndf_constraintset::pgAssemble(pgw,n); }
  void assembleActor( smoc_modes::PGWriter &pgw ) const {}
#endif

};

class smoc_scheduler_top
  : public smoc_firing_types {
//  public smoc_scheduler_base {
public:
  typedef smoc_scheduler_top      this_type;
protected:
  typedef std::pair<transition_ty *, smoc_root_node *>  transition_node_ty;
  typedef std::list<transition_node_ty>                 transition_node_list_ty;

  typedef CoSupport::SystemC::EventOrList
    <transition_ty> smoc_transition_ready_list;
  
  smoc_transition_ready_list ol;
  
  void getLeafNodes(smoc_node_list &nodes, smoc_graph *node);
  
  void schedule(smoc_graph *c);
};

template <typename T_top>
class smoc_top_moc
  // ATTENTION: smoc_scheduler_top must be last in the
  // inheritance list because it contains a smoc_event_or_list
  // wich must be deconstructed before T_top. This requirement
  // stems from inclusion of smoc_events in T_top into the
  // smoc_event_or_list in smoc_scheduler_top.
  : public T_top,
    public smoc_scheduler_top {
private:
  // called by elaboration_done (does nothing by default)
  void end_of_elaboration() {
    this->finalise();
    if (smoc_modes::dumpProblemgraph) {
      smoc_modes::dump(std::cout, *this);
      exit(0);
    }
/*
      sc_set_stop_mode(SC_STOP_IMMEDIATE);
      sc_report_handler::suppress(SC_UNSPECIFIED  |
				  SC_DO_NOTHING   |
				  SC_THROW        |
				  SC_LOG          |
				  SC_DISPLAY      |
				  SC_CACHE_REPORT |
				  SC_INTERRUPT    |
				  SC_STOP         |
				  SC_ABORT
				  );
      // suppress intrusive SystemC std::cout messages
      std::cout.rdbuf(std::cerr.rdbuf()); 
      sc_stop();
 */
  }
  
  void scheduleTop()
    { return smoc_scheduler_top::schedule(this); }
public:
  typedef smoc_top_moc<T_top> this_type;
  
  SC_HAS_PROCESS(this_type);
  
  smoc_top_moc()
    : T_top()
    { SC_THREAD(scheduleTop); }
  explicit smoc_top_moc( sc_module_name name )
    : T_top(name)
    { SC_THREAD(scheduleTop); }
  template <typename T1>
  explicit smoc_top_moc( sc_module_name name, T1 p1 )
    : T_top(name,p1)
    { SC_THREAD(scheduleTop); }
  template <typename T1, typename T2>
  explicit smoc_top_moc( sc_module_name name, T1 p1, T2 p2 )
    : T_top(name,p1,p2)
    { SC_THREAD(scheduleTop); }
  template <typename T1, typename T2, typename T3>
  explicit smoc_top_moc( sc_module_name name, T1 p1, T2 p2, T3 p3 )
    : T_top(name,p1,p2,p3)
    { SC_THREAD(scheduleTop); }
  template <typename T1, typename T2, typename T3, typename T4>
  explicit smoc_top_moc( sc_module_name name, T1 p1, T2 p2, T3 p3, T4 p4 )
    : T_top(name,p1,p2,p3,p4)
    { SC_THREAD(scheduleTop); }
  template <typename T1, typename T2, typename T3, typename T4, typename T5>
  explicit smoc_top_moc( sc_module_name name, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5 )
    : T_top(name,p1,p2,p3,p4,p5)
    { SC_THREAD(scheduleTop); }
};

#endif // _INCLUDED_SMOC_MOC_HPP
