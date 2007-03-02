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
#include "smoc_rendezvous.hpp"
#include "smoc_node_types.hpp"
#ifndef __SCFE__
# include "smoc_pggen.hpp"
#endif

#include <systemc.h>

#include <list>
#include <map>

#define T_chan_init_default smoc_fifo
class smoc_graph
: public sc_module,
  public smoc_root_node {
public:
//typedef T_node_type  node_type;
//typedef T_chan_kind  chan_kind;
  typedef smoc_graph    this_type;

  friend class smoc_top;
protected:
  template <typename T_chan_init, 
            template <typename, typename> class R,
            class P>
  void connectNodePorts(
                        smoc_port_out_base<typename T_chan_init::data_type, R, P> &b,
                        smoc_port_in_base<typename T_chan_init::data_type, R, P>  &a,
                        const T_chan_init i ) {
    typename T_chan_init::chan_type &chan =
      registerChan<T_chan_init>(i);
    connectChanPort(chan,a);
    connectChanPort(chan,b);
  }  
  template <int i, typename T_data_type, 
            template <typename, typename> class R,
            class P>
  void connectNodePorts(
                        smoc_port_out_base<T_data_type,R,P> &b,
                        smoc_port_in_base<T_data_type,R,P>  &a ) {
    typename T_chan_init_default<T_data_type>::chan_type &chan =
      registerChan( T_chan_init_default<T_data_type>(i) );
    connectChanPort(chan,a);
    connectChanPort(chan,b);
  }
  template <typename T_data_type, 
            template <typename, typename> class R,
            class P>
  void connectNodePorts(
                        smoc_port_out_base<T_data_type,R,P> &b,
                        smoc_port_in_base<T_data_type,R,P>  &a ) {
    typename T_chan_init_default<T_data_type>::chan_type &chan =
      registerChan( T_chan_init_default<T_data_type>() );
    connectChanPort(chan,a);
    connectChanPort(chan,b);
  }

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










  template <typename T_value_type>
  void connectInterfacePorts(
                             smoc_port_out<T_value_type> &a,
                             smoc_port_out<T_value_type> &b )
    { b(a); }
  template <typename T_value_type>
  void connectInterfacePorts(
                             smoc_port_in<T_value_type> &a,
                             smoc_port_in<T_value_type> &b )
  { b(a); }

#ifdef SYSTEMOC_ENABLE_WSDF 
  /// Connect multi-dimensional interface output port with actor output port
  template <typename T_value_type, unsigned N, template <typename> class STORAGE_OUT_TYPE>
  void connectInterfacePorts(
                             smoc_md_iport_out<T_value_type,N,STORAGE_OUT_TYPE> &a,
                             smoc_md_port_out<T_value_type,N,STORAGE_OUT_TYPE> &b )
    { b(a); }

  /// Connect multi-dimensional interface input port with actor input port
  template <typename T_value_type, unsigned N, template <typename,typename> class R>
  void connectInterfacePorts(
                             smoc_md_iport_in<T_value_type,N> &a,
                             smoc_md_port_in<T_value_type,N,R> &b )
  { b(a); }

  /// Connect multi-dimensional interface output ports
  template <typename T_value_type, unsigned N, template <typename> class STORAGE_OUT_TYPE>
  void connectInterfacePorts(
                             smoc_md_iport_out<T_value_type,N, STORAGE_OUT_TYPE> &a,
                             smoc_md_iport_out<T_value_type,N, STORAGE_OUT_TYPE> &b )
    { b(a); }

  /// Connect multi-dimensional interface output ports
  template <typename T_value_type, unsigned N>
  void connectInterfacePorts(
                             smoc_md_iport_in<T_value_type,N> &a,
                             smoc_md_iport_in<T_value_type,N> &b )
  { b(a); }
#endif

  void finalise();

  smoc_firing_state dummy;
public:
  explicit smoc_graph(sc_module_name name)
    : sc_module(name),
      smoc_root_node(dummy) {
    dummy = CALL(smoc_graph::finalise) >> dummy;
  }
  smoc_graph()
    : sc_module(sc_gen_unique_name("smoc_graph")),
      smoc_root_node(dummy) {
    dummy = CALL(smoc_graph::finalise) >> dummy;
  }

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
  template <typename T_chan_type, template <typename, typename> class R, class P, template <typename> class STORAGE_OUT_TYPE>
  void connectChanPort( T_chan_type &chan,
                        smoc_port_out_base<typename T_chan_type::data_type, R, P, STORAGE_OUT_TYPE> &p ) {
    p(chan);
  }
  template <typename T_chan_type, template <typename, typename> class R, class P>
  void connectChanPort( T_chan_type &chan,
                        smoc_port_in_base<typename T_chan_type::data_type, R, P> &p ) {
    p(chan);
  }
  template <typename T,
      template <typename, typename> class R,
      class P>
  void connectChanPort(
    smoc_multicast_sr_signal_type<T> &chan,
    smoc_port_in_base<
      typename smoc_multicast_sr_signal_type<T>::data_type,
      R,
      P> &p )
  {
//    assert( port2chan.find(&p) ==  port2chan.end() );
//    port2chan[&p] = &chan;
//    chan2ports[&chan].push_back(&p);
    p(chan.getOutlet(p));
  }
  template <typename T,
      template <typename, typename> class R,
      class P,
      template <typename> class STORAGE_OUT_TYPE>
  void connectChanPort(
    smoc_multicast_sr_signal_type<T> &chan,
    smoc_port_out_base<
      typename smoc_multicast_sr_signal<T>::data_type,
      R,
      P,
      STORAGE_OUT_TYPE> &p )
  {
//    assert( port2chan.find(&p) ==  port2chan.end() );
//    port2chan[&p] = &chan;
//    chan2ports[&chan].push_back(&p);
    p(chan.getEntry());
  }
  
  const smoc_node_list getNodes() const;
  const smoc_chan_list getChans() const;
  
#ifndef __SCFE__
  sc_module *myModule();

  void pgAssemble(smoc_modes::PGWriter &, const smoc_root_node *) const;
  void assembleActor( smoc_modes::PGWriter &pgw ) const;
#endif
};
#undef T_chan_init_default

#endif // _INCLUDED_SMOC_GRAPH_TYPE_HPP
