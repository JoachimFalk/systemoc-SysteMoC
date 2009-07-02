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

static const char message [] = "Hello SysteMoC!";

class Source: public smoc_actor {
public:
  // ports:
  smoc_port_out<char> out;
private:
  // functionality state:
  unsigned int count;
  unsigned int size;

  // guards:
  bool hasToken() const{
    return count<size;
  }

  // actions:
  void src() {
    out[0] = message[count];
    ++count;
  }

  // FSM states:
  smoc_firing_state start;
public:
  // actor constructor
  Source(sc_module_name name)
    : smoc_actor(name, start),
      count(0),
      size(sizeof(message))
  {

    // FSM definition:
    start = 
      GUARD(Source::hasToken)  >>
      out(1)                   >>
      CALL(Source::src) >> start
      ;
  }
};


class Sink: public smoc_actor {
public:
  // ports:
  smoc_port_in<char> in;
private:
  // actions:
  void sink() {
    std::cout << in[0] << std::endl;
  }

  // FSM states:
  smoc_firing_state start;
public:
  // actor constructor
  Sink(sc_module_name name)
    : smoc_actor(name, start)
  {

    // FSM definition:
    start =
      in(1)                 >>
      CALL(Sink::sink) >> start
      ;
  }
};



class NetworkGraph: public smoc_graph {
protected:
  // actors
  Source     src;
  Sink       sink;
public:
  // networkgraph constructer
  NetworkGraph(sc_module_name name)
    : smoc_graph(name),
      src("Source"), // create actors
      sink("Sink")
  {
    connectNodePorts(src.out, sink.in);
  }
};

int sc_main (int argc, char **argv) {
  
  // create networkgraph
  smoc_top_moc<NetworkGraph> top("top");

  // start simulation (SystemC)
  sc_start();
  return 0;
}
