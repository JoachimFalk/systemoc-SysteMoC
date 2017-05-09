//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph.hpp>

#include <systemc.h>
#include <tlm.h>

#include <iostream>

SC_MODULE(A) {

  sc_port< tlm::tlm_blocking_put_if<int> > out;
  sc_port< tlm::tlm_blocking_get_if<int> > in;

  SC_CTOR(A) {
    SC_THREAD(main);
  }

  void main() {
    for(size_t i = 0; i < 10000; ++i) {
      int x = in->get();
      std::cout << "A: got " << x << std::endl;
      out->put(x);
      std::cout << "A: put " << x << std::endl;
    }
  }
};

class B : public smoc_actor {
public:
  smoc_port_out<int> out;
  smoc_port_in<int> in;

  B(sc_core::sc_module_name name) :
    smoc_actor(name, run)
  {
    run = in(1) >> out(1) >> CALL(B::copy) >> run;
  }

  void copy() {
    int x = in[0];
    std::cout << "B: got " << x << std::endl;
    out[0] = x;
    std::cout << "B: put " << x << std::endl;
  }

private:
  smoc_firing_state run;
};

class Src : public smoc_actor {
public:
  smoc_port_out<int> out;

  Src(sc_core::sc_module_name name) :
    smoc_actor(name, run),
    x(0)
  {
    run = out(1) >> CALL(Src::copy) >> run;
  }

  void copy() {
    std::cout << "Src: " << x << std::endl;
    out[0] = x++;
  }

private:
  smoc_firing_state run;
  int x;
};

class Snk : public smoc_actor {
public:
  smoc_port_in<int> in;

  Snk(sc_core::sc_module_name name) :
    smoc_actor(name, run)
  {
    run = in(1) >> CALL(Snk::copy) >> run;
  }

  void copy() {
    int x = in[0];
    std::cout << "Snk: " << x << std::endl;
  }

private:
  smoc_firing_state run;
};

class G : public smoc_graph {
public:
  // smoc_port_in<int> in; <-- won't work
  sc_port< tlm::tlm_blocking_get_if<int> > in;
  smoc_port_out<int> out;

  G(sc_core::sc_module_name name) :
    smoc_graph(name),
    a("a"),
    b("b")
  {
    a.in(in);
    b.out(out);
    connectNodePorts(a.out,b.in);
  }
private:
  A a;
  B b;
};

class GTop : public smoc_graph {
public:
  GTop(sc_core::sc_module_name name) :
    smoc_graph(name),
    src("src"),
    snk("snk"),
    g("g")
  {
    connectNodePorts(src.out,g.in);
    connectNodePorts(g.out,snk.in);
  }
private:
  Src src;
  Snk snk;
  G g;
};


int sc_main (int argc, char **argv) {
  
  GTop gtop("gtop");

  smoc_scheduler_top top(&gtop);
  sc_core::sc_start();
  return 0;
}
