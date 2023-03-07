// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2013 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Matthias Schid <matthias.schid@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SMOC_DETAIL_GRAPHBASE_HPP
#define _INCLUDED_SMOC_DETAIL_GRAPHBASE_HPP

#include "../smoc_scheduler_top.hpp"

#include "NodeBase.hpp"
#include "../smoc_port_in.hpp"
#include "../smoc_port_out.hpp"
// FIXME: Migrate this includes to smoc
#include "../../systemoc/smoc_fifo.hpp"

#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>

#include <systemc>

namespace smoc { namespace Detail {

/**
 * base class for all graph classes; no scheduling of children (->
 * derive from this class and build FSM!). If you derive more stuff
 * from this class you have to change apply_visitor.hpp accordingly.
 */
class GraphBase: public NodeBase
{
  // need to call *StateChange
  friend class smoc_multireader_fifo_chan_base;
  friend class NodeBase;
  friend class smoc::smoc_scheduler_top; // doReset

  typedef GraphBase this_type;
private:
  /**
   * Helper class for determining the data type from ports
   * (Not needed if adapter classes exist)
   */ 
  template<class P>
  struct PortTraits: public boost::mpl::bool_<false>
    { typedef void data_type; };

  /**
   * Specialization of PortTraits for smoc_port_in
   */
  template<class T>
  struct PortTraits<smoc_port_in<T> >: public boost::mpl::bool_<true>
    { typedef T data_type; };

  /**
   * Specialization of PortTraits for smoc_port_out
   */
  template<class T>
  struct PortTraits<smoc_port_out<T> >: public boost::mpl::bool_<true>
    { typedef T data_type; };

public:  
///// connect ports using the specified channel initializer
//template<class Init>
//Init connector(const Init &i)
//  { return i; }

  // FIXME: We should really store these in a list and delete them one GraphBase destruction.
  template<typename T>
  T &registerNode(T* node)
    { return *node; }

  /// connect ports using the specified channel initializer
  template<class PortA, class PortB, class ChanInit>
  void connectNodePorts(PortA &a, PortB &b, ChanInit chanInit) {
    chanInit.connect(a).connect(b);
  }

  /// connect ports using the default channel initializer
  template<int s, class PortA, class PortB>
  void connectNodePorts(PortA &a, PortB &b) {
    typedef typename boost::mpl::if_<PortTraits<PortA>,
        typename PortTraits<PortA>::data_type,
        typename PortTraits<PortB>::data_type
      >::type data_type;
    smoc_fifo<data_type> chanInit(s);
    chanInit.connect(a).connect(b);
  }

  /// connect ports using the default channel initializer
  template<class PortA, class PortB>
  void connectNodePorts(PortA &a, PortB &b) {
    typedef typename boost::mpl::if_<PortTraits<PortA>,
        typename PortTraits<PortA>::data_type,
        typename PortTraits<PortB>::data_type
      >::type data_type;
    smoc_fifo<data_type> chanInit;
    chanInit.connect(a).connect(b);
  }

protected:
  GraphBase(const sc_core::sc_module_name &name, smoc_state *init);

  virtual void before_end_of_elaboration();
  virtual void end_of_elaboration();
  
  const NodeList       &getNodes() const;
  const smoc_chan_list &getChans() const;
  sc_core::sc_object   *getChild(std::string const &name) const;

  /// @brief Resets given node
  void doReset();

private:
  /// Actor and graph child objects of this graph.
  NodeList            nodes;
  /// Channel child objects of this graph.
  smoc_chan_list      channels;
  /// Child lookup map
  std::map<std::string, sc_core::sc_object *> childLookupMap;
  /// Scheduler for this graph. If this variable is NULL, than this graph will
  /// be scheduled by its parent graph.
  smoc_scheduler_top *scheduler;

  void setScheduler(smoc_scheduler_top *scheduler);
};
  
} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_GRAPHBASE_HPP */
