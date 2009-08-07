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

static const std::string MESSAGE_HELLO = "Hello SysteMoC!";
static const std::string MESSAGE_ABC   = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const std::string MESSAGE_123   = "0123456789";

class Source: public smoc_actor {
public:
  smoc_port_out<char> out;
  Source(sc_module_name name, const std::string msg) : smoc_actor(name, start),
    count(0), size(MESSAGE_123.size()), message(msg) {
    start = 
      GUARD(Source::hasToken)  >>
      //(VAR(count)<VAR(size))   >>
      out(1)                   >>
      CALL(Source::src)        >> start;
  }
private:
  smoc_firing_state start;

  unsigned int count, size;  // variables (functional state)
  const std::string message; //

  bool hasToken() const{ return count<size; } // guard
  void src() { out[0] = message[count++]; }   // action
};



class Sink: public smoc_actor {
public:
  // ports:
  smoc_port_in<char> in;

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

private:
  // actions:
  void sink() {
    std::cout << this->name() << " recv: \""
              << in[0] << "\"" << std::endl;
  }

  // FSM states:
  smoc_firing_state start;
};



class NetworkGraph: public smoc_graph {
public:
  // networkgraph constructor
  NetworkGraph(sc_module_name name)
    : smoc_graph(name),
      // create actors
      source("Source", MESSAGE_123),
      sink("Sink")
  {
    connectNodePorts(source.out, sink.in);
  }
private:
  // actors
  Source         source;
  Sink           sink;
};

int sc_main (int argc, char **argv) {

  // create networkgraph
  smoc_top_moc<NetworkGraph> top("top");

  // start simulation (SystemC)
  sc_start();
  return 0;
}
