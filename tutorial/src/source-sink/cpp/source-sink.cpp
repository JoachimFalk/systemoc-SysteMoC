// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA.
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
