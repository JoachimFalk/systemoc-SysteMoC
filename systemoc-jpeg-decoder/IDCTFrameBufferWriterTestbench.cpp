//  -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 expandtab:
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

#include <list>
#include <set>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_moc.hpp>

#include "channels.hpp"

#include "FileSource.hpp"
#include "MIdct2D.hpp"
#ifdef STATIC_IMAGE_SIZE
# include "FrameShuffler.hpp"
#else
# include "FrameBufferWriter.hpp"
#endif
#include "Dup.hpp"
#include "YCbCr2RGB.hpp"
#include "PGMsink.hpp"

struct Scan {
  IntCompID_t scanPattern[SCANPATTERN_LENGTH];
  std::string idctCoeffFileName;
};

typedef std::list<Scan> ScanVector;

static
int componentCount(const ScanVector &scanVector) {
  std::set<IntCompID_t> componentSet;
  
  for (ScanVector::const_iterator iter = scanVector.begin();
       iter != scanVector.end();
       ++iter) {
    const Scan &scan = *iter;
    
    for (unsigned int i = 0; i < SCANPATTERN_LENGTH; ++i)
      componentSet.insert(scan.scanPattern[i]);
  }
  return componentSet.size();
}

class IDCTScanSource: public smoc_actor {
public:
  smoc_port_out<IDCTCoeff_t>    out;
  smoc_port_out<JpegChannel_t>  outCtrlImage;
protected:
  unsigned int      width, height;
  ScanVector  scanVector;

  unsigned int      componentVals;

  std::ifstream inputStream;

  void sendNewFrame() {
    int comps = componentCount(scanVector);

    std::cerr << "IDCTScanSource: sendNewFrame"
      << " witdh: " << width
      << " height: " << height
      << " component count: " << comps
      << std::endl;
    outCtrlImage[0] = JS_CTRL_NEWFRAME_SET_CHWORD
      (width, height, comps);
  }

  void sendNewScan() {
    const Scan &scan = scanVector.front();
    
    std::cerr << "IDCTScanSource: sendNewScan"
      << " from file: " << scan.idctCoeffFileName
      << " scanPattern: ";
    for (unsigned int i = 0; i < SCANPATTERN_LENGTH; ++i) {
      std::cerr
        << static_cast<unsigned int>(scan.scanPattern[i]);
//      << (i < SCANPATTERN_LENGTH-1 ? ":" : "");
    }
    std::cerr << std::endl;
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

  void sendIDCTCoeff() {
    IDCTCoeff_t coeff;
    
    inputStream >> coeff;
    assert(inputStream.good()); --componentVals;
//  std::cerr << "IDCTScanSource: Got IDCT Coeff " << coeff << std::endl;
    out[0] = coeff;
  }

  void closeStream() {
    inputStream.close();
    // Reset EOF and ERROR bits
    inputStream.clear();
  }

  void allDone() {
    std::cerr << "IDCTScanSource: All done !!!" << std::endl;
  }

  bool haveScans() const
    { return !scanVector.empty(); }

  smoc_firing_state start;
  smoc_firing_state scanNew;
  smoc_firing_state scanSend;
  smoc_firing_state end;
public:
  IDCTScanSource(sc_module_name name,
      unsigned int width, unsigned int height, const ScanVector &scanVector)
    : smoc_actor(name, start),
      width(width), height(height), scanVector(scanVector) {
    start
      = outCtrlImage(1)                       >>
        CALL(IDCTScanSource::sendNewFrame)    >> scanNew
      ;
    scanNew
      = GUARD(IDCTScanSource::haveScans)      >>
        outCtrlImage(1)                       >>
        CALL(IDCTScanSource::sendNewScan)     >> scanSend
      | !GUARD(IDCTScanSource::haveScans)     >>
        CALL(IDCTScanSource::allDone)         >> end
      ;
    scanSend
      = (VAR(componentVals)  > 0U)            >>
        out(1)                                >>
        CALL(IDCTScanSource::sendIDCTCoeff)   >> scanSend
      | (VAR(componentVals) == 0U)            >>
        CALL(IDCTScanSource::closeStream)     >> scanNew
      ;
  }
};

class Testbench: public smoc_graph {
private:
  IDCTScanSource    mIDCTScanSource;
  MIdct2D           mIdct2D;
#ifdef STATIC_IMAGE_SIZE
  FrameShuffler     mShuffle;
#else
  FrameBufferWriter mFrameBuffer;
#endif
  Dup               mDup;
  YCrCb2RGB         mYCbCr;
  m_pgm_sink        mPGMsink;
public:
  Testbench(sc_module_name name, unsigned int width, unsigned int height, const ScanVector &scanVector)
    : smoc_graph(name),
      mIDCTScanSource("mIDCTScanSource", width, height, scanVector),
      mIdct2D("mIdct2D", 128, 0, 255),
#ifdef STATIC_IMAGE_SIZE
      mShuffle("Shuffle", width, height, componentCount(scanVector)),
#else
      mFrameBuffer("FrameBuffer"),
#endif
      mDup("mDup"),
      mYCbCr("mYCbCr"),
      mPGMsink("mPGMsink")
  {
    connectNodePorts<64>(mIDCTScanSource.out,           mIdct2D.in);
    connectNodePorts<1> (mIDCTScanSource.outCtrlImage,  mDup.in);
    
    connectNodePorts<1> (mDup.out1,                     mPGMsink.inCtrlImage);
#ifdef STATIC_IMAGE_SIZE
    connectNodePorts<1> (mDup.out2,                     mShuffle.inCtrlImage);
#else
    connectNodePorts<1> (mDup.out2,                     mFrameBuffer.inCtrlImage);
#endif
    
#ifdef STATIC_IMAGE_SIZE
    connectNodePorts<65536>(mIdct2D.out, mShuffle.in);
    connectNodePorts<1>(mShuffle.out, mYCbCr.in);
#else
    connectNodePorts<1>(mIdct2D.out, mFrameBuffer.in);
    connectNodePorts<1>(mFrameBuffer.out, mYCbCr.in);
#endif    
    connectNodePorts<1>(mYCbCr.out, mPGMsink.in);
  }
};

#ifndef KASCPAR_PARSING
int sc_main (int argc, char **argv) {
  if (argc <= 3) {
    std::cerr
      << (argv[0] != NULL ? argv[0] : "???")
      << " <width> <height> <scanpattern:idctcoeff filename>+" << std::endl;
    exit(-1);
  }
  
  unsigned int width, height;
  ScanVector  scanVector;
  
  width  = atoi(argv[1]);
  height = atoi(argv[2]);
  
  for (const char *const *argIter = &argv[3]; *argIter != NULL; ++argIter) {
    unsigned int pos = 0;
    const char *arg = *argIter;
    
    Scan scan;
    
    while (pos < SCANPATTERN_LENGTH) {
      if (arg[pos] < '0' || arg[pos] > '2') {
        std::cerr << argv[0] << ": scanpattern format error, scanpattern: [0-2]{6} !" << std::endl;
        exit (-1);
      }
      scan.scanPattern[pos] = arg[pos++] - '0';
    }
    if (arg[pos++] != ':') {
      std::cerr << argv[0] << ": missing colon after scanpattern !" << std::endl;
    }
    scan.idctCoeffFileName = &arg[pos];
    scanVector.push_back(scan);
  }
  
  smoc_top_moc<Testbench> testbench("testbench", width, height, scanVector);
  
  sc_start(-1);
  
  return 0;
}
#endif
