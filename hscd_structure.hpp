// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
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
    : smoc_top_moc<smoc_graph>(name) { }
  hscd_graph()
    : smoc_top_moc<smoc_graph>( sc_gen_unique_name("hscd_graph") ) { }

  template <typename T_chan_init>
  void connectNodePorts(
      smoc_port_out<typename T_chan_init::data_type> &b,
      smoc_port_in<typename T_chan_init::data_type>  &a,
      const T_chan_init i ) {
    smoc_graph::connectNodePorts(
      static_cast<smoc_port_out<typename T_chan_init::data_type> &>(b),
      static_cast<smoc_port_in <typename T_chan_init::data_type> &>(a),
      i );
  }
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
