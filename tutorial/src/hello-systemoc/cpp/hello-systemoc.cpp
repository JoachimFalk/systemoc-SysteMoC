// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
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

#include <iostream>

#include <systemoc/smoc_moc.hpp>

class HelloActor: public smoc_actor {

private:
  // actions:
  void src() {
    std::cout << "Actor " << this->name() << " says:\n"
              << "Hello World" << std::endl;
  }

  // FSM states:
  smoc_firing_state state_a, state_b;
public:
  // actor constructor
  HelloActor(sc_core::sc_module_name name)
    : smoc_actor(name, state_a)
  {

    // FSM definition:
    //  transition from state_a to state_b calling action src
    state_a = CALL(HelloActor::src) >> state_b
      ;
  }
};


class HelloNetworkGraph: public smoc_graph {
private:
  // actors
  HelloActor     helloActor;
public:
  // networkgraph constructor
  HelloNetworkGraph(sc_core::sc_module_name name)
    : smoc_graph(name),
      helloActor("HelloActor") // create actor HelloActor
  {
  }
};

int sc_main (int argc, char **argv) {
  
  // create networkgraph
  HelloNetworkGraph top("top");
  smoc_scheduler_top sched(top);

  // start simulation (SystemC)
  sc_core::sc_start();
  return 0;
}
