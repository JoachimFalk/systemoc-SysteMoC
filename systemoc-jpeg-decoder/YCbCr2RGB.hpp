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

#ifndef _INCLUDED_YCRCB2RGB_HPP
#define _INCLUDED_YCRCB2RGB_HPP

#include <cstdlib>
#include <iostream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "smoc_synth_std_includes.hpp"

#include "debug_config.h"
#include <cosupport/smoc_debug_out.hpp>
// if compiled with DBG_YCBCR2RGB create stream and include debug macros
#ifdef DBG_YCBCR2RGB
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif


class YCrCb2RGB: public smoc_actor {
public:
  smoc_port_in<Pixel_t>      in;
  smoc_port_out<Pixel_t>     out;

private:

  void transform_color(){
#if FAST
    CYN_LATENCY(0, 3, "jpeg_mYCbCr::transform_color");
#endif

    const unsigned int nbr_fractional_digits = 5;

    int pixel_in[3];
    Pixel_t tmp = in[0];
    pixel_in[0] = JS_RAWPIXEL_GETCOMP(tmp,0);       //Y
    pixel_in[1] = JS_RAWPIXEL_GETCOMP(tmp,1) - 128; //Cb
    pixel_in[2] = JS_RAWPIXEL_GETCOMP(tmp,2) - 128; //Cr

    DBG_OUT("Input pixel values: " 
	    << pixel_in[0] << " " 
	    << " " << pixel_in[1] + 128
	    << " " << pixel_in[2] + 128
	    << std::endl);

#ifndef KASCPAR_PARSING
    const int matrix[3][3] = {
      {
	CALC_INT_FIXPOINT(1,nbr_fractional_digits), 
	CALC_INT_FIXPOINT(0,nbr_fractional_digits), 
	CALC_INT_FIXPOINT(1.402,nbr_fractional_digits), 
      },
      {
	CALC_INT_FIXPOINT(1,nbr_fractional_digits), 
	CALC_INT_FIXPOINT(-0.34414,nbr_fractional_digits), 
	CALC_INT_FIXPOINT(-0.71414,nbr_fractional_digits), 
      },
      {
	CALC_INT_FIXPOINT(1,nbr_fractional_digits), 
	CALC_INT_FIXPOINT(1.772,nbr_fractional_digits), 
	CALC_INT_FIXPOINT(0,nbr_fractional_digits), 
      }
    };
#endif

    //check, that lines and columns are not flipped.
    //line 2, column 0
    assert(matrix[2][0] == CALC_INT_FIXPOINT(1,nbr_fractional_digits));

    int pixel_out[3] = {0,0,0}; 

    for(unsigned int i = 0; i < 3; i++){
      CYN_UNROLL(ALL, "matrix multiplication");
      for(unsigned int j = 0; j < 3; j++){
	pixel_out[i] += matrix[i][j] * pixel_in[j];
      }

      SHIFT_AND_ROUND(pixel_out[i], nbr_fractional_digits);
      if (pixel_out[i] >= 256){
	pixel_out[i] = 255;
      }else if (pixel_out[i] < 0){
	pixel_out[i] = 0;
      }
    }    

    out[0] = JS_RAWPIXEL_SETVAL(pixel_out[0],pixel_out[1],pixel_out[2]);

    DBG_OUT("Output pixel values: " 
	    << pixel_out[0] << " " 
	    << pixel_out[1] << " " 
	    << pixel_out[2] 
	    << std::endl);
      
  }

  smoc_firing_state main;

#ifdef DBG_ENABLE
  CoSupport::DebugOstream dbgout;
#endif // DBG_ENABLE

  
public:
  YCrCb2RGB(sc_module_name name)
    : smoc_actor(name, main)
#ifdef DBG_ENABLE
    , dbgout(std::cerr)
#endif // DBG_ENABLE
  {
#ifdef DBG_ENABLE
    //Set Debug ostream options
    CoSupport::Header my_header("YCbCr2RGB> ");
    dbgout << my_header;
#endif // DBG_ENABLE
    
    
    main =
      /* discard Huffman tables */
      ( in(1) && out(1)) >>
      CALL(YCrCb2RGB::transform_color) >> main;
    
  }
};

#endif // _INCLUDED_YCRCB2RGB_HPP
