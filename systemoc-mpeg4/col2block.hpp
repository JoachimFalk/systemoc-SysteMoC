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

#ifndef _INCLUDED_COL2BLOCK_HPP
#define _INCLUDED_COL2BLOCK_HPP

class m_col2block: public smoc_actor {
public:
  smoc_port_in<int> R0, R1, R2, R3, R4, R5, R6, R7;
  smoc_port_out<int> b;
private:
  void action0() {
    for ( int i = 0; i < 8; i++ )
      b[0*8 + i] = R0[i];
    for ( int i = 0; i < 8; i++ )
      b[1*8 + i] = R1[i];
    for ( int i = 0; i < 8; i++ )
      b[2*8 + i] = R2[i];
    for ( int i = 0; i < 8; i++ )
      b[3*8 + i] = R3[i];
    for ( int i = 0; i < 8; i++ )
      b[4*8 + i] = R4[i];
    for ( int i = 0; i < 8; i++ )
      b[5*8 + i] = R5[i];
    for ( int i = 0; i < 8; i++ )
      b[6*8 + i] = R6[i];
    for ( int i = 0; i < 8; i++ )
      b[7*8 + i] = R7[i];
  }
  
  smoc_firing_state start;
public:
  m_col2block(sc_module_name name)
    : smoc_actor(name, start) {
    start = (R0(8) && R1(8) && R2(8) && R3(8) &&
             R4(8) && R5(8) && R6(8) && R7(8))    >>
            b(64)                                 >>
            CALL(m_col2block::action0)            >> start;
  }
  
  virtual ~m_col2block(){}
};

#endif // _INCLUDED_COL2BLOCK_HPP
