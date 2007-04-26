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

#ifndef _INCLUDED_IDCTBLOCKSOURCE_HPP
#define _INCLUDED_IDCTBLOCKSOURCE_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>

#include "smoc_synth_std_includes.hpp"

class m_block_source_idct: public smoc_actor {
public:
  smoc_port_out<int> out;
  smoc_port_out<int> min;
private:
  size_t counter;
  size_t counter2; 

  
  void process() {

#ifndef KASCPAR_PARSING    
    const static int block_data[] = {
# include "Y_IdctCoeff.txt"
    };
    const static unsigned long block_data_size =   
      sizeof(block_data)/sizeof(block_data[0]);
#endif

    int myMin;

    int j = 0;

    while(j <= 63){
      if (block_data[counter] == 0){
        counter++;
        for(int i = 0; i < block_data[counter]; i++){
          out[j] = 0;
          j++;
          counter2++;
        }
      }else{
        out[j] = block_data[counter];        
        counter2++;
        j++;
      }
      counter++;
    }
    
    if (counter >= block_data_size){
      counter = 0;
    }

    myMin = -128;
    min[0] = myMin;
  }
 
  smoc_firing_state start;
public:
  m_block_source_idct(sc_module_name name,
      size_t periods)
    : smoc_actor(name, start), counter(0), counter2(0) {
    SMOC_REGISTER_CPARAM(periods);
    start = (out(64) && min(1) && VAR(counter2) < periods * 64)  >>
      CALL(m_block_source_idct::process)                        >> start;
  }

  ~m_block_source_idct() {
  }
};

#endif
