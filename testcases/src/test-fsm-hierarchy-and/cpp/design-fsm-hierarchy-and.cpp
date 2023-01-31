// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
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
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph.hpp>

template<class T>
class Src : public smoc_actor {
public:
  smoc_port_out<T> out;

  Src(sc_core::sc_module_name name, size_t _iter)
    : smoc_actor(name, start),
      start("start"),
      iter(_iter), i(0)
  {
    start =
         (out(1) && SMOC_VAR(iter) > 0U)
      >> SMOC_CALL(Src::src)
      >> start;
  }

private:
  smoc_firing_state start;
  size_t iter;
  T i;

  void src() {
    std::cout << name() << ": generate token with value "
              << i << std::endl;
    out[0] = i; ++i; --iter;
  }
};

template<class T>
class Snk : public smoc_actor {
public:
  smoc_port_in<T> in;

  Snk(sc_core::sc_module_name name)
    : smoc_actor(name, start),
      start("start")
  {
    start =
         in(1)
      >> SMOC_CALL(Snk::snk)
      >> start;
  }

private:
  smoc_firing_state start;
  
  void snk() {
    std::cout << name() << ": received token with value "
              << in[0] << std::endl;
  }
};

template<class T>
class Transform : public smoc_actor {
public:
  smoc_port_in<T>  i1;
  smoc_port_out<T> o1;

  Transform(sc_core::sc_module_name name)
    : smoc_actor(name, init)
    , i1("i1")
    , o1("o1")
    , init("init")
  {
    // All states except the initial state can be
    // temporary states (-> won't clutter the class)
    
    // AND state with two partitions (each of which is
    // an XOR state)
    smoc_and_state a("a");

    // leaf states can have names (currently only for
    // dumping purposes -> --dump-fsm)
    smoc_firing_state b("b");
    smoc_firing_state c("c");
    smoc_firing_state d("d");
    smoc_firing_state e("e");

    // Each XOR state must have a single initial state
    a.add(smoc_xor_state("a0").init(b).add(c));
    a.add(smoc_xor_state("a1").init(d).add(e));
    
    smoc_and_state f("f");
    smoc_firing_state g("g");
    smoc_firing_state h("h");
    smoc_firing_state i("i");
    smoc_firing_state j("j");

    f.add(smoc_xor_state("f0").init(g).add(h));
    f.add(smoc_xor_state("f1").init(i).add(j));
    
    smoc_and_state k("k");
    smoc_firing_state l("l");
    smoc_firing_state m("m");

    smoc_xor_state k0("k0"); k0.init(a).add(l);
    smoc_xor_state k1("k1"); k1.init(f).add(m);
    k.add(k0).add(k1);

    init =
      // Specifying AND state as target state
         i1(1)
      >> SMOC_CALL(Transform::print)("init -> k")
      >> SMOC_CALL(Transform::store)
      >> k
      // Specifying different initial states per partition
    |    i1(1)
      >> SMOC_CALL(Transform::print)("init -> (l,j)")
      >> SMOC_CALL(Transform::store)
      >> (l,j);

    (k, IN(i)) =
      // leaving AND states leaves each partition
         o1(1)
      >> SMOC_CALL(Transform::print)("(k,IN(i)) -> init")
      >> SMOC_CALL(Transform::write)
      >> init;

    // conditionally leaving and entering same AND state
    (k, IN(l)) =
      SMOC_CALL(Transform::print)("(k,IN(l)) -> (k,m)") >> (k,m);

    // transition in sub-AND-state
    b = SMOC_CALL(Transform::print)("b -> c") >> c;

    // can also check states in other hierarchy layer
    (b, IN(m)) = SMOC_CALL(Transform::print)("b -> b") >> b;

    // AND partitions can be accessed -> XOR state
    (k0, !IN(c), !IN(e))
      = SMOC_CALL(Transform::print)("(k[0], !IN(c), !IN(e)) -> (c,e)") >> (c,e);
  }     

private:
  smoc_firing_state init;
  T t;

  void print(const char* tname) {
    std::cout << name() << ": transition " << tname
              << std::endl;
  }

  void store() {
    std::cout << name() << ": store token with value "
              << i1[0] << std::endl;
    t = i1[0];
  }
  
  void write() {
    std::cout << name() << ": write token with value "
              << t << std::endl;
    o1[0] = t;
  }
};

class Top : public smoc_graph {
public:
  Top(sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name),
      src("src", iter),
      snk("snk"),
      trans("transform")
  {
    connectNodePorts(src.out, trans.i1);
    connectNodePorts(trans.o1, snk.in);
  }

private:
  Src<int> src;
  Snk<int> snk;
  Transform<int> trans;
};

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(-1);

  if (argc >= 2)
    iter = std::atol(argv[1]);

  smoc_top_moc<Top> top("top", iter);

  sc_core::sc_start();
  return 0;
}
