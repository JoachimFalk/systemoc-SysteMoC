// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_STRUCTURE_HPP
#define _INCLUDED_HSCD_STRUCTURE_HPP

#include <hscd_fifo.hpp>
#include <hscd_rendezvous.hpp>
#include <hscd_node_types.hpp>
#include <hscd_graph_type.hpp>

#include <list>
#include <map>

class hscd_sdf_structure
  : public hscd_graph_sdf<hscd_fixed_transact_node, hscd_fifo_kind, hscd_fifo>,
    public hscd_fixed_transact_node {
public:
  typedef hscd_sdf_structure	this_type;
public:
  hscd_sdf_structure( sc_module_name name )
    : hscd_graph_sdf<hscd_fixed_transact_node, hscd_fifo_kind, hscd_fifo>(name),
      hscd_fixed_transact_node( hscd_op_transact() ) {}
  
  template <typename T_value_type>
  void connectNodePorts( hscd_port_out<T_value_type> &a,
                         hscd_port_in<T_value_type>  &b,
                         hscd_fifo<T_value_type>      i = hscd_fifo<T_value_type>() ) {
    hscd_graph_sdf<hscd_fixed_transact_node, hscd_fifo_kind, hscd_fifo>::
      connectNodePorts(a,b,i);
  }
  template <int n, typename T_value_type>
  void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
    hscd_graph_sdf<hscd_fixed_transact_node, hscd_fifo_kind, hscd_fifo>::
      connectNodePorts(a,b,hscd_fifo<T_value_type>(n));
  }
  
#ifndef __SCFE__
  void assemble( hscd_modes::PGWriter &pgw ) const {
    return hscd_graph_sdf<hscd_fixed_transact_node, hscd_fifo_kind, hscd_fifo>::assemble(pgw);
  }
  void pgAssemble( sc_module *m, hscd_modes::PGWriter &pgw ) const {
    return hscd_graph_sdf<hscd_fixed_transact_node, hscd_fifo_kind, hscd_fifo>::pgAssemble(pgw);
  }
#endif
protected:
  void process() {}
};

class hscd_fifocsp_structure
  : public hscd_graph_sdf<hscd_choice_node, hscd_fifo_kind, hscd_fifo>,
    public hscd_choice_node {
public:
  typedef hscd_fifocsp_structure	this_type;
public:
  hscd_fifocsp_structure( sc_module_name name )
    : hscd_graph_sdf<hscd_choice_node, hscd_fifo_kind, hscd_fifo>(name),
      hscd_choice_node() {}
  
  template <typename T_value_type>
  void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
    hscd_graph_sdf<hscd_choice_node, hscd_fifo_kind, hscd_fifo>::
      connectNodePorts(a,b,hscd_fifo<T_value_type>());
  }
  template <int n, typename T_value_type>
  void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
    hscd_graph_sdf<hscd_choice_node, hscd_fifo_kind, hscd_fifo>::
      connectNodePorts(a,b,hscd_fifo<T_value_type>(n));
  }

#ifndef __SCFE__
  void assemble( hscd_modes::PGWriter &pgw ) const {
    return hscd_graph_sdf<hscd_choice_node, hscd_fifo_kind, hscd_fifo>::assemble(pgw);
  }
  void pgAssemble( sc_module *m, hscd_modes::PGWriter &pgw ) const {
    return hscd_graph_sdf<hscd_choice_node, hscd_fifo_kind, hscd_fifo>::pgAssemble(pgw);
  }
#endif
protected:
  void process() {}
};

class hscd_csp_structure
  : public hscd_graph_sdf<hscd_choice_node, hscd_rendezvous_kind, hscd_rendezvous>,
    public hscd_choice_node {
public:
  typedef hscd_csp_structure	this_type;
public:
  hscd_csp_structure( sc_module_name name )
    : hscd_graph_sdf<hscd_choice_node, hscd_rendezvous_kind, hscd_rendezvous>(name),
      hscd_choice_node() {}
  
  template <typename T_value_type>
  void connectNodePorts( hscd_port_out<T_value_type> &a, hscd_port_in<T_value_type> &b ) {
    hscd_graph_sdf<hscd_choice_node, hscd_rendezvous_kind, hscd_rendezvous>::
      connectNodePorts(a,b,hscd_rendezvous<T_value_type>());
  }
  
#ifndef __SCFE__
  void assemble( hscd_modes::PGWriter &pgw ) const {
    return hscd_graph_sdf<hscd_choice_node, hscd_rendezvous_kind, hscd_rendezvous>::assemble(pgw);
  }
  void pgAssemble( sc_module *m, hscd_modes::PGWriter &pgw ) const {
    return hscd_graph_sdf<hscd_choice_node, hscd_rendezvous_kind, hscd_rendezvous>::pgAssemble(pgw);
  }
#endif
protected:
  void process() {}
};

#endif // _INCLUDED_HSCD_STRUCTURE_HPP
