// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
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
#include <systemoc/smoc_node_types.hpp>

template<class T>
class Src : public smoc_actor {
public:
  smoc_port_out<T> out;

  Src(sc_core::sc_module_name name, size_t _iter)
    : smoc_actor(name, start),
      iter(_iter), i(0)
  {
    start =
         (out(1) && VAR(iter) > 0U)
      >> CALL(Src::src)
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
    : smoc_actor(name, start)
  {
    start =
         in(1)
      >> CALL(Snk::snk)
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
  smoc_port_in<T> in;
  smoc_port_out<T> out;

  Transform(sc_core::sc_module_name name)
    : smoc_actor(name, a), a("a")
  {
    smoc_firing_state   s_s_a("s_s_a");
    smoc_firing_state   s_s_b("s_s_b");

    smoc_xor_state  s_a;
    s_a.init(s_s_a).add(s_s_b);

    smoc_firing_state   s_b("s_b");

    smoc_firing_state   b("b");
    smoc_xor_state  c;
    c.init(s_a).add(s_b);

    smoc_firing_state   d("d");
    smoc_firing_state   e("e");
    
    /*
     * Add sub-states (non-initial states only)
     */
    

    /*
     * Top FSM
     */

    a =
         in(1)
      >> CALL(Transform::store)("a -> b")
      >> b;
  
    d =
         out(1)
      >> CALL(Transform::write)("d -> a")
      >> a;

    e =
         out(1)
      >> CALL(Transform::write)("e -> a")
      >> a;
    
    /*
     * Transitions into sub FSM(s)
     */
    
    b =
         GUARD(Transform::odd)
      >> CALL(Transform::process)("b -> s_s_b")
      >> s_s_b
    |    !GUARD(Transform::odd)
      >> CALL(Transform::process)("b -> c")
      >> c;

    /*
     * Sub FSM(s)
     */

    s_a =
         CALL(Transform::process)("s_a -> s_b")
      >> s_b;

    /*
     * Transitions from sub FSM(s)
     */

    s_b =
         GUARD(Transform::odd)
      >> CALL(Transform::process)("s_b -> e")
      >> e;

    c =
         !GUARD(Transform::odd)
      >> CALL(Transform::process)("c -> d")
      >> d;
  }

private:
  smoc_firing_state a;
  T t;

  bool odd() const {
    return t & 1;
  }

  void process(const char* tname) {
    std::cout << name() << ": transition " << tname
              << std::endl;
  }

  void store(const char* tname) {
    std::cout << name() << ": transition " << tname
              << std::endl;
    std::cout << name() << ": store token with value "
              << in[0] << std::endl;
    t = in[0];
  }
  
  void write(const char* tname) {
    std::cout << name() << ": transition " << tname
              << std::endl;
    std::cout << name() << ": write token with value "
              << t << std::endl;
    out[0] = t;
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
    connectNodePorts(src.out, trans.in);
    connectNodePorts(trans.out, snk.in);
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
