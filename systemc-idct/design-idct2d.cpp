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

#include <systemc.h>

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include "callib.hpp"
#include "block_idct.hpp"

#define INAMEblk "test_in.dat"
#define ONAMEblk "test_out.dat"

class m_source_idct: public sc_module {
public:
  sc_fifo_out<int> out;
  sc_fifo_out<int> min;
private:
  size_t periods;

  std::ifstream i1; 

  void process() {
    for (size_t i = 0; i < periods * 64; ) {
#ifndef NDEBUG
      if (i1.good()) {
#endif
        for ( size_t j = 0; j <= 63; j++ ) {
          int myOut;
          
          i++;
#ifdef NDEBUG
          myOut = i;
#else
          i1 >> myOut;
          std::cout << name() << " write " << myOut << std::endl;
#endif
          out.write(myOut);
        }
        int myMin = -256;
        
        min.write(myMin);
#ifndef NDEBUG
        std::cout << name() << " write min " << myMin << std::endl;
      } else {
        std::cout << "File empty! Please create a file with name test_in.dat!" << std::endl;
        exit (1) ;
      }
#endif
    }
  }
public:
  SC_HAS_PROCESS(m_source_idct);

  m_source_idct( sc_module_name name, size_t periods )
    : sc_module(name), periods(periods),
      i1(INAMEblk) {
    SC_THREAD(process);
  }
 
  ~m_source_idct( ){
    i1.close();
  }
};

class m_sink: public sc_module {
public:
  sc_fifo_in<int> in;
private:
  std::ofstream fo; 
  int           foo;

  void process() {
    while (true) {
      foo = in.read();
#ifndef NDEBUG
      std::cout << name() << " receiving " << foo << std::endl;
      fo << foo << std::endl;
#endif
    }
  }
public:
  SC_HAS_PROCESS(m_sink);
 
  m_sink(sc_module_name name)
    : sc_module(name),
      fo(ONAMEblk) {
    SC_THREAD(process);
  }

  ~m_sink() {
    fo.close();
  }
};

class IDCT2D_TEST: public sc_module {
private:
  sc_fifo<int>  f0,  f1,  f2;
  
  m_source_idct src_idct;
  m_block_idct  blidct;
  m_sink        snk;
public:
  IDCT2D_TEST(sc_module_name name, size_t periods)
    : sc_module(name),
      f0(128), f1(2), f2(128),
      src_idct("src_idct", periods),
      blidct("blidct"),
      snk("snk") {
    src_idct.out(f0); blidct.I(f0);
    src_idct.min(f1); blidct.MIN(f1);
    blidct.O(f2); snk.in(f2);
  }
};

int sc_main (int argc, char **argv) {
  size_t periods =
    (argc > 1)
    ? atoi(argv[1])
    : 100;
  
  IDCT2D_TEST top("top", periods);
  
  sc_start(-1);
  return 0;
}
