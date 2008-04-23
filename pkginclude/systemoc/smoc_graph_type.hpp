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

#ifndef _INCLUDED_SMOC_GRAPH_TYPE_HPP
#define _INCLUDED_SMOC_GRAPH_TYPE_HPP

#include "smoc_port.hpp"
#ifdef SYSTEMOC_ENABLE_WSDF 
# include "smoc_md_port.hpp"
# include "smoc_md_fifo.hpp"
#endif
#include "smoc_fifo.hpp"
#include "smoc_multicast_sr_signal.hpp"
#include "smoc_node_types.hpp"
#include "smoc_moc.hpp"
#ifndef __SCFE__
# include "smoc_pggen.hpp"
#endif

#include "smoc_chan_adapter.hpp"

#include <systemc.h>

#include <list>
#include <map>

class smoc_top;
class smoc_scheduler_top;

#define T_chan_init_default smoc_fifo

/**
 * use this class to connect arbitrary ports
 */
template<class ChanInit>
class smoc_port_connector {
private:
  typedef typename ChanInit::chan_type  ChanType;
public:
  
  smoc_port_connector(const ChanInit& init) :
    init(init),
    chan(new ChanType(init))
  {}

  template<class Port>
  smoc_port_connector& operator<<(Port& port)
  { chan->connect(port, init); return *this; }

private:
  // keep reference to channel initializer
  const ChanInit& init;

  // this may not implement any interface usable for ports
  // (e.g. sr-multicast: Entry/Outlet classes provide port
  // interfaces) so we call connect until there is a better
  // solution
  ChanType* chan;
};

/**
 * base class for all graph classes; no scheduling of childen (->
 * derive from this class and build FSM!)
 */
class smoc_graph_base
: public smoc_root_node {
  friend class smoc_top;
  friend class smoc_scheduler_top;
  
  typedef smoc_graph_base this_type;

protected:
 
  /**
   * Helper class for determining the data type from ports
   * (Not needed if adapter classes exist)
   */ 
  template<class P>
  struct PortTraits {
    static const bool isSpecialized = false;
    typedef void data_type;
  };

  /**
   * Specialization of PortTraits for smoc_port_in
   */
  template<class T>
  struct PortTraits< smoc_port_in<T> > { 
    static const bool isSpecialized = true;
    typedef T data_type;
  };

  /**
   * Specialization of PortTraits for smoc_port_out
   */
  template<class T>
  struct PortTraits< smoc_port_out<T> > {
    static const bool isSpecialized = true;
    typedef T data_type;
  };

#ifdef SYSTEMOC_ENABLE_WSDF 
  /**
   * Specialization of PortTraits for smoc_md_port_in
   */
  template<class T,unsigned N,template <typename, typename> class B>
  struct PortTraits< smoc_md_port_in<T,N,B> > { 
    static const bool isSpecialized = true;
    typedef T data_type;
  };

  /**
   * Specialization of PortTraits for smoc_md_port_out
   */
  template<class T,unsigned N,template <typename> class S>
  struct PortTraits< smoc_md_port_out<T,N,S> > {
    static const bool isSpecialized = true;
    typedef T data_type;
  };
#endif
  
  /// helper function for easier connector creation
  template<class ChanInit>
  smoc_port_connector<ChanInit> connector(ChanInit init)
    { return smoc_port_connector<ChanInit>(init); }

  /// connect ports using the specified channel initializer
  template<class PortA, class PortB, class ChanInit>
  void connectNodePorts(PortA &a, PortB &b, const ChanInit& i)
    { connector(i) << a << b; }
 
  /// connect ports using the default channel initializer
  template<class PortA, class PortB>
  void connectNodePorts(PortA &a, PortB &b) {
    connectNodePorts(a, b, T_chan_init_default<
      typename SysteMoC::Detail::Select<
        PortTraits<PortA>::isSpecialized,
        typename PortTraits<PortA>::data_type,
        typename PortTraits<PortB>::data_type
      >::result_type
    >());
  }
  
  /// connect ports using the specified channel initializer
  template<int s, class PortA, class PortB, class ChanInit>
  void connectNodePorts(PortA &a, PortB &b, const ChanInit& i)
    { connector(i) << a << b; }
  
  /// connect ports using the default channel initializer
  template<int s, class PortA, class PortB>
  void connectNodePorts(PortA &a, PortB &b) {
    connectNodePorts(a, b, T_chan_init_default<
      typename SysteMoC::Detail::Select<
        PortTraits<PortA>::isSpecialized,
        typename PortTraits<PortA>::data_type,
        typename PortTraits<PortB>::data_type
      >::result_type
    >(s));
  }

#ifdef SYSTEMOC_ENABLE_WSDF 

  /// FIXME: this should probably be generalized as well
  template<class EdgeInit, class PortA, class PortB>
  void indConnectNodePorts(PortA& a, PortB& b, const EdgeInit& e) {
    typedef typename EdgeInit::chan_init_type ChanInit;
    connectNodePorts(a, b, ChanInit(e, a.params(), b.params()));
  }

#endif

public:

  template<typename T>
  T& registerNode(T* node)
    { return *node; }

  const smoc_node_list& getNodes() const;
  const smoc_chan_list& getChans() const;
  void getChansRecursive( smoc_chan_list & channels) const;

protected:
  smoc_graph_base(sc_module_name name, smoc_firing_state& init, bool regObj);

  void finalise();

  // top graph scheduler
  // FIXME: make private when all graph types have FSM
  smoc_scheduler_top *top;
  
  // calls top graph scheduler (if available) 
  // FIXME: make private and non virtual  when all graph types
  // have FSM
  virtual void smocCallTop();

private:
  // process for top moc
  SC_HAS_PROCESS(this_type);
  
  // actor and graph child objects
  smoc_node_list nodes;

  // channel child objects
  smoc_chan_list channels;

#ifndef __SCFE__
  void pgAssemble(smoc_modes::PGWriter &pgw, const smoc_root_node *n) const;
  void assembleActor(smoc_modes::PGWriter &pgw) const;
#endif
};
  
#undef T_chan_init_default

/**
 * graph with FSM which schedules children by selecting
 * any executable transition
 */
class smoc_graph : public smoc_graph_base {
public:
  // construct graph with name
  explicit smoc_graph(sc_module_name name);

  // construct graph with generated name
  smoc_graph();

private:
  // graph scheduler FSM states
  smoc_firing_state init, run;
  
  // common constructor code
  void constructor();
  
  // initialize children scheduling
  void initScheduling();

  // schedule children of this graph
  void schedule();

  // a list containing the transitions of the graph's children
  // that may be executed
  smoc_transition_ready_list ol;
};


/**
 * FIXME: derive from smoc_graph_base -> write FSM -> delete smocCallTop
 */
class smoc_graph_sr : public smoc_graph {
public:
  // construct graph with name
  explicit smoc_graph_sr(sc_module_name name);
  
  // construct graph with generated name
  smoc_graph_sr();

protected:
  // call non-default schedule method of top scheduler
  // FIXME: remove when scheduling is done by FSM
  void smocCallTop();
};

#endif // _INCLUDED_SMOC_GRAPH_TYPE_HPP
