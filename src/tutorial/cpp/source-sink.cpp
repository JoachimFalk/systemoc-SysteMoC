// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of
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

class Sink: public smoc_actor {
public:
  // ports:
  smoc_port_in<char> in;

  Sink(sc_core::sc_module_name name)   // actor constructor
    : smoc_actor(name, start) {
    // FSM definition:
    start =
      in(1)                 >>
      CALL(Sink::sink)      >> start;
  }
private:
  smoc_firing_state start;  // FSM states

  void sink() {
    std::cout << this->name() << " recv: \'"
              << in[0] << "\'" << std::endl;
  }
};

class Source: public smoc_actor {
public:
  // ports:
  smoc_port_out<char> out;

  Source(sc_core::sc_module_name name)
    : smoc_actor(name, start) {
    start = 
      out(1)                   >>
      CALL(Source::src)        >> start;
  }
private:
  smoc_firing_state start;  // FSM states

  void src() {
    std::cout << this->name() << " send: \'X\'" << std::endl;
    out[0] = 'X';
  }
};




class NetworkGraph: public smoc_graph {
public:
  NetworkGraph(sc_core::sc_module_name name)  // networkgraph constructor
    : smoc_graph(name),
      source("Source"),             // create actors
      sink("Sink") {
    connectNodePorts(source.out, sink.in); // connect actors
  }
private:
  Source         source;   // actors
  Sink           sink;
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<NetworkGraph> top("top"); // create networkgraph

  sc_core::sc_start();   // start simulation (SystemC)
  return 0;
}
