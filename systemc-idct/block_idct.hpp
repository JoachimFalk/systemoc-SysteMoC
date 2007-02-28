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

#include "callib.hpp"
#include "IDCT2d.hpp"
#include "block2row.hpp"
#include "col2block.hpp"

class m_block_idct: public sc_module {
public:
  sc_fifo_in<int> I;  
  sc_fifo_in<int> MIN;
  sc_fifo_out<int> O;
private:
  sc_fifo<int>  f0,  f1,  f2,  f3,  f4,  f5,  f6,  f7,
                f8,  f9, f10, f11, f12, f13, f14, f15;
  
  m_block2row block2row1;
  m_col2block col2block1;
  m_idct2d    idct2d1;
public:
  m_block_idct(sc_module_name name )
    : sc_module(name),
      f0(16),
      f1(16),
      f2(16),
      f3(16),
      f4(16),
      f5(16),
      f6(16),
      f7(16),
      f8(16),
      f9(16),
      f10(16),
      f11(16),
      f12(16),
      f13(16),
      f14(16),
      f15(16),
      block2row1("block2row1"),
      col2block1("col2block1"),
      idct2d1("idct2d1") {
    block2row1.b(I);
    
    idct2d1.min(MIN);
    
    block2row1.C0(f0); idct2d1.i0(f0);
    block2row1.C1(f1); idct2d1.i1(f1);
    block2row1.C2(f2); idct2d1.i2(f2);
    block2row1.C3(f3); idct2d1.i3(f3);
    block2row1.C4(f4); idct2d1.i4(f4);
    block2row1.C5(f5); idct2d1.i5(f5);
    block2row1.C6(f6); idct2d1.i6(f6);
    block2row1.C7(f7); idct2d1.i7(f7);
    
    idct2d1.o0(f8); col2block1.R0(f8);
    idct2d1.o1(f9); col2block1.R1(f9);
    idct2d1.o2(f10); col2block1.R2(f10);
    idct2d1.o3(f11); col2block1.R3(f11);
    idct2d1.o4(f12); col2block1.R4(f12);
    idct2d1.o5(f13); col2block1.R5(f13);
    idct2d1.o6(f14); col2block1.R6(f14);
    idct2d1.o7(f15); col2block1.R7(f15);
    
    col2block1.b(O);
  }
};
