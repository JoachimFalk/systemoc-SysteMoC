//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#ifndef _INCLUDED_IDCTSOURCE_HPP
#define _INCLUDED_IDCTSOURCE_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <smoc_port.hpp>

#include "smoc_synth_std_includes.hpp"

class m_source_idct: public smoc_actor {
public:
  smoc_port_out<int> out;
  smoc_port_out<int> min;
private:
  size_t counter;
#ifndef USE_COUNTER_INPUT
  std::ifstream i1; 
#endif
  
  void process() {
    int myMin;
    int myOut;
    
#ifndef USE_COUNTER_INPUT
    if (i1.good()) {
#endif
      for ( int j = 0; j <= 63; j++ ) {
#ifdef USE_COUNTER_INPUT
        myOut = counter;
#else
        i1 >> myOut;
        cout << name() << "  write " << myOut << std::endl;
#endif
        out[j] = myOut;
				counter++;
      }
      myMin = -256;
#ifndef USE_COUNTER_INPUT
      cout << name() << "  write min " << myMin << std::endl;
#endif
      min[0] = myMin;
#ifndef USE_COUNTER_INPUT
    } else {
      cout << "File empty! Please create a file with name test_in.dat!" << std::endl;
      exit (1) ;
    }
#endif
  }
 
  smoc_firing_state start;
public:
  m_source_idct(sc_module_name name,
      SMOC_ACTOR_CPARAM(size_t, periods))
    : smoc_actor(name, start), counter(0) {
#ifndef USE_COUNTER_INPUT
    i1.open(INAMEblk);
#endif
    start = (out(64) && min(1) && VAR(counter) < periods * 64)  >>
            CALL(m_source_idct::process)                        >> start;
  }
  ~m_source_idct() {
#ifndef USE_COUNTER_INPUT
    i1.close();
#endif
  }
};


#endif
