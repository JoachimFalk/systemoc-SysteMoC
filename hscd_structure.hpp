// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_STRUCTURE_HPP
#define _INCLUDED_HSCD_STRUCTURE_HPP

#include <hscd_port.hpp>
#include <hscd_fifo.hpp>
#include <hscd_node_types.hpp>
#include <hscd_pggen.hpp>

#include <list>
#include <map>

template <typename T_node_type, typename T_chan_init>
class hscd_structure {
  public:
    typedef T_node_type					  node_type;
    typedef T_chan_init					  chan_init;
    typedef typename T_chan_init::chan_kind               chan_kind;
    typedef hscd_structure<node_type, chan_init>	  this_type;
    
    typedef std::list<node_type *>			  nodes_ty;
    typedef std::list<hscd_root_port *>			  ports_ty;
    typedef std::map<chan_kind *, ports_ty>               chan2ports_ty;
  private:
    typedef std::map<hscd_root_port *, hscd_root_port *>  iobind_ty;
    typedef std::map<hscd_root_port *, chan_kind *>       port2chan_ty;
    
    nodes_ty		nodes;
    chan2ports_ty	chan2ports;
    port2chan_ty	port2chan;
    iobind_ty           iobind;
  protected:
    template <typename T_value_type>
    void connectNodePorts(
        hscd_port_out<T_value_type> &b,
        hscd_port_in<T_value_type>  &a,
        const chan_init i = chan_init() ) {
      typename chan_init::chan_type<T_value_type> &chan = registerChan<T_value_type>(i);
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
    hscd_structure() {}
    
    template <typename T>
    T &registerNode( T *node ) {
      nodes.push_back(node);
      return *node;
    }
    
    template <typename T_value_type>
    typename chan_init::chan_type<T_value_type> &registerChan( const chan_init i = chan_init() ) {
      typename chan_init::chan_type<T_value_type> *chan =
        new typename chan_init::chan_type<T_value_type>(i);
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
    
    void assemble( const sc_module *m, hscd_modes::PGWriter &pgw ) const;
    void pgAssemble( const sc_module *m, hscd_modes::PGWriter &pgw ) const;
};

class hscd_sdf_structure
  : public hscd_structure<hscd_fixed_transact_node, hscd_fifo>,
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
    void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
      hscd_structure<hscd_fixed_transact_node, hscd_fifo>::connectNodePorts(a,b);
    }
    template <int n, typename T_value_type>
    void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
      hscd_structure<hscd_fixed_transact_node, hscd_fifo>::connectNodePorts(a,b,hscd_fifo(n));
    }
    
    void assemble( hscd_modes::PGWriter &pgw ) const {
      return hscd_structure<hscd_fixed_transact_node, hscd_fifo>::assemble(this,pgw);
    }
    void pgAssemble( sc_module *m, hscd_modes::PGWriter &pgw ) const {
      return hscd_structure<hscd_fixed_transact_node, hscd_fifo>::pgAssemble(this,pgw);
    }
  protected:
    void process() {}
};

class hscd_fifocsp_structure
  : public hscd_structure<hscd_choice_node, hscd_fifo>,
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
      hscd_structure<hscd_choice_node, hscd_fifo>::connectNodePorts(a,b);
    }
    template <int n, typename T_value_type>
    void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
      hscd_structure<hscd_choice_node, hscd_fifo>::connectNodePorts(a,b,hscd_fifo(n));
    }
  protected:
    void process() {}
};

#endif // _INCLUDED_HSCD_STRUCTURE_HPP
