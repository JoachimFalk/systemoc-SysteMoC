// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _INCLUDED_SMOC_GRAPH_TYPE_HPP
#define _INCLUDED_SMOC_GRAPH_TYPE_HPP

#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_rendezvous.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

#include <systemc.h>

#include <list>
#include <map>

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
class smoc_graph_petri
: public sc_module {
public:
  typedef T_node_type					node_type;
  typedef T_chan_kind					chan_kind;
  typedef smoc_graph_petri
    <T_node_type, T_chan_kind, T_chan_init_default>     this_type;
protected:
  template <typename T_chan_init>
  void connectNodePorts(
      smoc_port_out<typename T_chan_init::data_type> &b,
      smoc_port_in<typename T_chan_init::data_type>  &a,
      const T_chan_init i ) {
    typename T_chan_init::chan_type &chan =
      registerChan<T_chan_init>(i);
    connectChanPort(chan,a);
    connectChanPort(chan,b);
  }
  template <int i, typename T_data_type>
  void connectNodePorts(
      smoc_port_out<T_data_type> &b,
      smoc_port_in<T_data_type>  &a ) {
    typename T_chan_init_default<T_data_type>::chan_type &chan =
      registerChan( T_chan_init_default<T_data_type>(i) );
    connectChanPort(chan,a);
    connectChanPort(chan,b);
  }
  template <typename T_data_type>
  void connectNodePorts(
      smoc_port_out<T_data_type> &b,
      smoc_port_in<T_data_type>  &a ) {
    typename T_chan_init_default<T_data_type>::chan_type &chan =
      registerChan( T_chan_init_default<T_data_type>() );
    connectChanPort(chan,a);
    connectChanPort(chan,b);
  }
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
  
  void finalise();
public:
  explicit smoc_graph_petri( sc_module_name name )
    : sc_module( name ) {}
  smoc_graph_petri()
    : sc_module(
        sc_gen_unique_name("smoc_graph_petri") ) {}
  template <typename T>
  T &registerNode( T *node ) {
    return *node;
  }
  
  template <typename T_chan_init>
  typename T_chan_init::chan_type &registerChan( const T_chan_init i ) {
    typename T_chan_init::chan_type *chan =
      new typename T_chan_init::chan_type(i);
//    chan2ports[chan];
    return *chan;
  }
  template <typename T_chan_type>
  void connectChanPort( T_chan_type &chan,
                        smoc_port_out<typename T_chan_type::data_type> &p ) {
//    assert( port2chan.find(&p) ==  port2chan.end() );
//    port2chan[&p] = &chan;
//    chan2ports[&chan].push_back(&p);
    p(chan);
  }
  template <typename T_chan_type>
  void connectChanPort( T_chan_type &chan,
                        smoc_port_in<typename T_chan_type::data_type> &p ) {
//    assert( port2chan.find(&p) ==  port2chan.end() );
//    port2chan[&p] = &chan;
//    chan2ports[&chan].push_back(&p);
    p(chan);
  }
  
  const smoc_node_list getNodes() const;
  const smoc_chan_list getChans() const;
  
#ifndef __SCFE__
  void pgAssemble(smoc_modes::PGWriter &, const smoc_root_node *) const;
#endif
};

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
class smoc_graph_dataflow
  : public smoc_graph_petri<T_node_type,T_chan_kind,T_chan_init_default> {
public:
  typedef smoc_graph_dataflow
    <T_node_type, T_chan_kind, T_chan_init_default> this_type;
  
  explicit smoc_graph_dataflow( sc_module_name name )
    : smoc_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>(name) {}
  smoc_graph_dataflow()
    : smoc_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>(
        sc_gen_unique_name("smoc_graph_dataflow") ) {}
private:
  // disable
  template <typename T_chan_type>
  void connectChanPort( T_chan_type &chan,
                        smoc_port_out<typename T_chan_type::data_type> &p );
  template <typename T_chan_type>
  void connectChanPort( T_chan_type &chan,
                        smoc_port_in<typename T_chan_type::data_type> &p );
};

typedef smoc_graph_dataflow<smoc_root_node, smoc_fifo_kind, smoc_fifo>
          smoc_sdf_constraintset;
typedef smoc_graph_dataflow<smoc_root_node, smoc_fifo_kind, smoc_fifo>
          smoc_ddf_constraintset;
typedef smoc_graph_dataflow<smoc_root_node, smoc_fifo_kind, smoc_fifo>
          smoc_ndf_constraintset;
/*
typedef smoc_graph_dataflow<smoc_fixed_transact_node, smoc_fifo_kind, smoc_fifo>
          smoc_sdf_constraintset;
typedef smoc_graph_dataflow<smoc_transact_node, smoc_fifo_kind, smoc_fifo>
          smoc_ddf_constraintset;
typedef smoc_graph_dataflow<smoc_choice_node, smoc_fifo_kind, smoc_fifo>
          smoc_ndf_constraintset;

typedef smoc_graph_dataflow<smoc_choice_node, smoc_rendezvous_kind, smoc_rendezvous>
          smoc_csp_constraintset;
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

#endif // _INCLUDED_SMOC_GRAPH_TYPE_HPP
