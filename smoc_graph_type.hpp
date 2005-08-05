// vim: set sw=2 ts=8:

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
  :
#ifndef __SCFE__
    public smoc_modes::smoc_modes_base_structure,
#endif
    public sc_module {
public:
  typedef T_node_type					node_type;
  typedef T_chan_kind					chan_kind;
  typedef smoc_graph_petri
    <T_node_type, T_chan_kind, T_chan_init_default>     this_type;
  
  typedef std::list<node_type *>			nodes_ty;
  typedef std::list<chan_kind *>                        chans_ty;
private:
  typedef std::map<smoc_root_port *, smoc_root_port *>  iobind_ty;
  
  iobind_ty           iobind;
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
      smoc_port_out<T_value_type> &b ) {
    assert( iobind.find(&b) == iobind.end() );
    iobind.insert( iobind_ty::value_type(&b,&a) );
    b.bind(a);
  }
  template <typename T_value_type>
  void connectInterfacePorts(
      smoc_port_in<T_value_type> &a,
      smoc_port_in<T_value_type> &b ) {
    assert( iobind.find(&b) == iobind.end() );
    iobind.insert( iobind_ty::value_type(&b,&a) );
    b.bind(a);
  }

  void finalise() {
    nodes_ty nodes = getNodes();
    
    for ( typename nodes_ty::iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter )
      static_cast<smoc_root_node *>(*iter)->finalise();
  }

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
  
  const nodes_ty getNodes() const {
    nodes_ty subnodes;
    for ( sc_pvector<sc_object*>::const_iterator iter = get_child_objects().begin();
          iter != get_child_objects().end();
          ++iter ) {
      node_type *node = dynamic_cast<node_type *>(*iter);
      
      if (node)
        subnodes.push_back(node);
    }
    return subnodes;
  }
  const chans_ty getChans() const {
    chans_ty channels;
    for ( sc_pvector<sc_object*>::const_iterator iter = get_child_objects().begin();
          iter != get_child_objects().end();
          ++iter ) {
      chan_kind *chan = dynamic_cast<chan_kind *>(*iter);
      
      if (chan)
        channels.push_back(chan);
    }
    return channels;
  }
 
#ifndef __SCFE__
  void assemble( smoc_modes::PGWriter &pgw ) const;
  void pgAssemble( smoc_modes::PGWriter &pgw ) const;
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

typedef smoc_graph_dataflow<smoc_fixed_transact_node, smoc_fifo_kind, smoc_fifo>
          smoc_sdf_constraintset;
typedef smoc_graph_dataflow<smoc_transact_node, smoc_fifo_kind, smoc_fifo>
          smoc_ddf_constraintset;
typedef smoc_graph_dataflow<smoc_choice_node, smoc_fifo_kind, smoc_fifo>
          smoc_ndf_constraintset;
/*
typedef smoc_graph_dataflow<smoc_choice_node, smoc_rendezvous_kind, smoc_rendezvous>
          smoc_csp_constraintset;
*/

#endif // _INCLUDED_SMOC_GRAPH_TYPE_HPP
