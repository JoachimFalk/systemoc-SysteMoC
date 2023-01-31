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

static const char MESSAGE_HELLO [] = "Hello SysteMoC!";
static const char MESSAGE_ABC []   = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char MESSAGE_123 []   = "0123456789";

class Source: public smoc_actor {
public:
  smoc_port_out<char> out;
  Source(sc_core::sc_module_name name, const char* msg) : smoc_actor(name, start),
    count(0), size(strlen(msg)), message(msg) {
    start = 
      GUARD(Source::hasToken)  >>
      out(2)                   >>
      CALL(Source::src)        >> start;
  }
private:
  smoc_firing_state start;

  unsigned int count, size;  // variables (functional state)
  const char* message;       //

  bool hasToken() const{ return count<size; } // guard
  void src() {                               // action
    out[0] = message[count++];
    out[1] = message[count++];
  }};




class Sink: public smoc_actor {
public:
  // ports:
  smoc_port_in<char> in;
private:
  // actions:
  void sink() {
    std::cout << this->name() << " recv: \""
              << in[0] << "\"" << std::endl;
  }

  // FSM states:
  smoc_firing_state start;
public:
  // actor constructor
  Sink(sc_core::sc_module_name name)
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
  Source         source;
  Sink           sink;
public:
  // networkgraph constructor
  NetworkGraph(sc_core::sc_module_name name)
    : smoc_graph(name),
      // create actors
      source("Source", MESSAGE_123),
      sink("Sink")
  {
    //connectNodePorts(source.out, sink.in);
    connectNodePorts<4>(source.out, sink.in);
  }
};

int sc_main (int argc, char **argv) {

  // create networkgraph
  smoc_top_moc<NetworkGraph> top("top");

  // start simulation (SystemC)
  sc_core::sc_start();
  return 0;
}
