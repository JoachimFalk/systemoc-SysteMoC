// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 Joachim Falk <joachim.falk@gmx.de>
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

class m_source: public smoc_actor {
public:
  smoc_port_out<int> out;
private:
  size_t i;

  void process() {
    std::cout << name() << ": generating " << i << std::endl;
    out[0] = i++;
  }

  smoc_firing_state start;
public:
  m_source( sc_core::sc_module_name name, size_t iter)
    : smoc_actor( name, start ), i(0) {
    start =
        (out(1) && SMOC_VAR(i) < iter) >>
        SMOC_CALL(m_source::process) >> start
      ;
  }
};

class m_sink: public smoc_actor {
public:
  smoc_port_in<int> in;
private:
  void process() {
    std::cout << name() << ": receiving " << in[0] << std::endl;
  }

  smoc_firing_state start;
public:
  m_sink( sc_core::sc_module_name name )
    :smoc_actor( name, start ) {
    start =
        (in(1) && in.getValueAt(0) == 0) >>
        SMOC_CALL(m_sink::process) >> start
      |
        in(1) >>
        SMOC_CALL(m_sink::process) >> start
      ;
  }
};

class m_top: public smoc_graph {
public:
  m_top( sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name)
  {
    m_source &src1 = registerNode(new m_source("src1", iter));
    m_sink   &sink = registerNode(new m_sink("sink"));
    connectNodePorts(src1.out, sink.in, smoc_fifo<int>(2));
  }
};

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(1024);
  
  if (argc >= 2)
    iter = atol(argv[1]);

  smoc_top_moc<m_top> top("top", iter);
  sc_core::sc_start();
  return 0;
}
