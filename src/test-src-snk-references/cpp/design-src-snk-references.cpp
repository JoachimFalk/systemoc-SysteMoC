// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of
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

#include <systemoc/smoc_moc.hpp>

template<typename T, int SIZE>
  class Token {
  public:
    T Data[SIZE];
    Token(){}; // default constructor is needed for software synthese
    T &operator[](int index)
    {
      assert(0 <= index && index < SIZE);
      return Token<T, SIZE>::Data[index];
    }
  };

template<typename T, int SIZE>
  inline std::ostream &operator <<(std::ostream &out,
      const Token<T, SIZE> &token) {
    out << "...";
    return out;
  }

//template <typename T>
class m_h_src: public smoc_actor {
public:
  smoc_port_out<Token<int, 1> > out;
private:
  int     i=0;
  unsigned int  iter;
  
  void src() {
    Token<int, 1> &frame = out[0]; // create output reference
    std::cout << name() << ": generate token with value " << i << std::endl;
    i++;
    frame.Data[i] = i;
    --iter;
  }
  smoc_firing_state start;
public:
  m_h_src(sc_core::sc_module_name name, size_t _iter)
    : smoc_actor(name, start),
      i(1), iter(_iter) {
    SMOC_REGISTER_CPARAM(_iter);
    char *init = getenv("SRC_ITERS");
    if (init)
      iter = atoll(init);
    start = out(1) >> (VAR(iter) > 0) >> CALL(m_h_src::src) >> start;
  }
};


//template <typename T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_out<Token<int, 1> > in;
private:
  
  void sink(void) {
    Token<int, 1> frame;
    frame = in[0];
    std::cout << name() << ": received token with value " << frame.Data[0] << std::endl;
  }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_core::sc_module_name name)
    : smoc_actor(name, start) {
    start = in(1) >> CALL(m_h_sink::sink) >> start;
  }
};

class m_h_top: public smoc_graph {
public:

  m_h_src src;
  m_h_sink snk;

  m_h_top(sc_core::sc_module_name name, unsigned int iter)
    : smoc_graph(name),
      src("src", iter), snk("snk") {
    connectNodePorts(src.out,snk.in);
    //smoc_fifo<Token<int, 1> >q("queue", 2);
    //q.connect(src.out).connect(snk.in);
  }
};

int sc_main (int argc, char **argv) {
  unsigned int iter = 1; //
  if (argc >= 2)
    iter = atoi(argv[1]);
  std::cout << "Program starts !!!" << std::endl;
  
  smoc_top_moc<m_h_top> top("top", iter);
  sc_core::sc_start();
  std::cout << "Program ends !!!" << std::endl;
  return 0;
}
