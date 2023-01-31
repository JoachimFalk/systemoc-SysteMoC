// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2013 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include <iostream>

#include <systemoc/smoc_moc.hpp>

template <typename T>
class m_h_src: public smoc_actor {
public:
  smoc_port_in<void>  inActive;
  smoc_port_out<void> outActive;
  smoc_port_out<T>    out;
private:
  T       i;
  size_t  iter;
  
  void src() {
    std::cout << name() << ": generate token with value " << i << std::endl;
    out[0] = i++; --iter;
  }
  smoc_firing_state start;
public:
  m_h_src(sc_core::sc_module_name name, size_t _iter)
    : smoc_actor(name, start),
      i(1), iter(_iter) {
    start =
         (inActive(1)  && SMOC_VAR(iter) > 0U &&
          outActive(1) && out(1)) >>
         SMOC_CALL(m_h_src::src) >> start;
      ;
  }
};


template <typename T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  void sink(void) {
    std::cout << name() << ": received token with value " << in[0] << std::endl;
  }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_core::sc_module_name name)
    : smoc_actor(name, start) {
    start =
        in(1) >>
        SMOC_CALL(m_h_sink::sink) >> start
      ;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_src<int>    src1;
  m_h_src<int>    src2;
  m_h_src<int>    src3;
  m_h_sink<int>   snk;
public:
  m_h_top(sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name),
      src1("src1",iter),
      src2("src2",iter),
      src3("src3",iter),
      snk("snk")
  {
    smoc_multireader_fifo<int> f;

    f.connect(src1.out).connect(src2.out).connect(src3.out).connect(snk.in);

    connectNodePorts(src1.outActive, src2.inActive);
    connectNodePorts(src2.outActive, src3.inActive);
    connectNodePorts(src3.outActive, src1.inActive, smoc_fifo<void>() << 1);
  }
};

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(-1);
  
  if (argc >= 2)
    iter = atol(argv[1]);
  
  smoc_top_moc<m_h_top> top("top", iter);
  sc_core::sc_start();
  return 0;
}
