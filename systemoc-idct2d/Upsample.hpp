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

#ifndef _INCLUDED_UPSAMPLE_HPP
#define _INCLUDED_UPSAMPLE_HPP

#include "callib.hpp"

//#define VERBOSE_IDCT_UPSAMPLE

class m_Upsample: public smoc_actor {
public:
  smoc_port_in<int>  I;
  smoc_port_out<int> O;
private:
  int  mem;
  
  void upsampleStart() { 
#ifdef VERBOSE_IDCT_UPSAMPLE
#ifndef NDEBUG
#ifndef XILINX_EDK_RUNTIME
		cout << name() << ": " << "I[0] = " << I[0] << endl;
		cout << name() << ": " << "O[0] = " << mem << endl;
#else
		xil_printf("%s: I[0] = %d\r\n",name(),I[0]);
		xil_printf("%s: O[0] = %d\r\n",name(),mem);
#endif
#endif
#endif
		O[0] = mem = I[0]; 
	}
  void upsampleRest()  { 
#ifdef VERBOSE_IDCT_UPSAMPLE
#ifndef NDEBUG
#ifndef XILINX_EDK_RUNTIME
		cout << name() << ": " << "O[0] = " << mem << endl;
#else
		xil_printf("%s: O[0] = %d\r\n",name(),mem);
#endif
#endif
#endif
		O[0] = mem;        
	}
  
  smoc_firing_state s0, s1, s2, s3, s4, s5, s6, s7;
public:
  m_Upsample(sc_module_name name): smoc_actor(name, s0) {
    s0 = I(1) >> O(1) >> CALL(m_Upsample::upsampleStart) >> s1;
    s1 =         O(1) >> CALL(m_Upsample::upsampleRest)  >> s2;
    s2 =         O(1) >> CALL(m_Upsample::upsampleRest)  >> s3;
    s3 =         O(1) >> CALL(m_Upsample::upsampleRest)  >> s4;
    s4 =         O(1) >> CALL(m_Upsample::upsampleRest)  >> s5;
    s5 =         O(1) >> CALL(m_Upsample::upsampleRest)  >> s6;
    s6 =         O(1) >> CALL(m_Upsample::upsampleRest)  >> s7;
    s7 =         O(1) >> CALL(m_Upsample::upsampleRest)  >> s0;
  }
};

#endif // _INCLUDED_UPSAMPLE_HPP
