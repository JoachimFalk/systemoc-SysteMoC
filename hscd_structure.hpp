// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_STRUCTURE_HPP
#define _INCLUDED_HSCD_STRUCTURE_HPP

#include <hscd_port.hpp>
#include <hscd_fifo.hpp>
#include <hscd_node_types.hpp>
#include <hscd_pggen.hpp>

#include <list>
#include <map>

template <typename T_node_type>
class hscd_structure
  : public sc_module,
    public hscd_modes::hscd_modes_base_structure {
  public:
    typedef T_node_type					node_type;
    typedef hscd_structure<node_type>			this_type;
    
    typedef std::list<node_type *>			nodes_ty;
    typedef std::list<hscd_root_port *>			ports_ty;
    typedef std::map<sc_prim_channel *, ports_ty>       chan2ports_ty;
  private:
    typedef std::map<hscd_root_port *, sc_prim_channel *> port2chan_ty;
    
    nodes_ty		nodes;
    port2chan_ty	port2chan;
    chan2ports_ty	chan2ports;
  protected:
    template <typename T_chan_type>
    void connectNodePorts(
        hscd_port_out<typename T_chan_type::data_type> &b,
        hscd_port_in<typename T_chan_type::data_type> &a ) {
      port2chan_ty::iterator find_iter_a = port2chan.find(&a);
      port2chan_ty::iterator find_iter_b = port2chan.find(&b);
      
      assert( find_iter_a == port2chan.end() ||
	      find_iter_b == port2chan.end() );
      if ( find_iter_a != port2chan.end() ) {
        T_chan_type *c = dynamic_cast<T_chan_type *>(find_iter_a->second);
	assert(c != NULL);
        ports_ty &ports = chan2ports[c];
	b(*c); port2chan[&b] = c; ports.push_back(&b);
      } else if ( find_iter_b != port2chan.end() ) {
        T_chan_type *c = dynamic_cast<T_chan_type *>(find_iter_b->second);
	assert(c != NULL);
        ports_ty &ports = chan2ports[c];
	a(*c); port2chan[&a] = c; ports.push_back(&a);
      } else {
	T_chan_type *c = new T_chan_type();
        ports_ty &ports = chan2ports[c];
	a(*c); port2chan[&a] = c; ports.push_back(&a);
	b(*c); port2chan[&b] = c; ports.push_back(&b);
      }
    }
    template <typename T_value_type>
    void connectInterfacePorts(
        hscd_port_out<T_value_type> &a,
        hscd_port_out<T_value_type> &b ) { b.bind(a); }
    template <typename T_value_type>
    void connectInterfacePorts(
        hscd_port_in<T_value_type> &a,
        hscd_port_in<T_value_type> &b ) { b.bind(a); }
  public:
    hscd_structure()
      : sc_module( sc_gen_unique_name( "hscd_structure" ) ) {}
    
    explicit hscd_structure( sc_module_name name_ )
      : sc_module( name_ ) {}
    
    template <typename T>
    T &registerNode( T *node ) {
      nodes.push_back(node);
      return *node;
    }
    
    const nodes_ty      &getNodes() const { return nodes; }
    const chan2ports_ty &getChans() const { return chan2ports; }
    
    void assemble( hscd_modes::PGWriter &pgw ) const;
};

/*
template <typename T_node_type, typename T_chan_type, char *tn = "hscd_structure_channel" >
class hscd_structure_channel
  : public hscd_structure<T_node_type> {
  public:
    typedef T_node_type                                   node_type;
    typedef T_chan_type                                   chan_type;
    typedef hscd_structure_channel<node_type, chan_type>  this_type;
  public:
    hscd_structure_channel()
      : hscd_structure<node_type>( sc_gen_unique_name( tn ) ) {}
    
    explicit hscd_structure_channel( sc_module_name name_ )
      : hscd_structure<node_type>( name_ ) {}
    
    template <typename T_value_type>
    void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
      hscd_structure<node_type>::connectNodePorts<chan_type<T_value_type> >(a,b);
    }
    template <int n, typename T_value_type>
    void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
      hscd_structure<hscd_fixed_transact_node>::connectNodePorts<hscd_fifo<T_value_type,n> >(a,b);
    }
};*/

class hscd_sdf_structure
  : public hscd_structure<hscd_fixed_transact_node> {
  public:
    typedef hscd_sdf_structure	this_type;
  public:
    hscd_sdf_structure()
      : hscd_structure<hscd_fixed_transact_node>( sc_gen_unique_name( "hscd_sdf_structure" ) ) {}
    
    explicit hscd_sdf_structure( sc_module_name name_ )
      : hscd_structure<hscd_fixed_transact_node>( name_ ) {}
    
    template <typename T_value_type>
    void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
      hscd_structure<hscd_fixed_transact_node>::connectNodePorts<hscd_fifo<T_value_type> >(a,b);
    }
    template <int n, typename T_value_type>
    void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
      hscd_structure<hscd_fixed_transact_node>::connectNodePorts<hscd_fifo<T_value_type,n> >(a,b);
    }
};

class hscd_fifocsp_structure
  : public hscd_structure<hscd_choice_node> {
  public:
    typedef hscd_fifocsp_structure	this_type;
  public:
    hscd_fifocsp_structure()
      : hscd_structure<hscd_choice_node>( sc_gen_unique_name( "hscd_fifocsp_structure" ) ) {}
    
    explicit hscd_fifocsp_structure( sc_module_name name_ )
      : hscd_structure<hscd_choice_node>( name_ ) {}
    
    template <typename T_value_type>
    void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
      hscd_structure<hscd_choice_node>::connectNodePorts<hscd_fifo<T_value_type> >(a,b);
    }
    template <int n, typename T_value_type>
    void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
      hscd_structure<hscd_choice_node>::connectNodePorts<hscd_fifo<T_value_type,n> >(a,b);
    }
};

#endif // _INCLUDED_HSCD_STRUCTURE_HPP
