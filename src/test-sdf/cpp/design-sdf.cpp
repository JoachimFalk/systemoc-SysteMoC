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

class m_adder: public smoc_actor {
public:
  smoc_port_in<int>  in1;
  smoc_port_in<int>  in2;
  smoc_port_out<int> out;
private:
  void process() {
    int retval = in1[0] + in1[1] + in2[0];
    out[0] = retval;
    std::cout << name() << " adding "
        << in1[0] << " + "
        << in1[1] << " + "
        << in2[0] << " = " << retval << std::endl;
  }

  smoc_firing_state start;
public:
  m_adder( sc_core::sc_module_name name )
    :smoc_actor( name, start ) {
    start = (in1(2) && in2(1) && out(1)) >> SMOC_CALL(m_adder::process) >> start;
  }
};

//template <class T>
class m_multiply: public smoc_actor {
public:
  smoc_port_in<int>  in1;
  smoc_port_in<int>  in2;
  smoc_port_out<int> out1;
  smoc_port_out<int> out2;
private:
  void process() {
    int retval = in1[0] * in2[0];
    
    out1[0] = retval;
    out2[0] = retval;
    std::cout << name() << " multiplying "
        << in1[0] << " * "
        << in2[0] << " = " << retval << std::endl;
  }

  smoc_firing_state start;
public:
  m_multiply( sc_core::sc_module_name name )
    :smoc_actor( name, start ) {
    start = (in1(1) && in2(1) &&
             out1(1) && out2(1)) >>
             SMOC_CALL(m_multiply::process) >> start;
  }
};

class m_top2: public smoc_graph {
public:
  smoc_port_in<int>  in1;
  smoc_port_in<int>  in2;
  smoc_port_out<int> out;

  m_top2( sc_core::sc_module_name name )
    : smoc_graph(name)
  {
    m_adder    &adder = registerNode(new m_adder("adder"));
    m_multiply &mult  = registerNode(new m_multiply("multiply"));
    
    adder.in1(in1); // adder.in(in1);
    mult.in1(in2);  // mult.in1(in2);
    connectNodePorts(adder.out, mult.in2);
    connectNodePorts(mult.out2, adder.in2, smoc_fifo<int>() << 13);
    mult.out1(out); // mult.out(out);
  }
};

class m_source: public smoc_actor {
public:
  smoc_port_out<int> out;
private:
  size_t i;

  void process() {
    std::cout << name() << " generating " << i << std::endl;
    out[0] = i++;
  }

  smoc_firing_state start;
public:
  m_source(sc_core::sc_module_name name, size_t iter)
    :smoc_actor( name, start ), i(0) {
    start =  out(1) >> (SMOC_VAR(i) < iter) >> SMOC_CALL(m_source::process) >> start;
  }
};

class m_sink: public smoc_actor {
  public:
    smoc_port_in<int> in;
  private:
    void process() {
      std::cout << name() << " receiving " << in[0] << std::endl;
    }
    
    smoc_firing_state start;
  public:
    m_sink( sc_core::sc_module_name name )
      :smoc_actor( name, start ) {
      start = in(1) >> SMOC_CALL(m_sink::process) >> start;
    }
};

class m_top: public smoc_graph {
public:
  m_top( sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name)
  {
    m_top2        &top2 = registerNode(new m_top2("top2"));
    m_source      &src1 = registerNode(new m_source("src1", iter));
    m_source      &src2 = registerNode(new m_source("src2", iter));
    m_sink        &sink = registerNode(new m_sink("sink"));
    connectNodePorts( src1.out, top2.in1, smoc_fifo<int>(2) );
    connectNodePorts( src2.out, top2.in2, smoc_fifo<int>(2) );
    connectNodePorts( top2.out, sink.in,  smoc_fifo<int>(2) );
  }
};

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(-1);
  
  if (argc >= 2)
    iter = atol(argv[1]);
  
  smoc_top_moc<m_top> top("top", iter);
  sc_core::sc_start();
  return 0;
}
