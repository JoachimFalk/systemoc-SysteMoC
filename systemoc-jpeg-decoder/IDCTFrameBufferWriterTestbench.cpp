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

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_moc.hpp>

#include "channels.hpp"

#include "FileSource.hpp"
//IDCT
#include "block2row.hpp"
#include "IDCT2d.hpp"
#include "col2block.hpp"
#include "InvLevel.hpp"
#include "Clip.hpp"
#include "FrameBufferWriter.hpp"

struct Scan {
  IntCompID_t scanPattern[6];
  std::string idctCoeffFileName;
};

typedef std::vector<Scan> ScanVector;

class IDCTScanSource: public smoc_actor {
public:
  smoc_port_out<IDCTCoeff_t>   out;
  smoc_port_out<ImageParam>    outCtrlImage;
protected:
  std::ifstream inputStream;
  ScanVector    scanVector;

  void process() {
    codeword_t byte = inputStream.get();
    out[0] = byte;
  }

  bool streamValid() const
    { return inputStream.good(); }

  smoc_firing_state start;
public:
  IDCTScanSource(sc_module_name name, const ScanVector &scanVector)
    : smoc_actor(name, start), scanVector(scanVector) {
    start = (out(1) && GUARD(IDCTScanSource::streamValid)) >>
            CALL(IDCTScanSource::process)                  >> start;
  }
};

class Testbench: public smoc_graph {
private:
  IDCTScanSource    mIDCTScanSource;
  // Begin IDCT2D
  m_block2row       mBlock2Row;
  m_idct2d          mIDCT2D;
  m_col2block       mCol2Block;
  // End IDCT2D
//Round             mRound;
  InvLevel          mInvLevel;
  Clip              mClip;
  FrameBufferWriter mSink;
public:
  Testbench(sc_module_name name, const ScanVector &scanVector)
    : smoc_graph(name),
      mIDCTScanSource("mIDCTScanSource", scanVector),
      // Begin IDCT2D
      mBlock2Row("mBlock2Row"),
      mIDCT2D("mIDCT2D"),
      mCol2Block("mCol2Block"),
      // End IDCT2D
//    mRound("mRound"),
      mInvLevel("mInvLevel"),
      mClip("mClip"),
      mSink("mSink")
  {
    connectNodePorts(mIDCTScanSource.out, mBlock2Row.b, smoc_fifo<IDCTCoeff_t>(16));
    
    connectNodePorts(mBlock2Row.C0, mIDCT2D.i0, smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C1, mIDCT2D.i1, smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C2, mIDCT2D.i2, smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C3, mIDCT2D.i3, smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C4, mIDCT2D.i4, smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C5, mIDCT2D.i5, smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C6, mIDCT2D.i6, smoc_fifo<int>(16));
    connectNodePorts(mBlock2Row.C7, mIDCT2D.i7, smoc_fifo<int>(16));
    
    connectNodePorts(mIDCT2D.o0, mCol2Block.R0, smoc_fifo<int>(16));
    connectNodePorts(mIDCT2D.o1, mCol2Block.R1, smoc_fifo<int>(16));
    connectNodePorts(mIDCT2D.o2, mCol2Block.R2, smoc_fifo<int>(16));
    connectNodePorts(mIDCT2D.o3, mCol2Block.R3, smoc_fifo<int>(16));
    connectNodePorts(mIDCT2D.o4, mCol2Block.R4, smoc_fifo<int>(16));
    connectNodePorts(mIDCT2D.o5, mCol2Block.R5, smoc_fifo<int>(16));
    connectNodePorts(mIDCT2D.o6, mCol2Block.R6, smoc_fifo<int>(16));
    connectNodePorts(mIDCT2D.o7, mCol2Block.R7, smoc_fifo<int>(16));
    
//  connectNodePorts(mCol2Block.b, mRound.in, smoc_fifo<int>(16));
//  connectNodePorts(mRound.out, mInvLevel.in, smoc_fifo<int>(1));
    connectNodePorts(mCol2Block.b,  mInvLevel.in, smoc_fifo<int>(16));
    connectNodePorts(mInvLevel.out, mClip.in, smoc_fifo<int>(1));
    connectNodePorts(mClip.out,     mSink.in, smoc_fifo<int>(1));
    
    connectNodePorts(mIDCTScanSource.outCtrlImage, mSink.inCtrlImage, smoc_fifo<int>(1));
  }
};

#ifndef KASCPAR_PARSING
int sc_main (int argc, char **argv) {
  if (argc > 3) {
    std::cerr
      << (argv[0] != NULL ? argv[0] : "???")
      << " <width> <height> <scanpattern: idctcoeff filename>+" << std::endl;
    exit(-1);
  }
  
  smoc_top_moc<Testbench> testbench("testbench", argv[1]);
  
  sc_start(-1);
  
  return 0;
}
#endif
