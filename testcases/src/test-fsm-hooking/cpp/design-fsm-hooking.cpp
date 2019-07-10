// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2018-2018 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 * 
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
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
    : smoc_actor(name, start)
    , start("start"), iter(_iter),  i(0)
  {
    start =
         (out(1) && SMOC_VAR(iter) > 0U)
      >> SMOC_CALL(Src::src)
      >> start;
  }

public:
  smoc_firing_state start;
  size_t iter;
  T i;

  void src() {
    --i;
    std::cout << "src()";
    out[0] = i;
    if (i < 0)
      i = iter/10;
    --iter;
  }
};

void srcPre(smoc_actor *actor, const std::string &src, const std::string &action, const std::string &dst) {
  std::cout << actor->name() << "[" << action << "]: i == " << static_cast<Src<int> *>(actor)->i << "; " << src << " -- ";
}

void srcPost(smoc_actor *actor, const std::string &src, const std::string &action, const std::string &dst) {
  std::cout << " generate token with value " << static_cast<Src<int> *>(actor)->out[0] << " --> " << dst << std::endl;
}

template<class T>
class Snk : public smoc_actor {
public:
  smoc_port_in<T> in;

  Snk(sc_core::sc_module_name name)
    : smoc_actor(name, start)
    , start("start")
  {
    start =
         in(1)
      >> SMOC_CALL(Snk::snk)
      >> start;
  }

private:
  smoc_firing_state start;
  
  void snk() {
    std::cout << "received token with value " << in[0];
  }
};

void snkPre(smoc_actor *actor, const std::string &src, const std::string &action, const std::string &dst) {
  std::cout << actor->name() << "[" << action << "]: in[0] == " << static_cast<Snk<int> *>(actor)->in[0] << "; " << src << " -- ";
}

void snkPost(smoc_actor *actor, const std::string &src, const std::string &action, const std::string &dst) {
  std::cout << " --> " << dst << std::endl;
}

template<class T>
class Transform : public smoc_actor {
public:
  smoc_port_in<T>  i1, i2;
  smoc_port_out<T> o1, o2;

  Transform(sc_core::sc_module_name name)
    : smoc_actor(name, q), i1("i1"), i2("i2"), o1("o1"), o2("o2")
  {
    smoc_firing_state r("r");
    smoc_and_state    f;

    q.init(r).add(f);

    smoc_firing_state   f1("f1");
    smoc_firing_state   f2("f2");
    f.add(f1).add(f2);

    q  = (i1(2) && i2(2)) >> f;
    f1 = (i1(1) && SMOC_TOKEN(i1,0) != -1 && o1(1)) >> SMOC_CALL(Transform::copy1) >> f1;
    f2 = (i2(1) && SMOC_TOKEN(i2,0) != -1 && o2(1)) >> SMOC_CALL(Transform::copy2) >> f2;
    f  = (i1(1) && SMOC_TOKEN(i1,0) == -1 && i2(1) && SMOC_TOKEN(i2,0) == -1) >> q;
  }

private:
  smoc_xor_state q;

  void copy1() {
    o1[0] = i1[0];
    std::cout << "copy " << i1[0] << " from i1 to o1";
  }

  void copy2() {
    o2[0] = i2[0];
    std::cout << "copy " << i2[0] << " from i2 to o2";
  }
};

void preTransformDiscard(smoc_actor *actor, const std::string &src, const std::string &action, const std::string &dst) {
  Transform<int> *a = static_cast<Transform<int> *>(actor);
  std::cout << actor->name() << "[" << action << "]: " << src << " -- discarding i1[0] == " << a->i1[0] << ", i1[1] == " << a->i1[1] << ", i2[0] == " << a->i2[0] << ", i2[1] == " << a->i2[1] << " --> " << dst << std::endl;
}

void preTransformReset(smoc_actor *actor, const std::string &src, const std::string &action, const std::string &dst) {
  Transform<int> *a = static_cast<Transform<int> *>(actor);
  std::cout << actor->name() << "[" << action << "]: " << src << " -- reseting with i1[0] == " << a->i1[0] << ", i2[0] == " << a->i2[0] << " --> " << dst << std::endl;
}

void preTransformCopy1(smoc_actor *actor, const std::string &src, const std::string &action, const std::string &dst) {
  std::cout << actor->name() << "[" << action << "]: " << src << " -- ";
}

void postTransformCopy1(smoc_actor *actor, const std::string &src, const std::string &action, const std::string &dst) {
  std::cout << " --> " << dst << std::endl;
}

void preTransformCopy2(smoc_actor *actor, const std::string &src, const std::string &action, const std::string &dst) {
  std::cout << actor->name() << "[" << action << "]: " << src << " -- ";
}

void postTransformCopy2(smoc_actor *actor, const std::string &src, const std::string &action, const std::string &dst) {
  std::cout << " --> " << dst << std::endl;
}

class Top : public smoc_graph {
public:
  Top(sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name),
      src1("src1", iter), src2("src2", iter*1.5f),
      snk1("snk1"), snk2("snk2"),
      trans("transform")
  {
    connectNodePorts<9>(src1.out, trans.i1);
    connectNodePorts<9>(src2.out, trans.i2);
    connectNodePorts(trans.o1, snk1.in);
    connectNodePorts(trans.o2, snk2.in);
  }

public:
  Src<int> src1, src2;
  Snk<int> snk1, snk2;
  Transform<int> trans;
};

void nothing(smoc_actor *actor, const std::string &src, const std::string &action, const std::string &dst) {
}

void nomatch(smoc_actor *actor, const std::string &src, const std::string &action, const std::string &dst) {
  std::cout << "ERROR THIS SHOULD NEVER MATCH!!!" << std::endl;
  assert(!"WTF");
}

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(-1);

  if (argc >= 2)
    iter = std::atol(argv[1]);

  smoc_top_moc<Top> top("top", iter);

  smoc::smoc_add_transition_hook(&top.src1, "^start$", "", "", &nomatch, &nomatch);
  smoc::smoc_add_transition_hook(&top.src1, "^top\\.src1:start$", "^Src::src\\(\\)$", "^top\\.src1:start$", &srcPre, &srcPost);
  smoc::smoc_add_transition_hook(&top.src2, "", "^src$", "", &nomatch, &nomatch);
  smoc::smoc_add_transition_hook(&top.src2, "", "", "^start$", &nomatch, &nomatch);
  smoc::smoc_add_transition_hook(&top.src2, "start", "src", "start$", &srcPre, &srcPost);

  smoc::smoc_add_transition_hook(&top.trans, ":r$", "^$", ":f1,f2$", &preTransformDiscard, &nothing);
  smoc::smoc_add_transition_hook(&top.trans, ":f1,f2$", "^$", ":r$", &preTransformReset, &nothing);
  smoc::smoc_add_transition_hook(&top.trans, "", "copy1", "", &preTransformCopy1, &postTransformCopy1);
  smoc::smoc_add_transition_hook(&top.trans, "", "copy2", "", &preTransformCopy1, &postTransformCopy2);

  smoc::smoc_add_transition_hook(&top.snk1, "^start$", ".*", ".*", &nomatch, &nomatch);
  smoc::smoc_add_transition_hook(&top.snk1, "snk1:start$", "^Snk::snk\\(\\)$", "^top\\.snk1:start$", &snkPre, &snkPost);
  smoc::smoc_add_transition_hook(&top.snk2, ".*", "^src$", ".*", &nomatch, &nomatch);
  smoc::smoc_add_transition_hook(&top.snk2, ".*", ".*", "^start$", &nomatch, &nomatch);
  smoc::smoc_add_transition_hook(&top.snk2, "snk2:start", "snk", "snk2:start$", &snkPre, &snkPost);

  sc_core::sc_start();
  return 0;
}
