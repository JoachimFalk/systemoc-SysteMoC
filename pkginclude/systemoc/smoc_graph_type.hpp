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
 * use this class to connect arbitrary ports (-> provide adapters)
 */
template<class ChanInit>
class smoc_port_connector {
private:
  typedef typename ChanInit::chan_type  ChanType;
  typedef typename ChanType::iface_type IFaceImpl;
public:

  smoc_port_connector(const ChanInit& init) :
    chan(new ChanType(init))
    {}

  template<class IFace>
  smoc_port_connector& operator<<(sc_port<IFace>& port) {
    bind<IFace>(port);
    return *this;
  }

  template<class T>
  smoc_port_connector& operator<<(smoc_port_in<T>& port) {
    typedef typename smoc_port_in<T>::iface_type IFace;
    bind<IFace>(port);
    return *this;
  }

  template<class T>
  smoc_port_connector& operator<<(smoc_port_out<T>& port) {
    typedef typename smoc_port_out<T>::iface_type IFace;
    bind<IFace>(port);
    return *this;
  }

private:
  ChanType* chan;

  // select type A or B based on predicate P
  template<bool P, class A, class B>
  struct Select;

  // specialization: select type A
  template<class A,class B>
  struct Select<true,A,B>
    { typedef A result_type; };

  // specialization: select type B
  template<class A,class B>
  struct Select<false,A,B>
    { typedef B result_type; };

  // construct new instance
  template<class T, class R = T>
  struct Alloc {
    static R& apply(T& t)
      { return *(new R(t)); }
  };

  // copy instance
  template<class T, class R = T>
  struct Copy {
    static R& apply(T& t)
      { return t; }
  };

  // this does all the work...
  template<class IFace, class Port>
  void bind(Port& port) {
    typedef smoc_chan_adapter<IFace,IFaceImpl> Adapter;

    // test if adapter available: if a specialization exists,
    // we will use the adapter, otherwise, we plug the channel
    // into the port
    typedef typename Select<
              Adapter::isAdapter,
              Alloc<IFaceImpl,Adapter>,
              Copy<IFace> >::result_type Op;

    port(Op::apply(*chan));
  }
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

  /// helper function for easier connector creation
  template<class ChanInit>
  smoc_port_connector<ChanInit> connector(ChanInit init)
    { return smoc_port_connector<ChanInit>(init); }

  /// connect ports using the specified channel initializer
  template<class ChanInit, class PortA, class PortB>
  void connectNodePorts(PortA &a, PortB &b, const ChanInit& i)
    { connector(i) << a << b; }

  /// The functions below are convenience functions which
  /// deduce the data type automatically from smoc_ports
  /// FIXME: automate this procedure and remove functions

  /// connect ports using the default channel initializer
  /// (specify both size and data type)
  template<int i, class T>
  void connectNodePorts(smoc_port_out<T>& a, smoc_port_in<T>& b)
    { connectNodePorts(a, b, T_chan_init_default<T>(i)); }

  template<int i, class T, class PortB>
  void connectNodePorts(smoc_port_out<T>& a, PortB& b)
    { connectNodePorts(a, b, T_chan_init_default<T>(i)); }

  template<int i, class T, class PortA>
  void connectNodePorts(PortA& a, smoc_port_in<T>& b)
    { connectNodePorts(a, b, T_chan_init_default<T>(i)); }
  
  template<int i, class T, class PortA, class PortB>
  void connectNodePorts(PortA& a, PortB& b)
    { connectNodePorts(a, b, T_chan_init_default<T>(i)); }

  /// connect ports using the default channel initializer
  /// (specify only data type)
  template<class T>
  void connectNodePorts(smoc_port_out<T>& a, smoc_port_in<T>& b)
    { connectNodePorts(a, b, T_chan_init_default<T>()); }

  template<class T, class PortB>
  void connectNodePorts(smoc_port_out<T>& a, PortB& b)
    { connectNodePorts(a, b, T_chan_init_default<T>()); }

  template<class T, class PortA>
  void connectNodePorts(PortA& a, smoc_port_in<T>& b)
    { connectNodePorts(a, b, T_chan_init_default<T>()); }

  template<class T, class PortA, class PortB>
  void connectNodePorts(PortA& a, PortB& b)
    { connectNodePorts(a, b, T_chan_init_default<T>()); }

  // FIXME: adopt functions below to the connector class

#ifdef SYSTEMOC_ENABLE_WSDF 
  /// Connect multi-dimensional actor output port with multi-dimensional actor input port
  template <typename T_chan_init, 
            unsigned N,
            template <typename,typename> class R,
            template <typename> class STORAGE_OUT_TYPE
            >
  void connectNodePorts(
                        smoc_md_port_out<typename T_chan_init::data_type, N, STORAGE_OUT_TYPE> &b,
                        smoc_md_port_in<typename T_chan_init::data_type, N, R>  &a,
                        const T_chan_init i ) {
    typename T_chan_init::chan_type &chan =
      registerChan<T_chan_init>(i);
    connectChanPort(chan,a);
    connectChanPort(chan,b);
    b.setFiringLevelMap(i.wsdf_edge_param.calc_src_iteration_level_table());
    a.setFiringLevelMap(i.wsdf_edge_param.calc_snk_iteration_level_table());
  }
  /// Connect multi-dimensional actor output port with multi-dimensional interface input port
  template <typename T_chan_init, 
            unsigned N,
            template <typename> class STORAGE_OUT_TYPE>
  void connectNodePorts(
                        smoc_md_port_out<typename T_chan_init::data_type, N, STORAGE_OUT_TYPE> &b,
                        smoc_md_iport_in<typename T_chan_init::data_type, N> &a,
                        const T_chan_init i ) {
    typename T_chan_init::chan_type &chan =
      registerChan<T_chan_init>(i);
    connectChanPort(chan,a);
    connectChanPort(chan,b);
    b.setFiringLevelMap(i.wsdf_edge_param.calc_src_iteration_level_table());
    a.setFiringLevelMap(i.wsdf_edge_param.calc_snk_iteration_level_table());
  }
  /// Connect multi-dimensional interface output port with multi-dimensional actor input port
  template <typename T_chan_init, 
            unsigned N,
            template <typename, typename> class R,
            template <typename> class STORAGE_OUT_TYPE>
  void connectNodePorts(
                        smoc_md_iport_out<typename T_chan_init::data_type, N, STORAGE_OUT_TYPE> &b,
                        smoc_md_port_in<typename T_chan_init::data_type, N, R>  &a,
                        const T_chan_init i ) {
    typename T_chan_init::chan_type &chan =
      registerChan<T_chan_init>(i);
    connectChanPort(chan,a);
    connectChanPort(chan,b);
    b.setFiringLevelMap(i.wsdf_edge_param.calc_src_iteration_level_table());
    a.setFiringLevelMap(i.wsdf_edge_param.calc_snk_iteration_level_table());
  }

  
  /* Indirect channel setup */
  template <unsigned N,
            template <typename,typename> class R,
            typename T_edge_init,
            template <typename> class STORAGE_OUT_TYPE>
  void indConnectNodePorts(smoc_md_port_out<typename T_edge_init::data_type, N, STORAGE_OUT_TYPE> &b,
                           smoc_md_port_in<typename T_edge_init::data_type, N, R>  &a,
                           const T_edge_init i ) {
    typedef typename T_edge_init::chan_init_type T_chan_init;
    T_chan_init chan_init(i,b.params(),a.params());
    typename T_chan_init::chan_type &chan =
      registerChan<T_chan_init>(chan_init);
    connectChanPort(chan,a);
    connectChanPort(chan,b);
    b.setFiringLevelMap(chan_init.wsdf_edge_param.calc_src_iteration_level_table());
    a.setFiringLevelMap(chan_init.wsdf_edge_param.calc_snk_iteration_level_table());
  }
  template <unsigned N,
            typename T_edge_init,
            template <typename> class STORAGE_OUT_TYPE>
  void indConnectNodePorts(smoc_md_port_out<typename T_edge_init::data_type, N, STORAGE_OUT_TYPE> &b,
                           smoc_md_iport_in<typename T_edge_init::data_type, N>  &a,
                           const T_edge_init i ) {
    typedef typename T_edge_init::chan_init_type T_chan_init;
    T_chan_init chan_init(i,b.params(),a.params());
    typename T_chan_init::chan_type &chan =
      registerChan<T_chan_init>(chan_init);
    connectChanPort(chan,a);
    connectChanPort(chan,b);
    b.setFiringLevelMap(chan_init.wsdf_edge_param.calc_src_iteration_level_table());
    a.setFiringLevelMap(chan_init.wsdf_edge_param.calc_snk_iteration_level_table());
  }
  template <unsigned N,
            template <typename,typename> class R,
            typename T_edge_init,
            template <typename> class STORAGE_OUT_TYPE>
  void indConnectNodePorts(smoc_md_iport_out<typename T_edge_init::data_type, N, STORAGE_OUT_TYPE> &b,
                           smoc_md_port_in<typename T_edge_init::data_type, N, R>  &a,
                           const T_edge_init i ) {
    typedef typename T_edge_init::chan_init_type T_chan_init;
    T_chan_init chan_init(i,b.params(),a.params());
    typename T_chan_init::chan_type &chan =
      registerChan<T_chan_init>(chan_init);
    connectChanPort(chan,a);
    connectChanPort(chan,b);
    b.setFiringLevelMap(chan_init.wsdf_edge_param.calc_src_iteration_level_table());
    a.setFiringLevelMap(chan_init.wsdf_edge_param.calc_snk_iteration_level_table());
  }
#endif
public:

  template <typename T>
  T &registerNode( T *node ) {
    return *node;
  }
  
  template <typename T_chan_init>
  typename T_chan_init::chan_type &registerChan( const T_chan_init i ) {
    typename T_chan_init::chan_type *chan =
      new typename T_chan_init::chan_type(i);
    return *chan;
  }
  //template <typename T_chan_type, template <typename, typename> class R, class P, template <typename> class STORAGE_OUT_TYPE>
  template <typename T_chan_type, template <typename> class R, template <typename> class STORAGE_OUT_TYPE>
  void connectChanPort( T_chan_type &chan,
                        smoc_port_out_base<typename T_chan_type::data_type, R, STORAGE_OUT_TYPE> &p ) {
    p(chan);
  }
  //template <typename T_chan_type, template <typename, typename> class R, class P>
  template <typename T_chan_type, template <typename> class R>
  void connectChanPort( T_chan_type &chan,
                        smoc_port_in_base<typename T_chan_type::data_type, R> &p ) {
    p(chan);
  }
  template <typename T,
      //template <typename, typename> class R,
      template <typename> class R>
  void connectChanPort(
    smoc_multicast_sr_signal_type<T> &chan,
    smoc_port_in_base<typename smoc_multicast_sr_signal_type<T>::data_type,R> &p )
  {
//    assert( port2chan.find(&p) ==  port2chan.end() );
//    port2chan[&p] = &chan;
//    chan2ports[&chan].push_back(&p);
    p(chan.getOutlet(p));
  }
  template <typename T,
      //template <typename, typename> class R,
      template <typename> class R,
      template <typename> class STORAGE_OUT_TYPE>
  void connectChanPort(
    smoc_multicast_sr_signal_type<T> &chan,
    smoc_port_out_base<
      typename smoc_multicast_sr_signal<T>::data_type,
      R,
      STORAGE_OUT_TYPE> &p )
  {
//    assert( port2chan.find(&p) ==  port2chan.end() );
//    port2chan[&p] = &chan;
//    chan2ports[&chan].push_back(&p);
    p(chan.getEntry());
  }

  const smoc_node_list& getNodes() const;
  const smoc_chan_list& getChans() const;
  

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
  
  // called by elaboration_done (does nothing by default)
  void end_of_elaboration();
 
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
