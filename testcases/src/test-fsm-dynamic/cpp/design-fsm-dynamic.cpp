// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2013 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2014 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
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

class m_h_src: public smoc_actor {
public:
  smoc_port_out<int> out;
private:
  int i;
  int iter;
  
  void src(int j) {
    std::cout
      << name() << ": generate " << j << " token with value " << i << std::endl;
    for (int x = 0; x < j; ++x) {
      out[x] = i++;
    }
    iter -= j;
  }
  smoc_firing_state start;
public:
  m_h_src(sc_core::sc_module_name name, int iter)
    : smoc_actor(name, start),
      out("out"),
      i(1), iter(iter),
      start("start")
  {
    smoc_firing_state::Ptr oldState = &start;

    int mumStates = 3;
    for (int j = 1; j <= mumStates; ++j) {
      smoc_firing_state::Ptr newState = j < mumStates
        ? &smoc_firing_state("state"+CoSupport::String::asStr(j))
        : &start;
      *oldState = (out(j) && SMOC_VAR(this->iter) > 0)
        >> SMOC_CALL(m_h_src::src)(j) >> *newState;
      oldState = newState;
    }
  }
};


class m_h_sink: public smoc_actor {
public:
  smoc_port_in<int> in;
private:
  int i;
  
  void sink(void) {
    std::cout
      << name() << ": received token with value " << in[0] << std::endl;
  }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_core::sc_module_name name)
    : smoc_actor(name, start) {
    start = in(1) >> SMOC_CALL(m_h_sink::sink) >> start;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_src     src;
  m_h_sink    snk;
public:
  m_h_top(sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name),
      src("src", iter), snk("snk") {
    connectNodePorts<16>(src.out, snk.in);
  }
};

int sc_main (int argc, char **argv) {
  int iter = std::numeric_limits<int>::max();
  
  if (argc >= 2)
    iter = atol(argv[1]);
  
  smoc_top_moc<m_h_top> top("top", iter);
  sc_core::sc_start();
  return 0;
}
