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

#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>

class Src: public smoc_actor {
public:
  smoc_port_out<void> o1, o2, o3, o4;

  Src(sc_core::sc_module_name name, size_t _iter)
    : smoc_actor(name, start),
      start("start"),
      rng(4711), die(1,3), iter(_iter), cv(-1)
  {
    start = o2(1)                                        >> SMOC_CALL(Src::newcase) >> run;
    run   = (o2(1) && o3(1)          && SMOC_VAR(cv)==1) >> SMOC_CALL(Src::newcase) >> run
          | (o1(1) && o2(1) && o3(1) && SMOC_VAR(cv)==2) >> SMOC_CALL(Src::newcase) >> run
          | (o2(1) && o4(1)          && SMOC_VAR(cv)==3) >> SMOC_CALL(Src::newcase) >> run
          |                            (SMOC_VAR(cv)==0)                            >> end;
  }

private:
  smoc_firing_state start, run, end;

  boost::random::mt19937                    rng;
  boost::random::uniform_int_distribution<> die;
  size_t                                    iter;
  int                                       cv;

  void newcase() {
    int oldcv = cv;
    if (--iter > 0)
      cv = die(rng);
    else
      cv = 0;
    std::cout << name() << ": from case " << oldcv << " to " << cv << std::endl;
  }
};

class Transform : public smoc_actor {
public:
  smoc_port_in<void>  i1, i2, i3, i4;
protected:
  smoc_xor_state    d;
  smoc_firing_state a, b, c;
public:
  Transform(sc_core::sc_module_name name)
    : smoc_actor(name, d),
      i1("i1"), i2("i2"), i3("i3"), i4("i4"),
      d("d"), a("a"), b("b"), c("c")
  {
    d.init(a).add(c);

    a = i1(1) >> SMOC_CALL(Transform::print)("a->b") >> c;
    d = i2(1) >> SMOC_CALL(Transform::print)("d->b") >> b;
    b =
        i3(1) >> SMOC_CALL(Transform::print)("b->d") >> d
      | i4(1) >> SMOC_CALL(Transform::print)("b->c") >> c;
  }
private:
  void print(const char *tname)
    { std::cout << name() << ": transition " << tname << std::endl; }
};

class Top : public smoc_graph {
public:
  Top(sc_core::sc_module_name name, size_t iter)
    : smoc_graph(name),
      src("src", iter),
      trans("transform")
  {
    connectNodePorts(src.o1, trans.i1);
    connectNodePorts(src.o2, trans.i2);
    connectNodePorts(src.o3, trans.i3);
    connectNodePorts(src.o4, trans.i4);
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

  sc_core::sc_start();
  return 0;
}
