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

#include <systemc.h>

#include "callib.hpp"

class m_col2block: public sc_module {
public:
  sc_fifo_in<int> R0, R1, R2, R3, R4, R5, R6, R7;
  sc_fifo_out<int> b;
private:
  void action() {
    while (true) {
      for ( int i = 0; i < 8; i++ )
        b.write(R0.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R1.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R2.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R3.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R4.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R5.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R6.read());
      for ( int i = 0; i < 8; i++ )
        b.write(R7.read());
    }
  }
public:
  SC_HAS_PROCESS(m_col2block);
 
  m_col2block(sc_module_name name)
    : sc_module(name) {
    SC_THREAD(action);
  }
};

#endif // _INCLUDED_COL2BLOCK_HPP
