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

#ifndef KASCPAR_PARSING
  typedef std::vector<ComponentVal_t> FrameBuffer;
#else
# define FrameBuffer vector<ComponentVal_t>
#endif // KASCPAR_PARSING

  Pos         frameDim;
  IntCompID_t compCount;
  IntCompID_t compMissing;

  Pos         compPos[JPEG_MAX_COLOR_COMPONENTS];
  IntCompID_t scanPattern[SCANPATTERN_LENGTH];

  // pixel count in shuffle
  int         shuffleInterval;
  // how many pixels left to complete shuffle
  int         shuffleMissing;
  // how many pixels left till frame completion
  int         frameMissing;

  // index into scanPattern
  int         scanIndex;

  // index into 8x8 block
  int         blockIndex;

  // Is of size width*height*compCount
  FrameBuffer frameBuffer;

  void processFrameDesc() {
#ifndef KASCPAR_PARSING
    std::cerr << "FrameShuffler::processFrameDesc";
#endif // KASCPAR_PARSING

    assert(JS_ISCTRL(inCtrlImage[0]));
    assert(JS_GETCTRLCMD(inCtrlImage[0]) == CTRLCMD_NEWFRAME);
    
    frameDim.x = JS_CTRL_NEWFRAME_GET_DIMX(inCtrlImage[0]);
    frameDim.y = JS_CTRL_NEWFRAME_GET_DIMY(inCtrlImage[0]);
    compCount  = JS_CTRL_NEWFRAME_GET_COMPCOUNT(inCtrlImage[0]);
    
#ifndef KASCPAR_PARSING
    std::cerr << " width: " << frameDim.x << " height: " << frameDim.y
              << " component count: " << static_cast<unsigned int>(compCount) << std::endl;
#endif // KASCPAR_PARSING
    
    compMissing = compCount;
    
    // Only support color and grayscale output
    if (compCount > 1 && compCount < 3)
      compCount = 3;
    
    frameBuffer.resize(frameDim.x * frameDim.y * compCount);
  }

  void dumpFrame() {
    size_t index = 0;
    
#ifndef KASCPAR_PARSING
    std::cerr << "FrameShuffler::dumpFrame" << std::endl;
    assert(compCount == 1 || compCount == 3);
    
    if (compCount == 1)
      std::cout << "P2 " << frameDim.x << " " << frameDim.y << " 255" << std::endl;
    else
      std::cout << "P3 " << frameDim.x << " " << frameDim.y << " 255" << std::endl;
    //output a complete block line
    for (FrameBuffer::const_iterator iter = frameBuffer.begin();
         iter != frameBuffer.end();
         ++iter) {
      std::cout << static_cast<unsigned int>(*iter);
      if (++index % 20 == 0)
        std::cout << std::endl;
      else
        std::cout << " ";
    }
    std::cout << std::endl << std::endl;
#endif // KASCPAR_PARSING
  }

  void processScanDesc() {
#ifndef KASCPAR_PARSING
    std::cerr << "FrameShuffler::processScanDesc ";
#endif // KASCPAR_PARSING
    
    assert(JS_ISCTRL(inCtrlImage[0]));
    assert(JS_GETCTRLCMD(inCtrlImage[0]) == CTRLCMD_NEWSCAN);
    
    // Mark all possible components as already done
    for (int i = 0; i < JPEG_MAX_COLOR_COMPONENTS; ++i) {
      compPos[i].x = 0;
      compPos[i].y = frameDim.y;
    }
    for (int i = 0; i < SCANPATTERN_LENGTH; ++i) {
      scanPattern[i] = JS_CTRL_NEWSCAN_GETCOMP(inCtrlImage[0], i);
      // component contained in scan must be filled into frame => start at pos 0, 0
      if (compPos[scanPattern[i]].y)
        // found new component in scan => decrement missing component count
        --compMissing;
      compPos[scanPattern[i]].x = 0;
      compPos[scanPattern[i]].y = 0;
#ifndef KASCPAR_PARSING
      std::cerr
        << static_cast<unsigned int>(scanPattern[i]);
//      << (i < SCANPATTERN_LENGTH-1 ? ":" : "");
#endif // KASCPAR_PARSING
    }
#ifndef KASCPAR_PARSING
    std::cerr << std::endl;
#endif // KASCPAR_PARSING
    scanIndex = blockIndex = 0;
  }

  void writeComponent() {
    std::cerr << "FrameShuffler::writeComponent" << std::endl;
    assert(!JS_ISCTRL(in[0]));


    if (--shuffleMissing == 0)
      shuffleMissing = shuffleInterval;
    --frameMissing;

    out[0] = 0xFFFFFF;

/*
    assert(scanPattern[scanIndex] < compCount);
    frameBuffer[compCount * (
       (compPos[scanPattern[scanIndex]].y + blockIndex / JPEG_BLOCK_WIDTH) * frameDim.x +
        compPos[scanPattern[scanIndex]].x + blockIndex % JPEG_BLOCK_WIDTH
      ) + scanPattern[scanIndex]] = JS_COMPONENT_GETVAL(in[0]);
    
    blockIndex = (blockIndex + 1) % JPEG_BLOCK_SIZE;
    if (blockIndex == 0) {
      compPos[scanPattern[scanIndex]].x += JPEG_BLOCK_WIDTH;
      if (compPos[scanPattern[scanIndex]].x >= frameDim.x) {
        compPos[scanPattern[scanIndex]].x = 0;
        compPos[scanPattern[scanIndex]].y += JPEG_BLOCK_HEIGHT;
      }
      scanIndex = (scanIndex + 1) % SCANPATTERN_LENGTH;
    }
 */
  }

  bool frameEnd() const {
    assert(/*compMissing >= 0 &&*/ compMissing <= compCount);
    std::cerr << "FrameShuffler::frameEnd" << std::endl;
    return compMissing == 0;
  }

  bool scanEnd() const {
    std::cerr << "FrameShuffler::scanEnd" << std::endl;
#ifndef NDEBUG
    for (int i = 0; i < JPEG_MAX_COLOR_COMPONENTS; ++i) {
      assert(/*compPos[i].x >= 0 &&*/ compPos[i].x <  frameDim.x);
      assert(/*compPos[i].y >= 0 &&*/ compPos[i].y <= frameDim.y);
    }
#endif
    for (int i = 0; i < JPEG_MAX_COLOR_COMPONENTS; ++i) {
//    std::cerr << compPos[i].y << ", ";
      if (compPos[i].y != frameDim.y) {
//      std::cerr << std::endl;
        return false;
      }
    }
//  std::cerr << std::endl;
    return true;
  }

  smoc_firing_state getFrameDesc;
  smoc_firing_state getScanDescs;
  smoc_firing_state readScans;
public:
  FrameShuffler(sc_module_name name)
    : smoc_actor(name, getFrameDesc),
      shuffleInterval(1),
      shuffleMissing(1),
      frameMissing(16)
  {
    getFrameDesc
      // this must be a CTRLCMD_NEWFRAME
      = inCtrlImage(1)                              >>
        CALL(FrameShuffler::processFrameDesc)   >> getScanDescs
      ;
    getScanDescs
      =(!GUARD(FrameShuffler::frameEnd) &&
      // this must be a CTRLCMD_NEWSCAN
        inCtrlImage(1))                             >>
        CALL(FrameShuffler::processScanDesc)    >> getScanDescs
      |  GUARD(FrameShuffler::frameEnd)         >> readScans
      ;
    readScans
      =(VAR(shuffleMissing) > 1 &&
        in(0, VAR(shuffleInterval)))                >>
        out(1)                                      >>
        CALL(FrameShuffler::writeComponent)     >> readScans
      |(VAR(shuffleMissing) == 1 &&
        VAR(frameMissing) > 1 &&
        in(VAR(shuffleInterval)))                   >>
        out(1)                                      >>
        CALL(FrameShuffler::writeComponent)     >> readScans
      |(VAR(frameMissing) == 1 &&
        in(VAR(shuffleInterval)))                   >>
        out(1)                                      >>
        CALL(FrameShuffler::writeComponent)     >> getFrameDesc
      ;
/*

    newScan

    newScan
      // this must be a CTRLCMD_NEWSCAN
      =   GUARD(FrameShuffler::frameEnd)        >>
        CALL(FrameShuffler::dumpFrame)          >> newFrame
      | (!GUARD(FrameShuffler::frameEnd) &&
         inCtrlImage(1))                            >>
        CALL(FrameShuffler::processNewScan)     >> readScan
      ;
    readScan
      // read component values for scan
      =   GUARD(FrameShuffler::scanEnd)         >> newScan
      | (!GUARD(FrameShuffler::scanEnd) &&
         in(1))                                     >>
        CALL(FrameShuffler::writeComponent)     >> readScan
      ;
    */
  }
};

#endif // _INCLUDED_FRAME_BUFFER_WRITER_HPP
