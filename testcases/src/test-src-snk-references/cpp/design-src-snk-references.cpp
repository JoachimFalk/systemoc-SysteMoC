// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2013 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Franz-Josef Streit <franz-josef.streit@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 Matthias Schid <schid@codesign99>
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

#include "smoc_synth_std_includes.hpp"

#include <systemoc/smoc_moc.hpp>

class m_h_src: public smoc_actor {
public:
  smoc_port_out<Token<int, 1> > out;
private:
  int     i=0;
  unsigned int  iter;
  
  void src() {
    Token<int, 1> &frame = out[0]; // create output reference

    std::cout << "src: generate token with value " << i << std::endl;
    frame.Data[0] = i;
    i++;
    --iter;
  }
  smoc_firing_state start;
public:
  m_h_src(sc_core::sc_module_name name, size_t _iter)
    : smoc_actor(name, start),
      i(1), iter(_iter) {
    SMOC_REGISTER_CPARAM(_iter);
    char *init = getenv("SRC_ITERS");
    if (init)
      iter = atoll(init);
    start = out(1) >> (SMOC_VAR(iter) > 0U) >> SMOC_CALL(m_h_src::src) >> start;
  }
};

class m_h_sink: public smoc_actor {
public:
  smoc_port_in<Token<int, 1> > in;
private:
  
  void sink(void) {
    Token<int, 1> frame;
    frame = in[0];
    std::cout << "sink: received token with value " << frame.Data[0] << std::endl;
  }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_core::sc_module_name name)
    : smoc_actor(name, start) {
    start = in(1) >> SMOC_CALL(m_h_sink::sink) >> start;
  }
};

class m_h_top: public smoc_graph {
public:

  m_h_src src;
  m_h_sink snk;

  m_h_top(sc_core::sc_module_name name, unsigned int iter)
    : smoc_graph(name),
      src("src", iter),
      snk("snk") {
    connectNodePorts(src.out,snk.in);

  }
};

int sc_main (int argc, char **argv) {
  unsigned int iter = 1; //
  if (argc >= 2)
    iter = atoi(argv[1]);
  std::cout << "Program starts !!!" << std::endl;
  
  smoc_top_moc<m_h_top> top("top", iter);
  sc_core::sc_start();
  std::cout << "Program ends !!!" << std::endl;

  return 0;
}
