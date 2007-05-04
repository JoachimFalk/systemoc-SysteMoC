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

class FrameBufferWriter: public smoc_actor {
public:
  smoc_port_in<JpegChannel_t> in;
  smoc_port_out<Pixel_t>      out;
  smoc_port_in<JpegChannel_t> inCtrlImage;
protected:
  Pos         frameDim;

  //number of components
  IntCompID_t compCount; 
  IntCompID_t compMissing;

  Pos         compPos[JPEG_MAX_COLOR_COMPONENTS];
  IntCompID_t scanPattern[SCANPATTERN_LENGTH];

  // index into scanPattern
  int         scanIndex;

  // index into 8x8 block
  int         blockIndex;

  //
  int         framePixels;

  // Is of size width*height*compCount
  FrameBuffer frameBuffer;

  void processNewFrame() {
#ifndef KASCPAR_PARSING
    std::cerr << "FrameBufferWriter: processNewFrame";
#endif // KASCPAR_PARSING
    
    assert(JS_ISCTRL(inCtrlImage[0]));
    assert(JS_GETCTRLCMD(inCtrlImage[0]) == CTRLCMD_NEWFRAME);
    
    frameDim.x = JS_CTRL_NEWFRAME_GET_DIMX(inCtrlImage[0]);
    frameDim.y = JS_CTRL_NEWFRAME_GET_DIMY(inCtrlImage[0]);
    compCount  = JS_CTRL_NEWFRAME_GET_COMPCOUNT(inCtrlImage[0]);
    
    framePixels = frameDim.x * frameDim.y;
    
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

  void writePixel() {    
    if (compCount == 1){
      //assert(frameBuffer[(frameDim.x * frameDim.y - framePixels) * 1 + 0] >= 0);
      //assert(frameBuffer[(frameDim.x * frameDim.y - framePixels) * 1 + 0] < 256);
      // Y Cb Cr => Grayscale no Cb Cr
      out[0] = JS_RAWPIXEL_SETVAL(
        frameBuffer[(frameDim.x * frameDim.y - framePixels) * 1 + 0],
        128,
        128);
    }else{
      //assert(frameBuffer[(frameDim.x * frameDim.y - framePixels) * 1 + 0] >= 0);
      //assert(frameBuffer[(frameDim.x * frameDim.y - framePixels) * 1 + 0] < 256);
      //assert(frameBuffer[(frameDim.x * frameDim.y - framePixels) * 1 + 1] >= 0);
      //assert(frameBuffer[(frameDim.x * frameDim.y - framePixels) * 1 + 1] < 256);
      //assert(frameBuffer[(frameDim.x * frameDim.y - framePixels) * 1 + 2] >= 0);
      //assert(frameBuffer[(frameDim.x * frameDim.y - framePixels) * 1 + 2] < 256);
      out[0] = JS_RAWPIXEL_SETVAL(
        frameBuffer[(frameDim.x * frameDim.y - framePixels) * 3 + 0],
        frameBuffer[(frameDim.x * frameDim.y - framePixels) * 3 + 1],
        frameBuffer[(frameDim.x * frameDim.y - framePixels) * 3 + 2]);
    }
    --framePixels;
  }

  void processNewScan() {
#ifndef KASCPAR_PARSING
    std::cerr << "FrameBufferWriter: processNewScan ";
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
//  std::cerr << "FrameBufferWriter: writeComponent" << std::endl;
    
    assert(!JS_ISCTRL(in[0]));
    
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
  }

  bool frameEnd() const {
    assert(/*compMissing >= 0 &&*/ compMissing <= compCount);
    return compMissing == 0;
  }

  bool scanEnd() const {
    // std::cerr << "scanEnd" << std::endl;
    // std::cerr << frameDim.x << " " << frameDim.y << std::endl;
#ifndef NDEBUG
    for (int i = 0; i < JPEG_MAX_COLOR_COMPONENTS; ++i) {
      // std::cerr << compPos[i].x << " " << compPos[i].y << std::endl;
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

  smoc_firing_state newFrame;
  smoc_firing_state newScan;
  smoc_firing_state readScan;
  smoc_firing_state dumpFrame;
public:
  FrameBufferWriter(sc_module_name name)
    : smoc_actor(name, newFrame) {
    newFrame
      // this must be a CTRLCMD_NEWFRAME
      = inCtrlImage(1)                              >>
        CALL(FrameBufferWriter::processNewFrame)    >> newScan
      ;
    newScan
      // this must be a CTRLCMD_NEWSCAN
      =   GUARD(FrameBufferWriter::frameEnd)        >> dumpFrame
      | (!GUARD(FrameBufferWriter::frameEnd) &&
         inCtrlImage(1))                            >>
        CALL(FrameBufferWriter::processNewScan)     >> readScan
      ;
    readScan
      // read component values for scan
      =   GUARD(FrameBufferWriter::scanEnd)         >> newScan
      | (!GUARD(FrameBufferWriter::scanEnd) &&
         in(1))                                     >>
        CALL(FrameBufferWriter::writeComponent)     >> readScan
      ;
    dumpFrame
      = (out(1) && VAR(framePixels) >= 1)            >>
        CALL(FrameBufferWriter::writePixel)         >> dumpFrame
      | (VAR(framePixels) == 0)                     >> newFrame
      ;
  }
};

#endif // _INCLUDED_FRAME_BUFFER_WRITER_HPP
