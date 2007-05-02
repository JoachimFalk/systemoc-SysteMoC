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

#ifdef KASCPAR_PARSING
# define NDEBUG
#endif

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <vector>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

class FrameShuffler: public smoc_actor {
public:
  smoc_port_in<JpegChannel_t> in;
  smoc_port_out<Pixel_t>      out;
  smoc_port_in<JpegChannel_t> inCtrlImage;
protected:
  struct Pos {
    FrameDimX_t x;
    FrameDimY_t y;
  };

  FrameDimX_t dimX;
  FrameDimY_t dimY;
  IntCompID_t compCount;

  // line count in shuffle
  const int   shuffleLines;
  // on which shuffle line am I
  int         shufflePosY;
  // pixel position in frame
  FrameDimX_t posX;
  FrameDimY_t posY;

  void processFrameDesc() {
#ifndef KASCPAR_PARSING
    std::cerr << "FrameShuffler::processFrameDesc";
#endif // KASCPAR_PARSING
    
    assert(JS_ISCTRL(inCtrlImage[0]));
    assert(JS_GETCTRLCMD(inCtrlImage[0]) == CTRLCMD_NEWFRAME);
    
    assert(dimX == JS_CTRL_NEWFRAME_GET_DIMX(inCtrlImage[0]));
    assert(dimY == JS_CTRL_NEWFRAME_GET_DIMY(inCtrlImage[0]));
    posX = posY = shufflePosY = 0;
    
    assert(compCount  == JS_CTRL_NEWFRAME_GET_COMPCOUNT(inCtrlImage[0]));
    // Only support color and grayscale output
    assert(compCount == 1 || compCount == 3);
    
#ifndef KASCPAR_PARSING
    std::cerr << " width: " << dimX << " height: " << dimY
              << " component count: " << static_cast<unsigned int>(compCount) << std::endl;
#endif // KASCPAR_PARSING
  }

  void processScanDesc() {
#ifndef KASCPAR_PARSING
    std::cerr << "FrameShuffler::processScanDesc ";
#endif // KASCPAR_PARSING
    
    assert(JS_ISCTRL(inCtrlImage[0]));
    assert(JS_GETCTRLCMD(inCtrlImage[0]) == CTRLCMD_NEWSCAN);
    
#ifndef KASCPAR_PARSING
    for (int i = 0; i < SCANPATTERN_LENGTH; ++i) {
      // component contained in scan must be filled into frame => start at pos 0, 0
      std::cerr
        << static_cast<unsigned int>(JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], i));
    }
    std::cerr << std::endl;
#endif // KASCPAR_PARSING
    
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
    std::cerr << "FrameShuffler::writeComponent for (" << posX << ", " << posY << ")" << std::endl;
    assert(!JS_ISCTRL(in[0]));

    size_t posInBlock = (posX &  7U) + ((posY & 7U) << 3);
    size_t posOfBlock = ((posX & ~7U) << 3);
    if (compCount == 1) {
      std::cerr << "@" << (posInBlock | posOfBlock) << std::endl;

      out[0] = JS_RAWPIXEL_SETVAL(
        JS_COMPONENT_GETVAL(in[posInBlock | posOfBlock]),
        128,
        128);
    } else {
      out[0] = JS_RAWPIXEL_SETVAL(
        JS_COMPONENT_GETVAL(in[posInBlock | (posOfBlock*3 + (0<<6))]),
        JS_COMPONENT_GETVAL(in[posInBlock | (posOfBlock*3 + (1<<6))]),
        JS_COMPONENT_GETVAL(in[posInBlock | (posOfBlock*3 + (2<<6))]));
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
  FrameShuffler(sc_module_name name, size_t dimX, size_t dimY, size_t comp)
    : smoc_actor(name, getFrameDesc),
      dimX(dimX), dimY(dimY), compCount(comp), shuffleLines(8)
  {
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
