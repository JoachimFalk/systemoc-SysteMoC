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

#ifdef SYSTEMOC_ENABLE_MAESTRO
#include <Maestro/MetaMap/SMoCGraph.hpp>
#include <Maestro/MetaMap/CommunicationComponent.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO

namespace smoc { namespace Detail {

/**
 * base class for all graph classes; no scheduling of childen (->
 * derive from this class and build FSM!). If you derive more stuff
 * from this class you have to change apply_visitor.hpp accordingly.
 */
class GraphBase: public smoc_root_node 
#ifdef SYSTEMOC_ENABLE_MAESTRO
	, public MetaMap::SMoCGraph
#endif //SYSTEMOC_ENABLE_MAESTRO
{
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

#ifdef SYSTEMOC_ENABLE_MAESTRO

  /// connect ports using the default channel initializer
  template<class PortA, class PortB>
  void connectNodePorts(PortA &a, PortB &b) {

	  MM::MMAPI* api = MM::MMAPI::getInstance();

	  //get name of the actor of port a
	  string srcActorName = string(a.get_parent()->name());
	  //get name of the actor of port b
	  string dstActorName = string(b.get_parent()->name());

	  //get name of port a
	  string srcPortName = string(a.name());
	  //get name of port b
	  string dstPortName = string(b.name());

	  bool isActorCommRouted = api->isActorCommunicationRouted(srcActorName, srcPortName);

	  if (isActorCommRouted)
	  {
		connectRoutedPortsF(a, b);
	  }
	  else
	  {
		  connectNodePorts(a, b, smoc_fifo<
			  typename smoc::Detail::Select<
			  PortTraits<PortA>::isSpecialized,
			  typename PortTraits<PortA>::data_type,
			  typename PortTraits<PortB>::data_type
			  >::result_type
		  >());
	  }

  }

  /// connect ports using the default channel initializer
  template<class PortA, class PortB>
  void connectNodePortsB(PortA &a, PortB &b) {
	  connectNodePorts(a, b, smoc_fifo<
		  typename smoc::Detail::Select<
		  PortTraits<PortA>::isSpecialized,
		  typename PortTraits<PortA>::data_type,
		  typename PortTraits<PortB>::data_type
		  >::result_type
	  >());
  }
#else
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
#endif
  
  

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

#ifdef SYSTEMOC_ENABLE_MAESTRO

  template<class PortA, class PortB>
  void connectRoutedPortsI(PortA &a, PortB &b)
  {
	  connectRoutedPortsF(b, a);
  }

/**
* Method to map the routing of communication channels into the architecture communication components
* input file: routings.xml
*/
  template<class PortA, class PortB>
  void connectRoutedPortsF(PortA &a, PortB &b)
  {

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

	  for (int i = 0; i <= hops.size(); i++)
	  {
		  string currentHop;
		  string nextHop;

		  if (i > 0)
		  {
			  currentHop = get<0>(hops[i - 1]);
		  }
		  else
		  {
			  currentHop = api->getComponentForActor(srcActorName);
		  }

		  if (i < hops.size())
		  {
			  nextHop = get<0>(hops[i]);
		  }
		  else
		  {
			  nextHop = api->getComponentForActor(dstActorName);
		  }

		  smoc_port_out<PortA::data_type>* outputPort;
		  smoc_port_in<PortB::data_type>* inputPort;

		  //src is computation actor
		  if (i == 0)
		  {
			  outputPort = &a;
		  }
		  else //src is communication actor
		  {
			  //get port to connect from
			  string communicationPseudportToConnectFrom = api->getCommunicationComponentOutputPort(srcActorName, srcPortName, currentHop); //**

																																			//get instance of actor to connect from
			  smoc_actor* sourceCommunicationActorInstance = dynamic_cast<smoc_actor*>(api->getActor(currentHop));

			  //get instance of ports to connect from
			  CommunicationComponent* sourceCommunicationActor = dynamic_cast<CommunicationComponent*>(sourceCommunicationActorInstance);
			  outputPort = dynamic_cast<smoc_port_out<PortA::data_type>*>(sourceCommunicationActor->getPortByName(communicationPseudportToConnectFrom));


		  }

		  // dst is computation actor
		  if (i == hops.size())
		  {
			  inputPort = &b;
		  }
		  else //dst is communication actor
		  {
			  //get port to connect to
			  string communicationPseudportToConnectTo = api->getCommunicationComponentInputPort(srcActorName, srcPortName, nextHop); //**

																																	  //get instance of actor to connect to
			  smoc_actor* destinationCommunicationActorInstance = dynamic_cast<smoc_actor*>(api->getActor(nextHop));

			  //get instance of ports to connect to
			  CommunicationComponent* destinationCommunicationActor = dynamic_cast<CommunicationComponent*>(destinationCommunicationActorInstance);
			  inputPort = dynamic_cast<smoc_port_in<PortB::data_type>*>(destinationCommunicationActor->getPortByName(communicationPseudportToConnectTo));

		  }


		  connectNodePortsB(*outputPort, *inputPort);

	  }


  }
#endif

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
