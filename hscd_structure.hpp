// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_STRUCTURE_HPP
#define _INCLUDED_HSCD_STRUCTURE_HPP

#include <hscd_port.hpp>
#include <hscd_fifo.hpp>
#include <hscd_rendezvous.hpp>
#include <hscd_node_types.hpp>
#ifndef __SCFE__
# include <hscd_pggen.hpp>
#endif

#include <list>
#include <map>

template <typename T_node_type, typename T_chan_kind>
class hscd_structure_petri {
public:
  typedef T_node_type					node_type;
  typedef T_chan_kind					chan_kind;
  typedef hscd_structure_petri<node_type, chan_kind>	this_type;
  
  typedef std::list<node_type *>			nodes_ty;
  typedef std::list<hscd_root_port *>			ports_ty;
  typedef std::map<chan_kind *, ports_ty>               chan2ports_ty;
private:
  typedef std::map<hscd_root_port *, hscd_root_port *>  iobind_ty;
  typedef std::map<hscd_root_port *, chan_kind *>       port2chan_ty;
  
  nodes_ty		nodes;
  chan2ports_ty	chan2ports;
  port2chan_ty	port2chan;
  iobind_ty           iobind;
protected:
  template <typename T_chan_init>
  void connectNodePorts(
      hscd_port_out<typename T_chan_init::data_type> &b,
      hscd_port_in<typename T_chan_init::data_type>  &a,
      const T_chan_init i ) {
    typename T_chan_init::chan_type &chan = registerChan<T_chan_init>(i);
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
public:
  hscd_structure_petri() {}
  
  template <typename T>
  T &registerNode( T *node ) {
    nodes.push_back(node);
    return *node;
  }
  
  template <typename T_chan_init>
  typename T_chan_init::chan_type &registerChan( const T_chan_init i ) {
    typename T_chan_init::chan_type *chan =
      new typename T_chan_init::chan_type(i);
    chan2ports[chan];
    return *chan;
  }
  template <typename T_chan_type>
  void connectChanPort( T_chan_type &chan,
                        hscd_port_out<typename T_chan_type::data_type> &p ) {
    assert( port2chan.find(&p) ==  port2chan.end() );
    port2chan[&p] = &chan;
    chan2ports[&chan].push_back(&p);
    p(chan);
  }
  template <typename T_chan_type>
  void connectChanPort( T_chan_type &chan,
                        hscd_port_in<typename T_chan_type::data_type> &p ) {
    assert( port2chan.find(&p) ==  port2chan.end() );
    port2chan[&p] = &chan;
    chan2ports[&chan].push_back(&p);
    p(chan);
  }
  
  const nodes_ty      &getNodes() const { return nodes; }
  const chan2ports_ty &getChans() const { return chan2ports; }
 
#ifndef __SCFE__
  void assemble( const sc_module *m, hscd_modes::PGWriter &pgw ) const;
  void pgAssemble( const sc_module *m, hscd_modes::PGWriter &pgw ) const;
#endif
};

template <typename T_node_type, typename T_chan_kind>
class hscd_structure_sdf
  : public hscd_structure_petri<T_node_type,T_chan_kind> {
public:
  hscd_structure_sdf() {}
private:
  // disable
  template <typename T_chan_type>
  void connectChanPort( T_chan_type &chan,
                        hscd_port_out<typename T_chan_type::data_type> &p );
  template <typename T_chan_type>
  void connectChanPort( T_chan_type &chan,
                        hscd_port_in<typename T_chan_type::data_type> &p );
};

class hscd_sdf_structure
  : public hscd_structure_sdf<hscd_fixed_transact_node, hscd_fifo_kind>,
    public hscd_fixed_transact_node {
public:
  typedef hscd_sdf_structure	this_type;
public:
  hscd_sdf_structure()
    : hscd_fixed_transact_node( 
        sc_gen_unique_name("hscd_sdf_structure"), hscd_op_transact() ) {}
  explicit hscd_sdf_structure( sc_module_name _name )
    : hscd_fixed_transact_node( _name, hscd_op_transact() ) {}
  
  template <typename T_value_type>
  void connectNodePorts( hscd_port_out<T_value_type> &a,
                         hscd_port_in<T_value_type>  &b,
                         hscd_fifo<T_value_type>      i = hscd_fifo<T_value_type>() ) {
    hscd_structure_sdf<hscd_fixed_transact_node, hscd_fifo_kind>::
      connectNodePorts(a,b,i);
  }
  template <int n, typename T_value_type>
  void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
    hscd_structure_sdf<hscd_fixed_transact_node, hscd_fifo_kind>::
      connectNodePorts(a,b,hscd_fifo<T_value_type>(n));
  }
  
#ifndef __SCFE__
  void assemble( hscd_modes::PGWriter &pgw ) const {
    return hscd_structure_sdf<hscd_fixed_transact_node, hscd_fifo_kind>::assemble(this,pgw);
  }
  void pgAssemble( sc_module *m, hscd_modes::PGWriter &pgw ) const {
    return hscd_structure_sdf<hscd_fixed_transact_node, hscd_fifo_kind>::pgAssemble(this,pgw);
  }
#endif
protected:
  void process() {}
};

class hscd_fifocsp_structure
  : public hscd_structure_sdf<hscd_choice_node, hscd_fifo_kind>,
    public hscd_choice_node {
public:
  typedef hscd_fifocsp_structure	this_type;
public:
  hscd_fifocsp_structure()
    : hscd_choice_node(
        sc_gen_unique_name("hscd_fifocsp_structure") ) {}
  explicit hscd_fifocsp_structure( sc_module_name _name )
    : hscd_choice_node( _name ) {}
  
  template <typename T_value_type>
  void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
    hscd_structure_sdf<hscd_choice_node, hscd_fifo_kind>::
      connectNodePorts(a,b,hscd_fifo<T_value_type>());
  }
  template <int n, typename T_value_type>
  void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
    hscd_structure_sdf<hscd_choice_node, hscd_fifo_kind>::
      connectNodePorts(a,b,hscd_fifo<T_value_type>(n));
  }

#ifndef __SCFE__
  void assemble( hscd_modes::PGWriter &pgw ) const {
    return hscd_structure_sdf<hscd_choice_node, hscd_fifo_kind>::assemble(this,pgw);
  }
  void pgAssemble( sc_module *m, hscd_modes::PGWriter &pgw ) const {
    return hscd_structure_sdf<hscd_choice_node, hscd_fifo_kind>::pgAssemble(this,pgw);
  }
#endif
protected:
  void process() {}
};

class hscd_csp_structure
  : public hscd_structure_sdf<hscd_choice_node, hscd_rendezvous_kind>,
    public hscd_choice_node {
public:
  typedef hscd_csp_structure	this_type;
public:
  hscd_csp_structure()
    : hscd_choice_node(
        sc_gen_unique_name("hscd_csp_structure") ) {}
  explicit hscd_csp_structure( sc_module_name _name )
    : hscd_choice_node( _name ) {}
  
  template <typename T_value_type>
  void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
    hscd_structure_sdf<hscd_choice_node, hscd_rendezvous_kind>::
      connectNodePorts(a,b,hscd_rendezvous<T_value_type>());
  }

#ifndef __SCFE__
  void assemble( hscd_modes::PGWriter &pgw ) const {
    return hscd_structure_sdf<hscd_choice_node, hscd_rendezvous_kind>::assemble(this,pgw);
  }
  void pgAssemble( sc_module *m, hscd_modes::PGWriter &pgw ) const {
    return hscd_structure_sdf<hscd_choice_node, hscd_rendezvous_kind>::pgAssemble(this,pgw);
  }
#endif
protected:
  void process() {}
};

#endif // _INCLUDED_HSCD_STRUCTURE_HPP
