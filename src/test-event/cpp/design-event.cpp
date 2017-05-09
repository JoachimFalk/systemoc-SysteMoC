//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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
#include <systemoc/smoc_event.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_graph.hpp>
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_expr.hpp>

#include <cmath>
#include <cassert>
#include <cstdlib> // for atoi

using namespace std; 

class Src: public smoc_actor {
  SC_HAS_PROCESS(Src);
public:
  smoc_port_out<double> out;
private:
  size_t i;

  void src() {
    std::cout << "src@" << sc_core::sc_time_stamp() << ": " << i <<  std::endl;
    out[0] = i++;
    ev.reset();
    e.notify(10, sc_core::SC_NS);
  }

  smoc_firing_state start;

  smoc_event        ev;
  sc_core::sc_event e;
public:
  Src(sc_core::sc_module_name name)
    : smoc_actor(name, start), i(0)
  {
    start =
         TILL(ev)
      >> out(1)                       
      >> CALL(Src::src)
      >> start
    ;
    SC_METHOD(notifier);
    sensitive << e;
  }

  void notifier() {
    ev.notify();
  }
};

class Sink: public smoc_actor {
public:
  smoc_port_in<double> in1;
  smoc_port_in<double> in2;
  smoc_port_in<double> in3;
private:
  size_t  iter;

  void sink() {
    std::cout << "sink@" << sc_core::sc_time_stamp() << " " << iter << ": " << in1[0] << ", " << in2[0] << ", " << in3[0] <<  std::endl;
    --iter;
  }
  
  smoc_firing_state start, end;
public:
  Sink(sc_core::sc_module_name name, size_t iter)
    : smoc_actor(name, start), iter(iter)
  {
    start =
         (in1(1) && in2(1) && in3(1) && (SMOC_VAR(this->iter) > 0U)) >>
         CALL(Sink::sink) >> start
      |
         (SMOC_VAR(this->iter) == 0U) >> end
      ;
  }
};

class Graph: public smoc_graph {
public:
protected:
  Src      src1, src2, src3;
  Sink     snk;
public:
  Graph(sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name),
      src1("src1"), src2("src2"), src3("src3"), snk("snk", iter)
  {
    // Connect with fifo of size 3 and one initial token '-13'
    connectNodePorts(src1.out, snk.in1, smoc_fifo<double>(3) << -13); 
    // Connect with fifo of size 2
    connectNodePorts(src2.out, snk.in2, smoc_fifo<double>(2)); 
    // Connect with fifo of size 2
    connectNodePorts(src3.out, snk.in3, smoc_fifo<double>(2)); 
  }
};

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(-1);
  
  if (argc >= 2)
    iter = atol(argv[1]);
  
  smoc_top_moc<Graph> graph("graph", iter);
  sc_core::sc_start();
  return 0;
}
