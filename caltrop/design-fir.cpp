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

template <typename T>
class src: public smoc_actor {
public:
  smoc_port_out<T> out;
private:
  T i;
  
  void src() {
    std::cout << "src: " << i << std::endl;
    out[0] = i++;
  }
  smoc_firing_state start;
public:
  src(sc_module_name name)
    : smoc_actor(name, start),
      i(1) {
    start = (out.getAvailableSpace() >= 1) >>
            call(&m_h_src::src)            >> start;
  }
};

template <typename T>
class SimpleFIR: public smoc_actor {
  // code as shown before
}

template <typename T>
class sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  int i;
  
  void sink(void) { std::cout << "sink: " << in[0] << std::endl; }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_module_name name)
    : smoc_actor(name, start) {
    start = (in.getAvailableTokens() >= 1) >>
            call(&m_h_sink::sink)          >> start;
  }
};

class top: public smoc_graph {
protected:
  src<double>         s;
  SimpleFIR<double>   f;
  sink<double>        d;
  
  smoc_fifo<double>   f1;
  smoc_fifo<double>   f2;
public:
  static std::vector<double> gentaps() {
    std::vector<double> retval;
    
    // vector [0,0,1]
    retval.push_back(0);
    retval.push_back(0);
    retval.push_back(1);
    return retval;
  }
  
  top( sc_module_name name )
    : smoc_graph(name),
      s("s"),
      f("f", gentaps()),
      d("d") {
    s.out(f1); f.input(f1); // s.out -> f.input
    f.output(f2); d.in(f2); // f.output -> d.in
  }
};

int sc_main (int argc, char **argv) {
  smoc_top<top> t("t");
  
  sc_start(-1);
  return 0;
}
