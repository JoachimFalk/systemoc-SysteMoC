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

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

class Forward: public smoc_actor {
public:
  smoc_port_in<int> in;
  smoc_port_out<int> out;

  Forward(sc_core::sc_module_name name) : smoc_actor(name, start){
    start = 
      in(1)                    >>
      out(1)                   >>
      CALL(Forward::forward)   >> start;
  }
private:
  smoc_firing_state start;

  void forward() {
    std::cout << this->name() << " forward: \""
              << in[0] << "\"" << std::endl;
    out[0] = in[0];
  }
};


class NetworkGraph: public smoc_graph {
protected:
  // actors
  Forward         ping;
  Forward         pong;
public:
  // networkgraph constructor
  NetworkGraph(sc_core::sc_module_name name)
    : smoc_graph(name),
      // create actors
      ping("Ping"),
      pong("Pong")
  {
    smoc_fifo<int> initFifo(1);
    initFifo << 42;
    connectNodePorts(ping.out, pong.in);
    connectNodePorts(pong.out, ping.in, initFifo);
  }
};

int sc_main (int argc, char **argv) {

  // create networkgraph
  smoc_top_moc<NetworkGraph> top("top");

  // start simulation (SystemC)
  sc_core::sc_start();
  return 0;
}
