//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
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

#ifndef _INCLUDED_INV_ZIGZAG_HPP
#define _INCLUDED_INV_ZIGZAG_HPP

#ifdef KASCPAR_PARSING
# define NDEBUG
#endif

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

/// This actor takes the IDCT coefficients arriving in zig-zag order
/// and outputs them in a line based manner.
class InvZigZag: public smoc_actor {
public:
  smoc_port_in<JpegChannel_t> in;
  smoc_port_out<IDCTCoeff_t>  out;
private:

#ifndef KASCPAR_PARSING
  static
#endif // KASCPAR_PARSING
  const unsigned char zigzag_order[JPEG_BLOCK_SIZE];

  unsigned int block_pixel_id;

  void forward_pixel(){
    // Check, that no control words occur any more
    // Otherwise we produce wrong data!
    assert(!JS_ISCTRL(in[zigzag_order[block_pixel_id]]));
    out[0] = JS_COEFF_GETIDCTCOEFF(in[zigzag_order[block_pixel_id]]);
    block_pixel_id++;
    if (block_pixel_id >= JPEG_BLOCK_SIZE){
      block_pixel_id = 0;
    }
  }

  smoc_firing_state main;
public:
  InvZigZag(sc_module_name name)
    : smoc_actor(name, main),
      block_pixel_id(0)
  {
    main
      // ignore and forward control tokens
      = ( in(0,JPEG_BLOCK_SIZE) && out(1))       >>
      (VAR(block_pixel_id) != (unsigned int)JPEG_BLOCK_SIZE-1) >>
      CALL(InvZigZag::forward_pixel)             >> main
      | ( in(JPEG_BLOCK_SIZE) && out(1))         >>
      (VAR(block_pixel_id) == (unsigned int)JPEG_BLOCK_SIZE-1) >>
      CALL(InvZigZag::forward_pixel)             >> main;
  }
};

const unsigned char InvZigZag::zigzag_order[JPEG_BLOCK_SIZE] =
  {  0, 1, 5, 6,14,15,27,28,
     2, 4, 7,13,16,26,29,42,
     3, 8,12,17,25,30,41,43,
     9,11,18,24,31,40,44,53,
    10,19,23,32,39,45,52,54,
    20,22,33,38,46,51,55,60,
    21,34,37,47,50,56,59,61,
    35,36,48,49,57,58,62,63
  };


#endif // _INCLUDED_INV_ZIGZAG_HPP
