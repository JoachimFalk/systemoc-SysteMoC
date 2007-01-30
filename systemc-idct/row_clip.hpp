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

#include "callib.hpp"
#include "IDCTclip.hpp"
#include "min_duplex.hpp"

#define MAXVAL_PIXEL		(  255 ) // 2^BITS_PIXEL - 1

class m_clip: public sc_module {
public:
  sc_fifo_in<int>  i0, i1, i2, i3, i4, i5, i6, i7, min; 
  sc_fifo_out<int> o0, o1, o2, o3, o4, o5, o6, o7;
private:
  sc_fifo<int> f0, f1, f2, f3, f4, f5, f6, f7;
  
  m_IDCTclip   clip0, clip1, clip2, clip3, clip4, clip5, clip6, clip7;
  m_MIN_duplex dup;
public:
  m_clip( sc_module_name name )
    : sc_module(name),
      f0(DEFAULT_FIFO_SIZE),
      f1(DEFAULT_FIFO_SIZE),
      f2(DEFAULT_FIFO_SIZE),
      f3(DEFAULT_FIFO_SIZE),
      f4(DEFAULT_FIFO_SIZE),
      f5(DEFAULT_FIFO_SIZE),
      f6(DEFAULT_FIFO_SIZE),
      f7(DEFAULT_FIFO_SIZE),
      clip0("clip0", MAXVAL_PIXEL),
      clip1("clip1", MAXVAL_PIXEL),
      clip2("clip2", MAXVAL_PIXEL),
      clip3("clip3", MAXVAL_PIXEL),
      clip4("clip4", MAXVAL_PIXEL),
      clip5("clip5", MAXVAL_PIXEL),
      clip6("clip6", MAXVAL_PIXEL),
      clip7("clip7", MAXVAL_PIXEL),
      dup("dup") {
    clip0.I(i0); clip1.I(i1); clip2.I(i2); clip3.I(i3);
    clip4.I(i4); clip5.I(i5); clip6.I(i6); clip7.I(i7);
    dup.I(min);
    
    dup.O0(f0); clip0.MIN(f0);
    dup.O1(f1); clip1.MIN(f1);
    dup.O2(f2); clip2.MIN(f2);
    dup.O3(f3); clip3.MIN(f3);
    dup.O4(f4); clip4.MIN(f4);
    dup.O5(f5); clip5.MIN(f5);
    dup.O6(f6); clip6.MIN(f6);
    dup.O7(f7); clip7.MIN(f7);
    
    clip0.O(o0); clip1.O(o1); clip2.O(o2); clip3.O(o3);
    clip4.O(o4); clip5.O(o5); clip6.O(o6); clip7.O(o7);
  }
};

