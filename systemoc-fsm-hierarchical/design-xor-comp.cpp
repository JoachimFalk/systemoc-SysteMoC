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

class Src: public smoc_actor {
public:
  smoc_port_out<int> out;

  Src(sc_module_name name, size_t _iter)
    : smoc_actor(name, start),
      start("start"),
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
  int i;

  void src() {
    i = rand() & 3;
    std::cout << name() << ": generate token with value "
              << i << std::endl;
    out[0] = i; --iter;
  }
};

class Transform : public smoc_actor {
public:
  smoc_port_in<int>  in;
protected:
  smoc_xor_state    d;
  smoc_firing_state a;
  smoc_firing_state b;
  smoc_firing_state c;
public:
  Transform(sc_module_name name)
    : smoc_actor(name, d),
      d("d"), a("a"), b("b"), c("c")
  {
    d.init(a).add(b);

    a =
        (in(1) && Expr::token(in,0) == 0) >> CALL(Transform::print)("a->b") >> b
      |
        (in(1) && Expr::token(in,0) != 0) >> CALL(Transform::print)("a->a") >> a
      ;
    b =
        in(1)                             >> CALL(Transform::print)("b->b") >> b
      ;
    d =
        (in(1) && Expr::token(in,0) == 1) >> CALL(Transform::print)("d->c") >> c;
      ;
    c =
        (in(1) && Expr::token(in,0) == 2) >> CALL(Transform::print)("c->d") >> d
      |
        (in(1) && Expr::token(in,0) != 2) >> CALL(Transform::print)("c->b") >> b;
  }
private:
  void print(const char *tname)
    { std::cout << name() << ": transition " << tname << std::endl; }
};

class Top : public smoc_graph {
public:
  Top(sc_module_name name, size_t iter)
    : smoc_graph(name),
      src("src", iter),
      trans("transform")
  {
    connectNodePorts(src.out, trans.in);
  }

private:
  Src       src;
  Transform trans;
};

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(-1);

  if (argc >= 2)
    iter = std::atol(argv[1]);

  smoc_top_moc<Top> top("top", iter);

  sc_start();
  return 0;
}
