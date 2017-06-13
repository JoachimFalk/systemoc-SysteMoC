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
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph.hpp>

template <typename T>
class m_h_copy: public smoc_actor {
public:
  smoc_port_in<T> in;
  smoc_port_out<T> out;
private:
  void copy() {
    std::cout
      << name() << ": copy token with value " << in[0] << std::endl;
    out[0] = in[0];
  }
  smoc_firing_state run;
public:
  m_h_copy(sc_core::sc_module_name name)
    : smoc_actor(name, run)
  {
    run =
         in(1)
      >> out(1)
      >> SMOC_CALL(m_h_copy::copy)
      >> run;
  }
};

template <typename T>
class m_h_src: public smoc_actor {
public:
  smoc_port_out<T> out;
private:
  T       i;
  size_t  iter;
  
  void src() {
    std::cout
      << name() << ": generate token with value " << i << std::endl;
    out[0] = i++; --iter;
  }
  smoc_firing_state start;
public:
  m_h_src(sc_core::sc_module_name name, size_t _iter)
    : smoc_actor(name, start),
      i(1), iter(_iter) {
    start =
         (out(1) && SMOC_VAR(iter) > 0U)
      >> SMOC_CALL(m_h_src::src)       >> start;
  }
};


template <typename T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  int i;
  
  void sink(void) {
    std::cout
      << name() << ": received token with value " << in[0] << std::endl;
  }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_core::sc_module_name name)
    : smoc_actor(name, start) {
    start = in(1) >> SMOC_CALL(m_h_sink::sink) >> start;
  }
};

class m_h_inner: public smoc_graph {
public:
  smoc_port_in<int> in;
  smoc_port_out<int> out;
protected:
  m_h_copy<int> cpy;
public:
  m_h_inner(sc_core::sc_module_name name)
    : smoc_graph(name),
      cpy("cpy") {
    cpy.in(in);
    cpy.out(out);
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_src<int> src;
  m_h_inner inner;
  m_h_sink<int> snk;
public:
  m_h_top(sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name),
      src("src", iter), inner("inner"), snk("snk") {
    connectNodePorts(src.out, inner.in);
    connectNodePorts(inner.out, snk.in);
  }
};

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(-1);
  
  if (argc >= 2)
    iter = atol(argv[1]);
  
  smoc_top_moc<m_h_top> top("top", iter);
  sc_core::sc_start();
  return 0;
}
