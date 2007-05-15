//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:
/*
 * Copyright (c) 2007 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_FRAME_BUFFER_WRITER_HPP
#define _INCLUDED_FRAME_BUFFER_WRITER_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <vector>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

#include "debug_config.h"
#include <cosupport/smoc_debug_out.hpp>
// if compiled with DBG_SHUFFLER create stream and include debug macros
#ifdef DBG_SHUFFLER
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif

class FrameShuffler: public smoc_actor {
public:
  smoc_port_in<IDCTCoeff_t>     in;
  smoc_port_out<Pixel_t>        out;
  smoc_port_in<JpegChannel_t>   inCtrlImage;
protected:
  struct Pos {
    FrameDimX_t x;
    FrameDimY_t y;
  };

  FrameDimX_t dimX;
  FrameDimY_t dimY;
  IntCompID_t compCount;

#ifdef DBG_ENABLE
  CoSupport::DebugOstream dbgout;
#endif // DBG_ENABLE

  // line count in shuffle
  const int   shuffleLines;
  // on which shuffle line am I
  int         shufflePosY;
  // pixel position in frame
  FrameDimX_t posX;
  FrameDimY_t posY;

  void processFrameDesc() {
    DBG_OUT("processFrameDesc()");
    
    assert(JS_ISCTRL(inCtrlImage[0]));
    assert(JS_GETCTRLCMD(inCtrlImage[0]) == CTRLCMD_NEWFRAME);
    
    assert(dimX == JS_CTRL_NEWFRAME_GET_DIMX(inCtrlImage[0]));
    assert(dimY == JS_CTRL_NEWFRAME_GET_DIMY(inCtrlImage[0]));
    posX = posY = shufflePosY = 0;
    
    assert(compCount  == JS_CTRL_NEWFRAME_GET_COMPCOUNT(inCtrlImage[0]));
    // Only support color and grayscale output
    assert(compCount == 1 || compCount == 3);
    
    DBG_OUT(" width: " << dimX << " height: " << dimY
              << " component count: " << static_cast<unsigned int>(compCount) << std::endl);
  }

  void processScanDesc() {
    DBG_OUT("processScanDesc()");
    
    assert(JS_ISCTRL(inCtrlImage[0]));
    assert(JS_GETCTRLCMD(inCtrlImage[0]) == CTRLCMD_NEWSCAN);
    
    for (int i = 0; i < SCANPATTERN_LENGTH; ++i) {
      // component contained in scan must be filled into frame => start at pos 0, 0
      DBG_OUT(static_cast<unsigned int>(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], i)));
    }
    DBG_OUT(std::endl);
    
    assert(compCount == 1 || compCount == 3);
    if (compCount == 3) {
      assert(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], 0) == 0);
      assert(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], 1) == 1);
      assert(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], 2) == 2);
      assert(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], 3) == 0);
      assert(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], 4) == 1);
      assert(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], 5) == 2);
    } else {
      assert(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], 0) == 0);
      assert(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], 1) == 0);
      assert(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], 2) == 0);
      assert(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], 3) == 0);
      assert(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], 4) == 0);
      assert(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], 5) == 0);
    }
  }

  void writeComponent() {
    DBG_OUT("writeComponent for (" << posX << ", " << posY << ")" << std::endl);
    
    unsigned int posInBlock = (posX &  7U) + ((posY & 7U) << 3);
    unsigned int posOfBlock = ((posX & ~7U) << 3);
    if (compCount == 1) {
      DBG_OUT("@" << (posInBlock | posOfBlock) << std::endl);

      out[0] = JS_RAWPIXEL_SETVAL(
        in[posInBlock | posOfBlock],
        128,
        128);
    } else {
      out[0] = JS_RAWPIXEL_SETVAL(
        in[posInBlock | (posOfBlock*3 + (0<<6))],
        in[posInBlock | (posOfBlock*3 + (1<<6))],
        in[posInBlock | (posOfBlock*3 + (2<<6))]);
    }

    if (posX == dimX - 1) {
      posX = 0;
      if (shufflePosY == shuffleLines - 1) {
        shufflePosY = 0;
      } else
        ++shufflePosY;
      if (posY == dimY - 1) {
        posY = 0;
      } else
        ++posY;
    } else
      ++posX;
  }

  smoc_firing_state getFrameDesc;
  smoc_firing_state getScanDescs;
  smoc_firing_state readScan;
public:
  FrameShuffler(sc_module_name name, unsigned int dimX, unsigned int dimY, unsigned int comp)
    : smoc_actor(name, getFrameDesc),
      dimX(dimX), dimY(dimY), compCount(comp), shuffleLines(8)
#ifdef DBG_ENABLE
    , dbgout(std::cerr)
#endif // DBG_ENABLE
  {
    SMOC_REGISTER_CPARAM(dimX);
    SMOC_REGISTER_CPARAM(dimY);
    SMOC_REGISTER_CPARAM(comp);

#ifdef DBG_ENABLE
    //Set Debug ostream options
    CoSupport::Header my_header("FrameShuffler> ");
    dbgout << my_header;
#endif // DBG_ENABLE

    getFrameDesc
      // this must be a CTRLCMD_NEWFRAME
      = inCtrlImage(1)                          >>
        CALL(FrameShuffler::processFrameDesc)   >> getScanDescs
      ;
    getScanDescs
      = // this must be a CTRLCMD_NEWSCAN
        inCtrlImage(1)                          >>
        CALL(FrameShuffler::processScanDesc)    >> readScan
      ;
    readScan
      =((VAR(shufflePosY) != shuffleLines - 1 ||
         VAR(posX)        != dimX - 1) &&
        in(0, comp * dimX * shuffleLines))      >>
        out(1)                                  >>
        CALL(FrameShuffler::writeComponent)     >> readScan
      |((VAR(shufflePosY) == shuffleLines - 1 &&
         VAR(posY)        != dimY - 1 &&
         VAR(posX)        == dimX - 1) &&
        in(comp * dimX * shuffleLines))         >>
        out(1)                                  >>
        CALL(FrameShuffler::writeComponent)     >> readScan
      |((VAR(shufflePosY) == shuffleLines - 1 &&
         VAR(posY)        == dimY - 1) &&
        in(comp * dimX * shuffleLines))         >>
        out(1)                                  >>
        CALL(FrameShuffler::writeComponent)     >> getFrameDesc
      ;
  }
};

#endif // _INCLUDED_FRAME_BUFFER_WRITER_HPP
