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

#include "debug_config.h"
#include <cosupport/smoc_debug_out.hpp>
// if compiled with DBG_PGMSINK create stream and include debug macros
#ifdef DBG_PGMSINK
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif

#ifdef PERFORMANCE_EVALUATION
# ifdef XILINX_EDK_RUNTIME
#  include "x_perf_eval.h"
# else
#  include <cosupport/PerformanceEvaluation.hpp>
# endif
#endif // PERFORMANCE_EVALUATION

class m_pgm_sink: public smoc_actor {
public:
  smoc_port_in<Pixel_t> in;
  smoc_port_in<JpegChannel_t> inCtrlImage;
private:

  //number of components
  IntCompID_t compCount; 

  unsigned int image_width;
  unsigned int image_height;
  unsigned int missing_pixels;

  void processNewFrame() {
    DBG_OUT("processNewFrame()\n");
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
    DBG_OUT("write_image_header()\n");
#ifndef KASCPAR_PARSING

#ifndef PGM_SINK_SILENT_MODE
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
      std::cout << "P5 ";
    else
      std::cout << "P6 ";

    std::cout << image_width << " "
         << image_height<< " "
         << 255 
         << std::endl;
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
      std::cout << "P2 " ;
    else
      std::cout << "P3 " ;
    
    std::cout << image_width << " "
         << image_height<< " "
         << 255 
         << std::endl;
# endif
#endif
#endif // PGM_SINK_SILENT_MODE
#endif // KASCPAR_PARSING
  }
  
  void process_pixel() {
    DBG_OUT("process_pixel()\n");
    Pixel_t _in = in[0];
#ifndef KASCPAR_PARSING
# ifndef PGM_SINK_SILENT_MODE
    for (unsigned int c = 0; c < compCount; c++){      
#  ifdef SINK_BINARY_OUTPUT
#   ifndef XILINX_EDK_RUNTIME
      std::cout << (char)(JS_RAWPIXEL_GETCOMP(_in,c));
#   else
      xil_printf("%c", JS_RAWPIXEL_GETCOMP(_in,c));
#   endif
#  else
#   ifndef XILINX_EDK_RUNTIME
      std::cout << (int)(JS_RAWPIXEL_GETCOMP(_in,c)) << " ";
#   else
      xil_printf("%d ", JS_RAWPIXEL_GETCOMP(_in,c));
#   endif
#  endif
    }
# endif //PGM_SINK_SILENT_MODE


    missing_pixels--;
# ifndef SINK_BINARY_OUTPUT
    if (missing_pixels % image_width == 0){
#  ifndef PGM_SINK_SILENT_MODE
#   ifndef XILINX_EDK_RUNTIME
      std::cout << std::endl;
#   else
      xil_printf("\n");
#   endif
#  endif //PGM_SINK_SILENT_MODE
    }
# endif //SINK_BINARY_OUTPUT

    if (missing_pixels == 0){
#ifdef SC_MODULE // is "systemc.h" included?
      std::cerr << "image finished @" << sc_time_stamp() << std::endl;
#endif
# ifdef PERFORMANCE_EVALUATION
#  ifdef XILINX_EDK_RUNTIME
      x_perf_eval_toggle_end();
#  else
      PerformanceEvaluation::getInstance().stopUnit();
#  endif //XILINX_EDK_RUNTIME
# endif // PERFORMANCE_EVALUATION

# ifndef PGM_SINK_SILENT_MODE
#  ifndef SINK_BINARY_OUTPUT

#   ifndef XILINX_EDK_RUNTIME
      std::cout << std::endl;
#   else
      xil_printf("\n");
#   endif
#  endif //SINK_BINARY_OUTPUT
# endif //PGM_SINK_SILENT_MODE
    }
#endif //KASCPAR_PARSING    
  }
  
// ############################################################################
// # Debug
// ############################################################################
#ifdef DBG_ENABLE
  CoSupport::DebugOstream dbgout;
#endif // DBG_ENABLE

  smoc_firing_state newFrame;
  smoc_firing_state main;
  

public:
  m_pgm_sink( sc_module_name name )
    : smoc_actor( name, newFrame ),
      compCount(0),
      image_width(0),
      image_height(0),
      missing_pixels(0)
#ifdef DBG_ENABLE
      , dbgout(std::cerr)
#endif // DBG_ENABLE
  {
#ifdef DBG_ENABLE
    // Debug
    CoSupport::Header myHeader("PGMsink> ");
    dbgout << myHeader;
#endif // DBG_ENABLE

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
