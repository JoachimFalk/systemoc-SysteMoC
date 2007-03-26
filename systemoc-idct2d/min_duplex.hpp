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

#ifndef _INCLUDED_MIN_DUPLEX_HPP
#define _INCLUDED_MIN_DUPLES_HPP

#ifdef VERBOSE_ACTOR
#define VERBOSE_MIN_DUPLEX
#endif

class m_MIN_duplex: public smoc_actor {
public:
  smoc_port_in<int> I;
  smoc_port_out<int> O0;
  smoc_port_out<int> O1;
  smoc_port_out<int> O2;
  smoc_port_out<int> O3;
  smoc_port_out<int> O4;
  smoc_port_out<int> O5;
  smoc_port_out<int> O6;
  smoc_port_out<int> O7;

private:
  
  void action() {
#ifdef VERBOSE_MIN_DUPLEX
#ifndef NDEBUG
#ifndef XILINX_EDK_RUNTIME
		cout << name() << ": " << "I[0] = " << I[0] << endl;
#else
		xil_printf("%s: I[0] = %d\r\n",name(),I[0]);
#endif		
#endif
#endif
    O0[0] = I[0];
    O1[0] = I[0];
    O2[0] = I[0];
    O3[0] = I[0];
    O4[0] = I[0];
    O5[0] = I[0];
    O6[0] = I[0];
    O7[0] = I[0];
  }
  
  smoc_firing_state start;
  
public:
  m_MIN_duplex(sc_module_name name)
    : smoc_actor(name, start){
      start =  (I.getAvailableTokens() >= 1) >>
      		(O0.getAvailableSpace() >= 1 &&
		O1.getAvailableSpace() >= 1 &&
		O2.getAvailableSpace() >= 1 &&
		O3.getAvailableSpace() >= 1 &&
		O4.getAvailableSpace() >= 1 &&
		O5.getAvailableSpace() >= 1 &&
		O6.getAvailableSpace() >= 1 &&
		O7.getAvailableSpace() >= 1) >>
	      CALL(m_MIN_duplex::action) 	>> start;
    }
};

#endif // _INCLUDED_MIN_DUPLEX_HPP
