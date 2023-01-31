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

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph.hpp>

#include <systemc.h>
#include <tlm.h>

#include <iostream>

class Src : public smoc_actor {
public:
  smoc_port_out<void> out;

  Src(sc_core::sc_module_name name, size_t iter)
    : smoc_actor(name, run1), iter(iter)
  {
    run1 =
        (out(1) && (SMOC_VAR(this->iter) >= 2U)) >>
        SMOC_CALL(Src::src1)                     >>
        SMOC_CALL(Src::src1)                     >> run2
     ;
    run2 =
        (out(1) && (SMOC_VAR(this->iter) >= 2U)) >>
        SMOC_CALL(Src::src1)                     >>
        SMOC_CALL(Src::src2)(0)                  >> run3
     ;
    run3 =
        (out(1) && (SMOC_VAR(this->iter) >= 2U)) >>
        SMOC_CALL(Src::src2)(1)                  >>
        SMOC_CALL(Src::src1)                     >> run4
     ;
    run4 =
        (out(1) && (SMOC_VAR(this->iter) >= 2U)) >>
        SMOC_CALL(Src::src2)(2)                  >>
        SMOC_CALL(Src::src2)(3)                  >> run5
      ;
    run5 =
        (out(1) && (SMOC_VAR(this->iter) >= 3U)) >>
        SMOC_CALL(Src::src2)(4)                  >>
        SMOC_CALL(Src::src1)                     >>
        SMOC_CALL(Src::src2)(5)                  >> run1
      ;
  }

  void src1() {
    std::cout << "Src::src1() " << iter << std::endl;
    --iter;
  }

  void src2(long long x) {
    std::cout << "Src::src2() " << iter << ", " << x << std::endl;
    --iter;
  }

private:
  size_t iter;

  smoc_firing_state run1, run2, run3, run4, run5;
};

class Snk : public smoc_actor {
public:
  smoc_port_in<void> in;

  Snk(sc_core::sc_module_name name) :
    smoc_actor(name, run)
  {
    run = in(1) >> SMOC_CALL(Snk::snk) >> run;
  }

  void snk() {
    std::cout << "Snk::snk()" << std::endl;
  }

private:
  smoc_firing_state run;
};

class Graph : public smoc_graph {
public:
  Graph(sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name),
      src("src", iter),
      snk("snk")
  {
    (smoc_fifo<void>(1) << 1).connect(src.out).connect(snk.in);
  }
private:
  Src src;
  Snk snk;
};

int sc_main (int argc, char **argv) { 
  size_t iter = static_cast<size_t>(-1);
  
  if (argc >= 2)
    iter = atol(argv[1]);

  Graph top("top", iter);
  smoc_scheduler_top s(top);
  sc_core::sc_start();
  return 0;
}
