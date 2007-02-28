// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 * 
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
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

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <systemoc/smoc_pggen.hpp>
#endif

template <typename T>
class m_h_src: public smoc_actor {
public:
  smoc_port_out<T> out;
private:
  T i;
  
  void src() {
    std::cout << name() << ": " << i << std::endl;
    out[0] = i++;
  }
  smoc_firing_state start;
public:
  m_h_src(sc_module_name name)
    : smoc_actor(name, start),
      i(1) {
    start = out(1) >> call(&m_h_src::src) >> start;
  }
};


template <typename T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  int i;
  
  void sink(void) { std::cout << name() << ": " << in[0] << std::endl; }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_module_name name)
    : smoc_actor(name, start) {
    start = in(1) >> call(&m_h_sink::sink) >> start;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_src<int>     src;
  m_h_sink<int>    snk;
public:
  m_h_top( sc_module_name name )
    : smoc_graph(name),
      src("src"), snk("snk") {
    connectNodePorts(src.out, snk.in);
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_h_top> top("top");
  sc_start(-1);
  return 0;
}
