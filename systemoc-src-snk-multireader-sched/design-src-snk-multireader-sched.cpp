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

#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_node_types.hpp>
#include <systemoc/smoc_multireader_fifo.hpp>

template <typename T,size_t N>
class m_h_src: public smoc_actor {
public:
  smoc_port_out<T> out;
private:
  T       i;
  size_t  iter;
  
  void src() {
    for(size_t j = 0; j < N; ++j) {
      std::cout
        << name() << ": generate token with id "
        << out.tokenId(j) << " with value " << i << std::endl;
      out[j] = i++; --iter;
    }
  }
  smoc_firing_state start;
public:
  m_h_src(sc_module_name name, size_t _iter)
    : smoc_actor(name, start),
      i(1), iter(_iter) {
    start =
         (out(N) && VAR(iter) > (N-1))
      >> CALL(m_h_src::src)       >> start;
  }
};


template <typename T,size_t N>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  void sink() {
    for(size_t i = 0; i < N; ++i) {
      std::cout
        << name() << ": received token with id "
        << in.tokenId(i) << " and value " << in[i] << std::endl;
    }
  }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_module_name name)
    : smoc_actor(name, start) {
    start = in(N) >> CALL(m_h_sink::sink) >> start;
  }
};

class MyScheduler : public smoc_multireader_scheduler {
protected:
  bool canNotify(const sc_port_base*) {
    return true;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_src<int,4>     src;
  m_h_sink<int,3>    snk1;
  m_h_sink<int,2>    snk2;
  m_h_sink<int,1>    snk3;
  MyScheduler os;
public:
  m_h_top(sc_module_name name, size_t iter)
    : smoc_graph(name),
      src("src", iter),
      snk1("snk1"),
      snk2("snk2"),
      snk3("snk3")
  {
    smoc_multireader_fifo<int> f(4,&os); f << -1;

    connector(f) << src.out << snk1.in << snk2.in << snk3.in;

  }
};

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(-1);
  
  if (argc >= 2)
    iter = atol(argv[1]);
  
  smoc_top_moc<m_h_top> top("top", iter);
  sc_start();
  return 0;
}
