// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#include "NodeBase.hpp"
// FIXME: Migrate these includes to smoc
#include "../../systemoc/smoc_port.hpp"
#include "../../systemoc/smoc_fifo.hpp"
//#include "../../systemoc/smoc_chan_adapter.hpp"

#ifdef SYSTEMOC_ENABLE_MAESTRO
#include <Maestro/MetaMap/CommunicationComponent.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO

#include <boost/mpl/if.hpp>
#include <boost/mpl/bool.hpp>

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
#ifdef SYSTEMOC_ENABLE_MAESTRO
    MM::MMAPI *api = MM::MMAPI::getInstance();
    
    //get name of the actor of port a
    std::string srcActorName = string(a.get_parent()->name());
//  //get name of the actor of port b
//  std::string dstActorName = string(b.get_parent()->name());
    
    //get name of port a
    std::string srcPortName = string(a.name());
//  //get name of port b
//  std::string dstPortName = string(b.name());
    
    bool isActorCommRouted = api->isActorCommunicationRouted(srcActorName, srcPortName);
    
    if (isActorCommRouted) {
      assert(!"FIXME: Implement routed communication supporting channel initializers!");
    } else
#endif // SYSTEMOC_ENABLE_MAESTRO
    {
      chanInit.connect(a).connect(b);
    }
  }

  /// connect ports using the default channel initializer
  template<int s, class PortA, class PortB>
  void connectNodePorts(PortA &a, PortB &b) {
    typedef typename boost::mpl::if_<PortTraits<PortA>,
        typename PortTraits<PortA>::data_type,
        typename PortTraits<PortB>::data_type
      >::type data_type;
    
#ifdef SYSTEMOC_ENABLE_MAESTRO
    MM::MMAPI *api = MM::MMAPI::getInstance();
    
    //get name of the actor of port a
    std::string srcActorName = string(a.get_parent()->name());
//  //get name of the actor of port b
//  std::string dstActorName = string(b.get_parent()->name());
    
    //get name of port a
    std::string srcPortName = string(a.name());
//  //get name of port b
//  std::string dstPortName = string(b.name());
    
    bool isActorCommRouted = api->isActorCommunicationRouted(srcActorName, srcPortName);
    
    if (isActorCommRouted) {
      assert(!"FIXME: Implement routed communication supporting channel initializers!");
    } else
#endif // SYSTEMOC_ENABLE_MAESTRO
    {
      smoc_fifo<data_type> chanInit(s);
      chanInit.connect(a).connect(b);
    }
  }

  /// connect ports using the default channel initializer
  template<class PortA, class PortB>
  void connectNodePorts(PortA &a, PortB &b) {
    typedef typename boost::mpl::if_<PortTraits<PortA>,
        typename PortTraits<PortA>::data_type,
        typename PortTraits<PortB>::data_type
      >::type data_type;
    
#ifdef SYSTEMOC_ENABLE_MAESTRO
    MM::MMAPI *api = MM::MMAPI::getInstance();
    
    //get name of the actor of port a
    std::string srcActorName = string(a.get_parent()->name());
//  //get name of the actor of port b
//  std::string dstActorName = string(b.get_parent()->name());
    
    //get name of port a
    std::string srcPortName = string(a.name());
//  //get name of port b
//  std::string dstPortName = string(b.name());
    
    bool isActorCommRouted = api->isActorCommunicationRouted(srcActorName, srcPortName);
    
    if (isActorCommRouted) {
      connectRoutedPortsF(a, b);
    } else
#endif // SYSTEMOC_ENABLE_MAESTRO
    {
      smoc_fifo<data_type> chanInit;
      chanInit.connect(a).connect(b);
    }
  }

protected:

#ifdef SYSTEMOC_ENABLE_MAESTRO
//template<class PortA, class PortB>
//void connectRoutedPortsI(PortA &a, PortB &b)
//  { connectRoutedPortsF(b, a); }

  /**
   * Method to map the routing of communication channels into the architecture communication components
   * input file: routings.xml
   */
  template<class PortA, class PortB>
  void connectRoutedPortsF(PortA &a, PortB &b) {
    typedef typename boost::mpl::if_<PortTraits<PortA>,
        typename PortTraits<PortA>::data_type,
        typename PortTraits<PortB>::data_type
      >::type data_type;
    
    MM::MMAPI* api = MM::MMAPI::getInstance();
    
    //get name of the actor of port a
    string srcActorName = string(a.get_parent()->name());
    //get name of the actor of port b
    string dstActorName = string(b.get_parent()->name());
    
    //get name of port a
    string srcPortName = string(a.name());
    //get name of port b
    string dstPortName = string(b.name());
    
    vector<tuple<string, string, string> > hops = api->getHopsOfMappedCommunication(srcActorName, srcPortName);
    
    for (int i = 0; i <= hops.size(); i++) {
      string currentHop;
      string nextHop;
      
      if (i > 0) {
        currentHop = get<0>(hops[i - 1]);
      } else {
        currentHop = api->getComponentForActor(srcActorName);
      }
      
      if (i < hops.size()) {
        nextHop = get<0>(hops[i]);
      } else {
        nextHop = api->getComponentForActor(dstActorName);
      }
      
      smoc_fifo<data_type> chanInit;
      
      if (i == 0) {
        // src is computation actor
        chanInit.connect(a);
      } else {
        // src is communication actor
        //get port to connect from
        string communicationPseudportToConnectFrom = api->getCommunicationComponentOutputPort(srcActorName, srcPortName, currentHop);
        //get instance of actor to connect from
        smoc_actor* sourceCommunicationActorInstance = dynamic_cast<smoc_actor*>(api->getActor(currentHop));

        //get instance of ports to connect from
        CommunicationComponent* sourceCommunicationActor = dynamic_cast<CommunicationComponent*>(sourceCommunicationActorInstance);
        smoc_port_out<data_type> *outputPort = dynamic_cast<smoc_port_out<data_type> *>(sourceCommunicationActor->getPortByName(communicationPseudportToConnectFrom));
        chanInit.connect(*outputPort);
      }
      
      if (i == hops.size()) {
        // dst is computation actor
        chanInit.connect(b);
      } else {
        //dst is communication actor
        //get port to connect to
        string communicationPseudportToConnectTo = api->getCommunicationComponentInputPort(srcActorName, srcPortName, nextHop);

        //get instance of actor to connect to
        smoc_actor* destinationCommunicationActorInstance = dynamic_cast<smoc_actor*>(api->getActor(nextHop));

        //get instance of ports to connect to
        CommunicationComponent* destinationCommunicationActor = dynamic_cast<CommunicationComponent*>(destinationCommunicationActorInstance);
        smoc_port_in<data_type>  *inputPort = dynamic_cast<smoc_port_in<data_type> *>(destinationCommunicationActor->getPortByName(communicationPseudportToConnectTo));
        chanInit.connect(*inputPort);
      }
    }
  }
#endif // SYSTEMOC_ENABLE_MAESTRO

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
