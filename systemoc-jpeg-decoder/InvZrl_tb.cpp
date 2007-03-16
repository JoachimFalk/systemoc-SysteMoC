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

#define DUMP_INTERMEDIATE

#include <list>
#include <set>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_moc.hpp>

#include "channels.hpp"

#ifdef DUMP_INTERMEDIATE
# include "1D_dup2.hpp"
# include "BlockSnk.hpp"
#endif

#include "ConstSource.hpp"
#include "TuppleScanSrc.hpp"
#include "InvZrl.hpp"
#include "DcDecoding.hpp"
#include "InvQuant.hpp"
#include "InvZigZag.hpp"
#include "CtrlSieve.hpp"
// Begin IDCT2D
#include "block2row.hpp"
#include "IDCT2d.hpp"
#include "col2block.hpp"
// End IDCT2D
//#include "Round.hpp"
#include "InvLevel.hpp"
#include "Clip.hpp"
#include "FrameBufferWriter.hpp"

class Testbench: public smoc_graph {
private:
  m_const_source<qt_table_t> mConstSrc0;
  m_const_source<qt_table_t> mConstSrc1;
  m_const_source<qt_table_t> mConstSrc2;
  m_const_source<qt_table_t> mConstSrc3;
  TuppleScanSource mTuppleScanSrc;
  InvZrl mInvZrl;  
  DcDecoding mDcDeconding;
  InvQuant mInvQuant;
  CtrlSieve         mCtrlSieve;
  InvZigZag mInvZigZag;
#ifdef DUMP_INTERMEDIATE
  m_1D_dup2<IDCTCoeff_t>mDup2_1;
  m_block_sink mBlockSnk;
#endif
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
  Testbench(sc_module_name name, 
	    size_t width, 
	    size_t height, 
	    const ScanVector &scanVector)
    : smoc_graph(name),
      mConstSrc0("mConstSrc0",1),
      mConstSrc1("mConstSrc1",1),
      mConstSrc2("mConstSrc2",1),
      mConstSrc3("mConstSrc3",1),
      mTuppleScanSrc("mTuppleSource",width,height,scanVector),
      mInvZrl("mInvZrl"),
      mDcDeconding("mDcDecoder"),
      mInvQuant("mInvQuant"),
      mCtrlSieve("CtrlSieve"),
      mInvZigZag("mInvZigZag"),
#ifdef DUMP_INTERMEDIATE
      mDup2_1("DupIDCTCoeff"),
      mBlockSnk("mBlockSnk"),
#endif
      // Begin IDCT2D
      mBlock2Row("mBlock2Row"),
      mIDCT2D("mIDCT2D"),
      mCol2Block("mCol2Block"),
      // End IDCT2D
//    mRound("Round"),
      mInvLevel("InvLevel"),
      mClip("Clip"),
      mSink("Sink")
  { 

    connectNodePorts<65>(mConstSrc0.out,mInvQuant.qt_table_0);
    connectNodePorts<65>(mConstSrc1.out,mInvQuant.qt_table_1);
    connectNodePorts<65>(mConstSrc2.out,mInvQuant.qt_table_2);
    connectNodePorts<65>(mConstSrc3.out,mInvQuant.qt_table_3);

    connectNodePorts<1>(mTuppleScanSrc.out,mInvZrl.in);
    connectNodePorts<1>(mTuppleScanSrc.outCtrlImage,mSink.inCtrlImage);
    
    connectNodePorts<1>(mInvZrl.out,mDcDeconding.in);
    connectNodePorts<1>(mDcDeconding.out,mInvQuant.in);
    connectNodePorts<1>(mInvQuant.out,            mCtrlSieve.in);
    
    // FIXME: by rewriting mInvZigZag, the required buffer space can
    // be reduced
    connectNodePorts<JPEG_BLOCK_SIZE>(mCtrlSieve.out,mInvZigZag.in);

#ifdef DUMP_INTERMEDIATE
    connectNodePorts<1>(mInvZigZag.out,mDup2_1.in);
    connectNodePorts<64>(mDup2_1.out1,mBlock2Row.b);
    connectNodePorts<1>(mDup2_1.out2,mBlockSnk.in);
#else
    //InvZigZag -> IDCT, IDCT -> mRound
    connectNodePorts<64>(mInvZigZag.out, mBlock2Row.b);
#endif   

    connectNodePorts<16>(mBlock2Row.C0, mIDCT2D.i0);
    connectNodePorts<16>(mBlock2Row.C1, mIDCT2D.i1);
    connectNodePorts<16>(mBlock2Row.C2, mIDCT2D.i2);
    connectNodePorts<16>(mBlock2Row.C3, mIDCT2D.i3);
    connectNodePorts<16>(mBlock2Row.C4, mIDCT2D.i4);
    connectNodePorts<16>(mBlock2Row.C5, mIDCT2D.i5);
    connectNodePorts<16>(mBlock2Row.C6, mIDCT2D.i6);
    connectNodePorts<16>(mBlock2Row.C7, mIDCT2D.i7);
    
    connectNodePorts<16>(mIDCT2D.o0, mCol2Block.R0);
    connectNodePorts<16>(mIDCT2D.o1, mCol2Block.R1);
    connectNodePorts<16>(mIDCT2D.o2, mCol2Block.R2);
    connectNodePorts<16>(mIDCT2D.o3, mCol2Block.R3);
    connectNodePorts<16>(mIDCT2D.o4, mCol2Block.R4);
    connectNodePorts<16>(mIDCT2D.o5, mCol2Block.R5);
    connectNodePorts<16>(mIDCT2D.o6, mCol2Block.R6);
    connectNodePorts<16>(mIDCT2D.o7, mCol2Block.R7);
    
//  connectNodePorts<64>(mCol2Block.b, mRound.in);
//  connectNodePorts<1>(mRound.out, mInvLevel.in);
    connectNodePorts<64>(mCol2Block.b, mInvLevel.in);
    connectNodePorts<1>(mInvLevel.out, mClip.in);
    connectNodePorts<1>(mClip.out,     mSink.in);
    
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
  
  size_t      width, height;
  ScanVector  scanVector;
  
  width  = atoi(argv[1]);
  height = atoi(argv[2]);
  
  for (const char *const *argIter = &argv[3]; *argIter != NULL; ++argIter) {
    size_t      pos = 0;
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
