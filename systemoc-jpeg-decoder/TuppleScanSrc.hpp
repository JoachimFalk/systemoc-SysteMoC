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

#ifndef _INCLUDED_TUPPLESCANSRC_HPP
#define _INCLUDED_TUPPLESCANSRC_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

#include "debug_config.h"
#include <cosupport/smoc_debug_out.hpp>
// if compiled with DBG_TUPPLESCAN_SRC create stream and include debug macros
#ifdef DBG_TUPPLESCAN_SRC
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif

struct Scan {
  IntCompID_t scanPattern[SCANPATTERN_LENGTH];
  std::string idctCoeffFileName;
};

typedef std::list<Scan> ScanVector;

class TuppleScanSource: public smoc_actor {
public:
  smoc_port_out<JpegChannel_t>    out;
  smoc_port_out<JpegChannel_t>  outCtrlImage;
protected:
  unsigned int width, height;
  ScanVector  scanVector;

  unsigned int componentVals;

  std::ifstream inputStream;

  void sendNewFrame() {
    std::set<IntCompID_t> componentSet;
    
    for (ScanVector::const_iterator iter = scanVector.begin();
         iter != scanVector.end();
         ++iter) {
      const Scan &scan = *iter;
      
      for (unsigned int i = 0; i < SCANPATTERN_LENGTH; ++i)
        componentSet.insert(scan.scanPattern[i]);
    }
    DBG_OUT("TuppleScanSource: sendNewFrame"
	    << " witdh: " << width
	    << " height: " << height
	    << " component count: " << componentSet.size()
	    << std::endl);
    outCtrlImage[0] = JS_CTRL_NEWFRAME_SET_CHWORD
      (width, height, componentSet.size());
  }

  void sendNewScan() {
    const Scan &scan = scanVector.front();
    
    DBG_OUT("TuppleScanSource: sendNewScan"
	    << " from file: " << scan.idctCoeffFileName
	    << " scanPattern: ");
    for (unsigned int i = 0; i < SCANPATTERN_LENGTH; ++i) {
      DBG_OUT(static_cast<unsigned int>(scan.scanPattern[i]));
    }
    DBG_OUT(std::endl);
    outCtrlImage[0] = JS_CTRL_NEWSCAN_SET_CHWORD
      (scan.scanPattern[0], scan.scanPattern[1], scan.scanPattern[2],
       scan.scanPattern[3], scan.scanPattern[4], scan.scanPattern[5]);
    inputStream.open(scan.idctCoeffFileName.c_str());
    assert(inputStream.good());
    {
      std::set<IntCompID_t> componentSet;
      for (unsigned int i = 0; i < SCANPATTERN_LENGTH; ++i)
        componentSet.insert(scan.scanPattern[i]);
      componentVals = width * height * componentSet.size();
    }
    scanVector.pop_front();
  }

  void sendTupple() {
    
    CategoryAmplitude_t amplitude;
    Category_t category;
    RunLength_t rle;
    bool dc;
	
    {
      std::string rlz;
      unsigned int temp; // Workaround, because >> is too silly.
      
      inputStream >> rlz >> temp >> amplitude;
      category = temp;
      if (rlz == "DC") {
        rle = 0;
	dc = true;
      } else {
        rle = atol(rlz.c_str());
	dc = false;
      }
    }
    
    assert(inputStream.good());
    
    assert(dc || (componentVals & (JPEG_BLOCK_SIZE - 1)));
    // check for magic end of block tupple
    if (rle == 0 && category == 0 && (!dc)) {
      // down round to the next block start
      componentVals = (componentVals - 1) & (~(JPEG_BLOCK_SIZE - 1));
    } else {
      componentVals -= (1 + rle);
    }
    
    DBG_OUT("rle = " << rle 
            << ", cat = " << (unsigned int)category 
            << ", amplitude = " << amplitude 
            << std::endl);
    DBG_OUT("componentVals = " << componentVals << std::endl);
    
    out[0] = JS_DATA_TUPPLED_SET_CHWORD(amplitude,rle,category);
  }

  void closeStream() {
    inputStream.close();
    // Reset EOF and ERROR bits
    inputStream.clear();
  }

  void allDone() {
    DBG_OUT("TuppleScanSource: All done !!!" << std::endl);
  }

  bool haveScans() const
    { return !scanVector.empty(); }

  smoc_firing_state start;
  smoc_firing_state scanNew;
  smoc_firing_state scanSend;
  smoc_firing_state end;

  CoSupport::DebugOstream dbgout;
public:
  TuppleScanSource(sc_module_name name,
      unsigned int width, unsigned int height, const ScanVector &scanVector)
    : smoc_actor(name, start),
      width(width), height(height), scanVector(scanVector),
      dbgout(std::cout) {
    //Set Debug ostream options
    dbgout << CoSupport::Header("TuppleScanSrc> ");
    
    start
      = outCtrlImage(1)                         >>
        CALL(TuppleScanSource::sendNewFrame)    >> scanNew
      ;
    scanNew
      = GUARD(TuppleScanSource::haveScans)      >>
        outCtrlImage(1)                         >>
        CALL(TuppleScanSource::sendNewScan)     >> scanSend
      | !GUARD(TuppleScanSource::haveScans)     >>
        CALL(TuppleScanSource::allDone)         >> end
      ;
    scanSend
      = (VAR(componentVals)  > 0U)            >>
        out(1)                                >>
        CALL(TuppleScanSource::sendTupple)    >> scanSend
      | (VAR(componentVals) == 0U)            >>
        CALL(TuppleScanSource::closeStream)   >> scanNew
      ;
  }
};

#endif // _INCLUDED_TUPPLESCANSRC_HPP
