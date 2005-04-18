// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_GRAPH_TYPE_HPP
#define _INCLUDED_HSCD_GRAPH_TYPE_HPP

#include <hscd_port.hpp>
#include <hscd_fifo.hpp>
#include <hscd_rendezvous.hpp>
#include <hscd_node_types.hpp>
#ifndef __SCFE__
# include <hscd_pggen.hpp>
#endif

#include <systemc.h>

#include <list>
#include <map>

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
class hscd_graph_petri
  :
#ifndef __SCFE__
    public hscd_modes::hscd_modes_base_structure,
#endif
    public sc_module {
public:
  typedef T_node_type					node_type;
  typedef T_chan_kind					chan_kind;
  typedef hscd_graph_petri
    <T_node_type, T_chan_kind, T_chan_init_default>     this_type;
  
  typedef std::list<node_type *>			nodes_ty;
  typedef std::list<chan_kind *>                        chans_ty;
private:
  typedef std::map<hscd_root_port *, hscd_root_port *>  iobind_ty;
  
  iobind_ty           iobind;
protected:
  template <typename T_chan_init>
  void connectNodePorts(
      hscd_port_out<typename T_chan_init::data_type> &b,
      hscd_port_in<typename T_chan_init::data_type>  &a,
      const T_chan_init i ) {
    typename T_chan_init::chan_type &chan =
      registerChan<T_chan_init>(i);
    connectChanPort(chan,a);
    connectChanPort(chan,b);
  }
  template <typename T_data_type>
  void connectNodePorts(
      hscd_port_out<T_data_type> &b,
      hscd_port_in<T_data_type>  &a ) {
    typename T_chan_init_default<T_data_type>::chan_type &chan =
      registerChan( T_chan_init_default<T_data_type>() );
    connectChanPort(chan,a);
    connectChanPort(chan,b);
  }
  template <typename T_value_type>
  void connectInterfacePorts(
      hscd_port_out<T_value_type> &a,
      hscd_port_out<T_value_type> &b ) {
    assert( iobind.find(&b) == iobind.end() );
    iobind.insert( iobind_ty::value_type(&b,&a) );
    b.bind(a);
  }
  template <typename T_value_type>
  void connectInterfacePorts(
      hscd_port_in<T_value_type> &a,
      hscd_port_in<T_value_type> &b ) {
    assert( iobind.find(&b) == iobind.end() );
    iobind.insert( iobind_ty::value_type(&b,&a) );
    b.bind(a);
  }

  void finalise() {
    nodes_ty nodes = getNodes();
    
    for ( typename nodes_ty::iterator iter = nodes.begin();
          iter != nodes.end();
          ++iter )
      (*iter)->finalise();
  }

public:
  explicit hscd_graph_petri( sc_module_name name )
    : sc_module( name ) {}
  hscd_graph_petri()
    : sc_module(
        sc_gen_unique_name("hscd_graph_petri") ) {}
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
                        hscd_port_out<typename T_chan_type::data_type> &p ) {
//    assert( port2chan.find(&p) ==  port2chan.end() );
//    port2chan[&p] = &chan;
//    chan2ports[&chan].push_back(&p);
    p(chan);
  }
  template <typename T_chan_type>
  void connectChanPort( T_chan_type &chan,
                        hscd_port_in<typename T_chan_type::data_type> &p ) {
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
  void assemble( hscd_modes::PGWriter &pgw ) const;
  void pgAssemble( hscd_modes::PGWriter &pgw ) const;
#endif
};

template <typename T_node_type,
          typename T_chan_kind,
          template <typename T_value_type> class T_chan_init_default>
class hscd_graph_dataflow
  : public hscd_graph_petri<T_node_type,T_chan_kind,T_chan_init_default> {
public:
  typedef hscd_graph_dataflow
    <T_node_type, T_chan_kind, T_chan_init_default> this_type;
  
  explicit hscd_graph_dataflow( sc_module_name name )
    : hscd_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>(name) {}
  hscd_graph_dataflow()
    : hscd_graph_petri<T_node_type, T_chan_kind, T_chan_init_default>(
        sc_gen_unique_name("hscd_graph_dataflow") ) {}
private:
  // disable
  template <typename T_chan_type>
  void connectChanPort( T_chan_type &chan,
                        hscd_port_out<typename T_chan_type::data_type> &p );
  template <typename T_chan_type>
  void connectChanPort( T_chan_type &chan,
                        hscd_port_in<typename T_chan_type::data_type> &p );
};

typedef hscd_graph_dataflow<hscd_fixed_transact_node, hscd_fifo_kind, hscd_fifo>
          hscd_sdf_constraintset;
typedef hscd_graph_dataflow<hscd_transact_node, hscd_fifo_kind, hscd_fifo>
          hscd_kpn_constraintset;
typedef hscd_graph_dataflow<hscd_choice_node, hscd_fifo_kind, hscd_fifo>
          hscd_ddf_constraintset;
typedef hscd_graph_dataflow<hscd_choice_node, hscd_rendezvous_kind, hscd_rendezvous>
          hscd_csp_constraintset;

#endif // _INCLUDED_HSCD_GRAPH_TYPE_HPP
