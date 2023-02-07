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

#include <cstdlib>
#include <iostream>
#include <ctime>

#include <sys/time.h>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_multiplex_fifo.hpp>
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph.hpp>

#include <CoSupport/DataTypes/CheckedVector.hpp>
#include <CoSupport/Streams/stl_output_for_pair.hpp>
#include <CoSupport/String/Concat.hpp>

#include <boost/random/uniform_int.hpp>
#include <boost/random/mersenne_twister.hpp>

using CoSupport::DataTypes::CheckedVector;
using CoSupport::String::Concat;

uint64_t getUSecs() {
  timeval tv;
  gettimeofday(&tv,0);
  return tv.tv_sec * 1000000ull + tv.tv_usec;
}

typedef std::pair<size_t, int> Token;

struct ColorAccessor {
  static
  size_t get(const Token &t)
    { return t.first; }
  static
  void   put(Token &t, size_t c)
    { t.first = c; }
};

class m_h_src: public smoc_actor {
public:
  smoc_port_out<Token> out;
private:
  boost::random::mt19937                    rng;
  boost::random::uniform_int_distribution<> die;
  size_t                                    iter;
  int val;

  void src() {
    Token t(die(rng), val++);

    std::cout
      << name() << ": generate token with value " << t << std::endl;
  
    out[0] = t;

    assert(iter > 0u); 
    --iter;
  }

  smoc_firing_state start;
public:
  m_h_src(sc_core::sc_module_name name, size_t seed, size_t iter, size_t inst)
    : smoc_actor(name, start),
      rng(seed), die(0, inst-1),
      iter(iter),
      val(0)
  {
    start =
        (out(1) && (SMOC_VAR(this->iter) > 0U)) >>
        SMOC_CALL(m_h_src::src) >> start
      ;
  }
};

class m_h_snk: public smoc_actor {
public:
  smoc_port_in<Token> in;
private:
  const size_t idx;

  void snk() {
    std::cout
      << name() << ": received token with value " << in[0] << std::endl;
    assert(in[0].first == idx);
  }
  
  smoc_firing_state start;
public:
  m_h_snk(sc_core::sc_module_name name, size_t idx)
    : smoc_actor(name, start),
      idx(idx)
  {
    start = in(1) >> SMOC_CALL(m_h_snk::snk) >> start;
  }
  
  struct factory {
  public:
    factory(const char* p)
      : p(p)
      {}
    void construct(m_h_snk* m, size_t i)
      { new (m) m_h_snk(Concat(p)(i).get().c_str(), i); }
  private:
    const char* p;
  };
};

class m_h_top: public smoc_graph {
protected:
  m_h_src  src;
  CheckedVector<m_h_snk> snk;
public:
  m_h_top(sc_core::sc_module_name name, size_t iter, size_t inst, size_t size, size_t ooo, size_t seed)
    : smoc_graph(name),
      src("src", seed, iter, inst),
      snk(inst, m_h_snk::factory("snk"))
  {
    smoc_multiplex_fifo<Token, ColorAccessor> f(size,ooo);
    f.connect(src.out);
    for(size_t i = 0; i < inst; ++i) {
      smoc_multiplex_vfifo<Token, ColorAccessor> foo(f.getVirtFifo());
      foo.getFifoId();
      foo.connect(snk[i].in);
//    f.getVirtFifo().connect(snk[i].in);
    }
  }
};

int sc_main (int argc, char **argv) {
  size_t iter = static_cast<size_t>(-1);
  size_t inst = 3;
  size_t size = 17;
  size_t ooo = 3;
  size_t seed = getUSecs();
 
  // number of iterations 
  if (argc >= 2)
    iter = atol(argv[1]);
  
  // number of port pairs
  if (argc >= 3)
    inst = atol(argv[2]);
  
  // size of multiplex fifo 
  if (argc >= 4)
    size = atol(argv[3]);

  // size of ooo area
  if (argc >= 5)
    ooo = atol(argv[4]);
  
  // seed
  if (argc >= 6)
    seed =  atol(argv[5]);

  smoc_top_moc<m_h_top> top("top", iter, inst, size, ooo, seed);
  sc_core::sc_start();
  return 0;
}
