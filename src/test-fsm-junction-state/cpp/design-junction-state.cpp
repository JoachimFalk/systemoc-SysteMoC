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
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph.hpp>

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
    : smoc_actor(name, init)
  {
    smoc_junction_state s;
    
    init = 
         (in(1) && GUARD(Transform::mod)(2))
      >> CALL(Transform::store)("init -> s (mod2)")
      >> s
    |    (in(1) && !GUARD(Transform::mod)(2))
      >> CALL(Transform::store)("init -> s (!mod2)")
      >> s;

    // because s is a smoc_junction_state, the guards
    // of s's transitions are also allowed to read from
    // the input tokens requested in the transitions of
    // init with target state s!
    // Also, guards must not depend on values which are
    // calculated by init's actions, because guards will
    // also be evaluated atomically!!!

    s =
         (out(1) && GUARD(Transform::mod)(3))
      >> CALL(Transform::write)("s -> init (mod3)")
      >> init
    |    (out(1) && GUARD(Transform::mod)(5))
      >> CALL(Transform::write)("s -> init (mod5)")
      >> init
    |    (out(1) && !GUARD(Transform::mod)(3) && !GUARD(Transform::mod)(5))
      >> CALL(Transform::write)("s -> init (!mod3 && !mod5)")
      >> init;
  }

private:
  smoc_firing_state init;
  T t;

  bool mod(T m) const {
    return (in[0] % m) == 0;
  }

  void store(const char* n) {
    t = in[0];
    std::cout << name() << ": stored token with value "
              << t << std::endl;
    std::cout << name() << ": transition " << n
              << std::endl;
  }
  
  void write(const char* n) {
    out[0] = t;
    std::cout << name() << ": wrote token with value "
              << t << std::endl;
    std::cout << name() << ": transition " << n
              << std::endl;
  }
  
  void nop(const char* n) {
    std::cout << name() << ": transition " << n
              << std::endl;
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
