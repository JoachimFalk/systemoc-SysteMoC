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

#ifndef _INCLUDED_SMOC_DETAIL_GRAPHBASE_HPP
#define _INCLUDED_SMOC_DETAIL_GRAPHBASE_HPP

#include <systemc>

//#include "SimulationContext.hpp"

#include "../smoc_scheduler_top.hpp"

// FIXME: Migrate these incldues to smoc
#include "../../systemoc/smoc_port.hpp"
#include "../../systemoc/smoc_fifo.hpp"
#include "../../systemoc/smoc_multicast_sr_signal.hpp"
#include "../../systemoc/smoc_actor.hpp"
#include "../../systemoc/smoc_chan_adapter.hpp"

namespace smoc { namespace Detail {

/**
 * base class for all graph classes; no scheduling of childen (->
 * derive from this class and build FSM!). If you derive more stuff
 * from this class you have to change apply_visitor.hpp accordingly.
 */
class GraphBase: public smoc_root_node {
  // need to call *StateChange
  friend class smoc_multireader_fifo_chan_base;
  friend class smoc_reset_chan;
  friend class smoc_root_node;
  friend class smoc::smoc_scheduler_top; // doReset

  typedef GraphBase this_type;
public:  
//protected:
 
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

  /// connect ports using the specified channel initializer
  template<class Init>
  Init connector(const Init &i)
    { return i; }

  /// connect ports using the specified channel initializer
  template<class PortA, class PortB, class ChanInit>
  void connectNodePorts(PortA &a, PortB &b, ChanInit i)
    { i.connect(a).connect(b); }

  /// connect ports using the default channel initializer
  template<class PortA, class PortB>
  void connectNodePorts(PortA &a, PortB &b) {
    connectNodePorts(a, b, smoc_fifo<
      typename smoc::Detail::Select<
        PortTraits<PortA>::isSpecialized,
        typename PortTraits<PortA>::data_type,
        typename PortTraits<PortB>::data_type
      >::result_type
    >());
  }

  /// connect ports using the default channel initializer
  template<int s, class PortA, class PortB>
  void connectNodePorts(PortA &a, PortB &b) {
    connectNodePorts(a, b, smoc_fifo<
      typename smoc::Detail::Select<
        PortTraits<PortA>::isSpecialized,
        typename PortTraits<PortA>::data_type,
        typename PortTraits<PortB>::data_type
      >::result_type
    >(s));
  }
public:

  template<typename T>
  T& registerNode(T* node)
    { return *node; }

  const smoc_node_list &getNodes() const;
  const smoc_chan_list &getChans() const;
//void getNodesRecursive(smoc_node_list &nodes) const;
//void getChansRecursive(smoc_chan_list &chans) const;

protected:
  GraphBase(const sc_core::sc_module_name &name, smoc_firing_state &init);

  virtual void before_end_of_elaboration();
  virtual void end_of_elaboration();
  
  /// @brief Resets given node
  void doReset();

private:
  /// Actor and graph child objects of this graph.
  smoc_node_list      nodes;
  /// Channel child objects of this graph.
  smoc_chan_list      channels;
  /// Scheduler for this graph. If this variable is NULL, than this graph will
  /// be scheduled by its parent graph.
  smoc_scheduler_top *scheduler;

  void setScheduler(smoc_scheduler_top *scheduler);
};
  
} } // namespace smoc::Detail

#endif // _INCLUDED_SMOC_DETAIL_GRAPHBASE_HPP
