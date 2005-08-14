// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_STRUCTURE_HPP
#define _INCLUDED_HSCD_STRUCTURE_HPP

#include <hscd_fifo.hpp>
//#include <hscd_rendezvous.hpp>
#include <hscd_node_types.hpp>

#include <smoc_graph_type.hpp>
#include <smoc_moc.hpp>

class hscd_graph
: public smoc_top_moc<smoc_graph> {
public:
  typedef hscd_graph this_type;
  
  
public:
  explicit hscd_graph( sc_module_name name )
    : smoc_top_moc<smoc_graph>(name) { is_v1_actor = true; }
  hscd_graph()
    : smoc_top_moc<smoc_graph>( sc_gen_unique_name("hscd_graph") ) { is_v1_actor = true; }

/*
  template <typename T_chan_init>
  void connectNodePorts(
      hscd_port_out<typename T_chan_init::data_type> &b,
      hscd_port_in<typename T_chan_init::data_type>  &a,
      const T_chan_init i ) {
    smoc_graph::connectNodePorts(
      static_cast<smoc_port_out<typename T_chan_init::data_type> &>(b),
      static_cast<smoc_port_in <typename T_chan_init::data_type> &>(a),
      i );
  }*/
  template <int n, typename T_data_type>
  void connectNodePorts(
      smoc_port_out<T_data_type> &b,
      smoc_port_in<T_data_type> &a ) {
    smoc_graph::connectNodePorts(
      static_cast<smoc_port_out<T_data_type> &>(b),
      static_cast<smoc_port_in <T_data_type> &>(a),
      hscd_fifo<T_data_type>(n) );
  }
  template <typename T_data_type>
  void connectNodePorts(
      smoc_port_out<T_data_type> &b,
      smoc_port_in<T_data_type>  &a ) {
    smoc_graph::connectNodePorts(
      static_cast<smoc_port_out<T_data_type> &>(b),
      static_cast<smoc_port_in <T_data_type> &>(a) );
  }
};

typedef hscd_graph hscd_sdf_structure;

typedef hscd_graph hscd_fifocsp_structure;

// typedef smoc_csp_constraintset hscd_csp_structure;

#endif // _INCLUDED_HSCD_STRUCTURE_HPP
