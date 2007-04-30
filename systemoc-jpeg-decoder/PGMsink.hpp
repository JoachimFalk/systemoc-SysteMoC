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

#ifndef _INCLUDED_PGM_SINK_HPP
#define _INCLUDED_PGM_SINK_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>

//#include "smoc_synth_std_includes.hpp"


#ifdef SINK_BINARY_OUTPUT
# error
#endif

class m_pgm_sink: public smoc_actor {
public:
  smoc_port_in<Pixel_t> in;
  smoc_port_in<JpegChannel_t> inCtrlImage;
private:

  //number of components
  IntCompID_t compCount; 

  size_t image_width;
  size_t image_height;
  size_t missing_pixels;

  void processNewFrame() {
    assert(JS_ISCTRL(inCtrlImage[0]));
    assert(JS_GETCTRLCMD(inCtrlImage[0]) == CTRLCMD_NEWFRAME);
    
    image_width  = JS_CTRL_NEWFRAME_GET_DIMX(inCtrlImage[0]);
    image_height = JS_CTRL_NEWFRAME_GET_DIMY(inCtrlImage[0]);
    compCount   = JS_CTRL_NEWFRAME_GET_COMPCOUNT(inCtrlImage[0]);
    
    // Only support color and grayscale output
    if (compCount > 1 && compCount < 3)
      compCount = 3;

    write_image_header();
    missing_pixels = image_width * image_height;
  }

  void write_image_header() {
#ifdef SINK_BINARY_OUTPUT
# ifdef XILINX_EDK_RUNTIME
    if (compCount == 1)
      xil_printf("P5 ");
    else
      xil_printf("P6 ");

    xil_printf("%d %d 255\n",
               image_width,
               image_height);
# else
    if (compCount == 1)
      cout << "P5 ";
    else
      cout << "P6 ";

    cout << image_width << " "
         << image_height<< " "
         << 255 
         << endl;
# endif
#else
# ifdef XILINX_EDK_RUNTIME
    if (compCount == 1)
      xil_printf("P2 ");
    else
      xil_printf("P3 ");
    
    xil_printf("%d %d 255\n",
               image_width,
               image_height);
# else
    if (compCount == 1)
      cout << "P2 " ;
    else
      cout << "P3 " ;
    
    cout << image_width << " "
         << image_height<< " "
         << 255 
         << endl;
# endif
#endif
  }
  
  void process_pixel() {
    for (unsigned int c = 0; c < compCount; c++){      
#ifdef SINK_BINARY_OUTPUT
# ifndef XILINX_EDK_RUNTIME
      cout << (char)(JS_RAWPIXEL_GETCOMP(in[0],c));
# else
      xil_printf("%c", JS_RAWPIXEL_GETCOMP(in[0],c));
# endif
#else
# ifndef XILINX_EDK_RUNTIME
      cout << (int)(JS_RAWPIXEL_GETCOMP(in[0],c)) << " ";
# else
      xil_printf("%d ", JS_RAWPIXEL_GETCOMP(in[0],c));
# endif
#endif
    }

    missing_pixels--;
#ifndef SINK_BINARY_OUTPUT
    if (missing_pixels % image_width == 0){
# ifndef XILINX_EDK_RUNTIME
      cout << endl;
# else
      xil_printf("\n");
# endif
    }

    if (missing_pixels == 0){
# ifndef XILINX_EDK_RUNTIME
      cout << endl;
# else
      xil_printf("\n");
# endif
    }
#endif
      
  }
  

  smoc_firing_state newFrame;
  smoc_firing_state main;
  

public:
  m_pgm_sink( sc_module_name name )
    : smoc_actor( name, newFrame ),
      compCount(0),
      image_width(0),
      image_height(0),
      missing_pixels(0)
  {

    newFrame
      = inCtrlImage(1) 
      >> (JS_GETCTRLCMD(inCtrlImage.getValueAt(0)) == (JpegChannel_t)CTRLCMD_NEWFRAME)  
      >>  CALL(m_pgm_sink::processNewFrame)    
      >> main
      | inCtrlImage(1)
      >> (JS_GETCTRLCMD(inCtrlImage.getValueAt(0)) != (JpegChannel_t)CTRLCMD_NEWFRAME)  
      >> newFrame;
	
    main = in(1) 
      >> (VAR(missing_pixels) != (unsigned)1)
      >> CALL(m_pgm_sink::process_pixel)  >> main
      | in(1)
      >> (VAR(missing_pixels) == (unsigned)1)
      >> CALL(m_pgm_sink::process_pixel)  >> newFrame;
  }
  
  ~m_pgm_sink() {
  }
};

#endif
