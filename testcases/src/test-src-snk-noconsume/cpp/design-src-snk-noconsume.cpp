// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
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
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph.hpp>

template <typename T>
class m_h_src: public smoc_actor {
public:
  smoc_port_out<T> out;
private:
  T i;
  
  void src() {
    std::cout << name() << ": " << i << std::endl;
    out[0] = i++;
  }
  smoc_firing_state start;
public:
  m_h_src(sc_core::sc_module_name name, const T &iters)
    : smoc_actor(name, start),
      i(1) {
    start = (out(1) && SMOC_VAR(i) <= iters) >> SMOC_CALL(m_h_src::src) >> start;
  }
};


template <typename T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  int i;
  
  void sink(void) { std::cout << name() << ": " << in[0] << std::endl; }
  
  smoc_firing_state s0, s1;
public:
  m_h_sink(sc_core::sc_module_name name)
    : smoc_actor(name, s0) {
    s0 = in(0,1) >> SMOC_CALL(m_h_sink::sink) >> s1;
    s1 = in(1)   >> SMOC_CALL(m_h_sink::sink) >> s0;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_src<int>     src;
  m_h_sink<int>    snk;
public:
  m_h_top(sc_core::sc_module_name name, int iters)
    : smoc_graph(name),
      src("src", iters), snk("snk") {
    connectNodePorts(src.out, snk.in);
  }
};

int sc_main (int argc, char **argv) {
  int iters = 100;
  if (argc == 2) {
    iters = atoi(argv[1]);
  }
  smoc_top_moc<m_h_top> top("top", iters);
  sc_core::sc_start();
  return 0;
}
