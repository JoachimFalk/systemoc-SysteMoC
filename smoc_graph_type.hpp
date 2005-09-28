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

template <typename T>
class smoc_channel_hack: public smoc_actor {
public:
  typedef smoc_channel_hack<T> this_type;
  
  smoc_port_in<T>  i;
  smoc_port_out<T> o;
private:
  void process() { o[0] = i[0]; }
  
  smoc_firing_state start;
public:
  smoc_channel_hack( sc_module_name name )
    : smoc_actor( name, start ) {
    start =
         i(1)
      >> o(1)
      >> CALL(smoc_channel_hack::process)
      >> start
    ;
  }
};

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
    // BIG HACK !!!
    smoc_channel_hack<typename T_chan_init::data_type> &hack =
      *new smoc_channel_hack<typename T_chan_init::data_type>(
         sc_gen_unique_name("smoc_comm_process") );
    
    typename T_chan_init::chan_type &chan1 =
      registerChan<T_chan_init>(i);
    typename T_chan_init::chan_type &chan2 =
      registerChan<T_chan_init>(i);
    connectChanPort(chan1,a);
    connectChanPort(chan1,hack.o);
    connectChanPort(chan2,hack.i);
    connectChanPort(chan2,b);
  }
  template <typename T_data_type>
  void connectNodePorts(
      smoc_port_out<T_data_type> &b,
      smoc_port_in<T_data_type>  &a ) {
    // BIG HACK !!!
    smoc_channel_hack<T_data_type> &hack =
      *new smoc_channel_hack<T_data_type>(
         sc_gen_unique_name("smoc_comm_process") );
    
    typename T_chan_init_default<T_data_type>::chan_type &chan1 =
      registerChan( T_chan_init_default<T_data_type>() );
    typename T_chan_init_default<T_data_type>::chan_type &chan2 =
      registerChan( T_chan_init_default<T_data_type>() );
    connectChanPort(chan1,a);
    connectChanPort(chan1,hack.o);
    connectChanPort(chan2,hack.i);
    connectChanPort(chan2,b);
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

#endif // _INCLUDED_SMOC_GRAPH_TYPE_HPP
