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

#ifndef _INCLUDED_IDCTBLOCKSINK_HPP
#define _INCLUDED_IDCTBLOCKSINK_HPP

#ifdef KASCPAR_PARSING
# define USE_COUNTER_INPUT
typedef unsigned long size_t;
#endif

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>
#include "callib.hpp"

class m_block_sink: public smoc_actor {
public:
  smoc_port_in<int> in;
private:

  const size_t image_width;
  const size_t image_height;

  unsigned long block_count;
  const unsigned long block_nbr;

  void new_image() {
    cout << "P2 " 
         << image_width << " "
         << image_height<< " "
         << 255 
         << endl;
    process();
  }
  
  void process() {
    //output a complete block line
    for(unsigned int y = 0; y < 8; y++){
      for(unsigned int x = 0; x < image_width; x++){      
        unsigned int bx = x / 8;
        unsigned int rx = x % 8;
        // +128 : DC-Level shift
        std::cout << in[bx*64+y*8+rx] + 128 << " ";
      }
      std::cout << std::endl;
    }
    block_count += image_width / 8;
    if (block_count >= block_nbr)
      block_count = 0;
  }
  
  smoc_firing_state start;
public:
  m_block_sink( sc_module_name name,
                SMOC_ACTOR_CPARAM(size_t, image_width),
                SMOC_ACTOR_CPARAM(size_t, image_height)
                )
    : smoc_actor( name, start ),
      image_width(image_width),
      image_height(image_height),
      block_count(0),
      //Only support complete blocks
      block_nbr(image_width/8*image_height/8)
  {
    // Read a complete line as once
    start = in(64*(image_width/8)) 
      >> (VAR(block_count) != (unsigned)0)
      >> CALL(m_block_sink::process)  >> start
      | in(64*(image_width/8)) 
      >> (VAR(block_count) == (unsigned)0)
      >> CALL(m_block_sink::new_image)  >> start;
  }
  
  ~m_block_sink() {
  }
};

#endif
