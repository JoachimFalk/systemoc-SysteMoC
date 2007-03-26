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
    C0[0]=b[0];C0[1]=b[ 8];C0[2]=b[16];C0[3]=b[24];C0[4]=b[32];C0[5]=b[40];C0[6]=b[48];C0[7]=b[56];
    C1[0]=b[1];C1[1]=b[ 9];C1[2]=b[17];C1[3]=b[25];C1[4]=b[33];C1[5]=b[41];C1[6]=b[49];C1[7]=b[57];
    C2[0]=b[2];C2[1]=b[10];C2[2]=b[18];C2[3]=b[26];C2[4]=b[34];C2[5]=b[42];C2[6]=b[50];C2[7]=b[58];
    C3[0]=b[3];C3[1]=b[11];C3[2]=b[19];C3[3]=b[27];C3[4]=b[35];C3[5]=b[43];C3[6]=b[51];C3[7]=b[59];
    C4[0]=b[4];C4[1]=b[12];C4[2]=b[20];C4[3]=b[28];C4[4]=b[36];C4[5]=b[44];C4[6]=b[52];C4[7]=b[60];
    C5[0]=b[5];C5[1]=b[13];C5[2]=b[21];C5[3]=b[29];C5[4]=b[37];C5[5]=b[45];C5[6]=b[53];C5[7]=b[61];
    C6[0]=b[6];C6[1]=b[14];C6[2]=b[22];C6[3]=b[30];C6[4]=b[38];C6[5]=b[46];C6[6]=b[54];C6[7]=b[62];
    C7[0]=b[7];C7[1]=b[15];C7[2]=b[23];C7[3]=b[31];C7[4]=b[39];C7[5]=b[47];C7[6]=b[55];C7[7]=b[63];
  }
  smoc_firing_state start;
public:
  m_block2row(sc_module_name name)
    : smoc_actor(name, start){
    start = (b.getAvailableTokens() >= 64)   >>
            (C0.getAvailableSpace() >= 8 &&
             C1.getAvailableSpace() >= 8 &&
             C2.getAvailableSpace() >= 8 &&
             C3.getAvailableSpace() >= 8 &&
             C4.getAvailableSpace() >= 8 &&
             C5.getAvailableSpace() >= 8 &&
             C6.getAvailableSpace() >= 8 &&
             C7.getAvailableSpace() >= 8)   >>
            CALL(m_block2row::action0)      >> start;
    
    }
  virtual ~m_block2row(){}
};
#endif // _INCLUDED_BLOCK2ROW_HPP
