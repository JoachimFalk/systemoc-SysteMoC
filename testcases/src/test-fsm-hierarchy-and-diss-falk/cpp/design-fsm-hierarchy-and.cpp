// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include <iostream>
#include <cstdlib>

#include <systemoc/smoc_moc.hpp>

#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>

class Testbench: public smoc_actor {
  SC_HAS_PROCESS(Testbench);
public:
  smoc_port_out<void> o1, o2;
  smoc_port_in<void>  i1, i2;
private:
  smoc_firing_state bf, be, bg, cf, ce, cg;

  smoc_event        smocEvent;
  sc_core::sc_event scEvent;

  // This produces randomness out of thin air, i.e., it is a pseudo-random number generator.
  boost::random::mt19937                    rng;
  boost::random::uniform_int_distribution<> coin;
  size_t                                    iter;
  int                                       random;

public:
  Testbench(sc_core::sc_module_name name, size_t _iter)
    : smoc_actor(name, bf)
    , o1("o1"), o2("o2"), i1("i1"), i2("i2")
    , bf("bf"), be("be"), bg("bg"), cf("cf"), ce("ce"), cg("cg")
    , rng(4711), coin(1,2), iter(_iter), random(coin(rng))
  {
    SC_METHOD(notifyEvent);
    sensitive << scEvent;

    bf = (SMOC_TILL(smocEvent) && SMOC_GUARD(Testbench::caseA)    && o2(1)) >>
         SMOC_CALL(Testbench::print)("bf->be") >> be
       | (SMOC_TILL(smocEvent) && SMOC_GUARD(Testbench::caseB)    && o1(1) && i1(2)) >>
         SMOC_CALL(Testbench::print)("bf->cf") >> cf
       ;
    be = (SMOC_TILL(smocEvent) && SMOC_GUARD(Testbench::caseA)    && i2(1)) >>
         SMOC_CALL(Testbench::print)("be->bg") >> bg
       | (SMOC_TILL(smocEvent) && SMOC_GUARD(Testbench::caseB)    && o1(1) && i1(2)) >>
         SMOC_CALL(Testbench::print)("be->ce") >> ce
       ;
    bg = (SMOC_TILL(smocEvent) && SMOC_GUARD(Testbench::caseA)    && o2(3)) >>
         SMOC_CALL(Testbench::print)("bg->bf") >> bf
       | (SMOC_TILL(smocEvent) && SMOC_GUARD(Testbench::caseB)    && o1(1) && i1(2)) >>
         SMOC_CALL(Testbench::print)("bg->cg") >> cg
       ;
    cf = (SMOC_TILL(smocEvent) && SMOC_GUARD(Testbench::caseBoth) && o2(1)) >>
         SMOC_CALL(Testbench::print)("cf->ce") >> ce
       ;
    ce = (SMOC_TILL(smocEvent) && SMOC_GUARD(Testbench::caseBoth) && i2(1)) >>
         SMOC_CALL(Testbench::print)("ce->cg") >> cg
       ;
    cg = (SMOC_TILL(smocEvent) && SMOC_GUARD(Testbench::caseA)    && o2(3)) >>
         SMOC_CALL(Testbench::print)("cg->cf") >> cf
       | (SMOC_TILL(smocEvent) && SMOC_GUARD(Testbench::caseB)    && i2(2)) >>
         SMOC_CALL(Testbench::print)("cg->bg") >> bg
       ;
  }

private:
  bool caseA()    const { return random == 1; }
  bool caseB()    const { return random == 2; }
  bool caseBoth() const { return random >= 1; }

  void print(const char *tname) {
    smocEvent.reset();
    random = coin(rng);
    if (--iter == 0)
      random = 0;
    else
      scEvent.notify(10, sc_core::SC_NS);
    std::cout << name() << ": transition " << tname << " (random: " << random << ")" << std::endl;
  }

  void notifyEvent()
    { smocEvent.notify(); }

};

class Transform : public smoc_actor {
public:
  smoc_port_in<void>   i1, i2;
  smoc_port_out<void>  o1, o2;
protected:
  smoc_and_state    y;
  smoc_xor_state    a, d;
  smoc_firing_state b, c, e, f, g;
public:
  Transform(sc_core::sc_module_name name)
    : smoc_actor(name, y),
      i1("i1"), i2("i2"), o1("o1"), o2("o2"),
      y("y"), a("a"), d("d"), b("b"), c("c"), e("e"), f("f"), g("g")
  {
    y.add(a).add(d);
    a.init(b).add(c);
    d.add(e).init(f).add(g);

    b =          i1(1) >> o1(2) >> SMOC_CALL(Transform::print)("b->c") >> c;
    (c, IN(g)) =          o2(2) >> SMOC_CALL(Transform::print)("c->b") >> b;

    e          =          o2(1) >> SMOC_CALL(Transform::print)("e->g") >> g;
    f          = i2(1)          >> SMOC_CALL(Transform::print)("f->e") >> e
               |          o2(1) >> SMOC_CALL(Transform::print)("f->g") >> g;
    g          = i2(3)          >> SMOC_CALL(Transform::print)("g->f") >> f;
  }
private:
  void print(const char *tname)
    { std::cout << name() << ": transition " << tname << std::endl; }
};

class Top : public smoc_graph {
public:
  Top(sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name),
      tb("tb", iter),
      trans("transform")
  {
    connectNodePorts(tb.o1, trans.i1, smoc_fifo<void>(1));
    connectNodePorts(tb.o2, trans.i2, smoc_fifo<void>(3));
    connectNodePorts(trans.o1, tb.i1, smoc_fifo<void>(2) << 2);
    connectNodePorts(trans.o2, tb.i2, smoc_fifo<void>(2) << 2);
  }

private:
  Testbench tb;
  Transform trans;
};

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(-1);

  if (argc >= 2)
    iter = std::atol(argv[1]);

  smoc_top_moc<Top> top("top", iter);

  sc_core::sc_start();
  return 0;
}
