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
