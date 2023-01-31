// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
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
  m_h_src(sc_core::sc_module_name name)
    : smoc_actor(name, start),
      i(1) {
    start = out(1) >> SMOC_CALL(m_h_src::src) >> start;
  }
};

class m_h_srcbool: public smoc_actor {
public:
  smoc_port_out<int> out;
private:
  bool i;
  
  void src() {
    std::cout << name() << ": " << i << std::endl;
    out[0] = i ? 1 : 0; i = !i;
  }
  smoc_firing_state start;
public:
  m_h_srcbool(sc_core::sc_module_name name)
    : smoc_actor(name, start),
      i(false) {
    start = out(1) >> SMOC_CALL(m_h_srcbool::src) >> start;
  }
};

template <typename T>
class Select: public smoc_actor {
public:
  smoc_port_in<int>  Control;
  smoc_port_in<T>    Data0, Data1;
  smoc_port_out<T>   Output;
private:
  void action0() { 
    std::cout << name() << ": action0" << std::endl;
    Output[0] = Data0[0];
  }
  void action1() {
    std::cout << name() << ": action1" << std::endl;
    Output[0] = Data1[0];
  }
  smoc_firing_state state;
public:
  Select(sc_core::sc_module_name name)
    : smoc_actor(name, state) {
    state
      = (Control(1) && Data0(1) &&
         Control.getValueAt(0) == 0)  >>
        Output(1)                     >>
        SMOC_CALL(Select::action0)    >> state
      | (Control(1) && Data1(1) &&
         Control.getValueAt(0) == 1)  >>
        Output(1)                     >>
        SMOC_CALL(Select::action1)    >> state
      ;
  }
};

template <typename T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  size_t iter;
  
  void sink(void) {
    std::cout << name() << ": " << in[0] << std::endl;
    --iter;
  }
  
  smoc_firing_state start, end;
public:
  m_h_sink(sc_core::sc_module_name name, size_t iter)
    : smoc_actor(name, start), iter(iter) {
    start =
         (in(1) && (SMOC_VAR(this->iter) > 0U)) >>
         SMOC_CALL(m_h_sink::sink) >> start
      |
         (SMOC_VAR(this->iter) == 0U) >> end
      ;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_srcbool         srcbool;
  m_h_src<double>     src1, src2;
  Select<double>      select;
  m_h_sink<double>    sink;
public:
  m_h_top(sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name),
      srcbool("srcbool"),
      src1("src1"), src2("src2"),
      select("select"),
      sink("sink", iter) {
    connectNodePorts(srcbool.out, select.Control);
    connectNodePorts(src1.out, select.Data0);
    connectNodePorts(src2.out, select.Data1);
    connectNodePorts(select.Output, sink.in);
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
