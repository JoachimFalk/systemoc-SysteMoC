// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <iostream>

#include <systemoc/smoc_moc.hpp>

static const std::string MESSAGE_HELLO = "Hello SysteMoC!";

class Source : public smoc_actor
{
public:
  smoc_port_out<char> out;

  Source(sc_core::sc_module_name name) :
    smoc_actor(name, start), count(0), size(MESSAGE_HELLO.size()), message(
        MESSAGE_HELLO)
  {
    start = GUARD(Source::hasToken) >> out(1) >> CALL(Source::src) >> start;
  }

private:
  smoc_firing_state start;

  unsigned int count, size; // variables (functional state)
  const std::string message; //

  bool
  hasToken() const
  {
    return count < size;
  } // guard
  void
  src()
  {
    std::cerr << this->name() << "> @ " << sc_core::sc_time_stamp() << "\tsend: \'"
        << message[count] << "\'" << std::endl;
    out[0] = message[count++];
  } // action
};

class Sink : public smoc_actor
{
public:
  // ports:
  smoc_port_in<char> in;

  Sink(sc_core::sc_module_name name) // actor constructor
  :
    smoc_actor(name, start)
  {
    // FSM definition:
    start = in(1) >> CALL(Sink::sink) >> start;
  }
private:
  smoc_firing_state start; // FSM states

  void
  sink()
  {
    std::cout << this->name() << "> @ " << sc_core::sc_time_stamp() << "\trecv: \'"
        << in[0] << "\'" << std::endl;
  }
};

class NetworkGraph : public smoc_graph
{
public:
  NetworkGraph(sc_core::sc_module_name name) // network graph constructor
  :
    smoc_graph(name), source("Source"), // create actors
        sink("Sink")
  {
    smoc_fifo<char> fifo("queue", 4);
    fifo.connect(source.out).connect(sink.in); // connect actors
  }
private:
  Source source; // actors
  Sink sink;
};

int
sc_main(int argc, char **argv)
{
  NetworkGraph top("top"); // create network graph
  smoc_scheduler_top sched(top);

  sc_core::sc_start(); // start simulation (SystemC)
  return 0;
}
