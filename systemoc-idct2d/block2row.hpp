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

#ifndef _INCLUDED_BLOCK2ROW_HPP
#define _INCLUDED_BLOCK2ROW_HPP

#include "callib.hpp"

#ifdef VERBOSE_ACTOR
#define VERBOSE_IDCT_BLOCK2ROW
#endif

class m_block2row: public smoc_actor {
public:
  smoc_port_in<int>  b;
  smoc_port_out<int> C0, C1, C2, C3, C4, C5, C6, C7;
private:
  void action0() {
#ifdef VERBOSE_IDCT_BLOCK2ROW
#ifndef NDEBUG
    for ( int i = 0; i < 64; i++ )
#ifndef XILINX_EDK_RUNTIME
      cout << name() << ": " << "b[" << i << "] = " << b[i] << endl;
#else
      xil_printf("%s: b[%d] = %d\r\n",name(),i,b[i]);
#endif
#endif
#endif
    for ( int i = 0; i < 8; ++i ) {
      C0[i] = b[8*i];
      C1[i] = b[8*i+1];
      C2[i] = b[8*i+2];
      C3[i] = b[8*i+3];
      C4[i] = b[8*i+4];
      C5[i] = b[8*i+5];
      C6[i] = b[8*i+6];
      C7[i] = b[8*i+7];
    }
  }
  smoc_firing_state start;
public:
  m_block2row(sc_module_name name)
    : smoc_actor(name, start) {
    start = b(64)                                 >>
            (C0(8) && C1(8) && C2(8) && C3(8) &&
             C4(8) && C5(8) && C6(8) && C7(8))    >>
            CALL(m_block2row::action0)            >> start;
  }
  virtual ~m_block2row(){}
};
#endif // _INCLUDED_BLOCK2ROW_HPP
