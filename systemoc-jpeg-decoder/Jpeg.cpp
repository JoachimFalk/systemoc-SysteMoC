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
#include "Parser.hpp"
#include "InvByteStuff.hpp"
#include "HuffDecoder.hpp"
#include "InvZrl.hpp"
#include "DcDecoding.hpp"
#include "InvQuant.hpp"
#include "InvZigZag.hpp"
#include "CtrlSieve.hpp"
//IDCT
#include "Round.hpp"
#include "InvLevel.hpp"
#include "Clip.hpp"
#include "FrameBufferWriter.hpp"

class Jpeg: public smoc_graph {
private:
  FileSource        mSrc;
  Parser            mParser;
  InvByteStuff      mInvByteStuff;
  HuffDecoder       mHuffDecoder;
  InvZrl            mInvZrl;
  DcDecoding        mDcDecoding;
  InvQuant          mInvQuant;
  CtrlSieve         mCtrlSieve;
  InvZigZag         mInvZigZag;
  //IDCT
  Round             mRound;
  InvLevel          mInvLevel;
  Clip              mClip;
  FrameBufferWriter mSink;
public:
  Jpeg(sc_module_name name, const std::string &fileName)
    : smoc_graph(name),
      mSrc("mSrc", fileName),
      mParser("mParser"),
      mInvByteStuff("mInvByteStuff"),
      mHuffDecoder("mHuffDecoder"),
      mInvZrl("mInvZrl"),
      mDcDecoding("mDcDecoding"),
      mInvQuant("InvQuant"),
      mCtrlSieve("CtrlSieve"),
      mInvZigZag("InvZigZag"),
      //IDCT
      mRound("Round"),
      mInvLevel("Round"),
      mClip("Clip"),
      mSink("Sink")
  {
#ifndef KASCPAR_PARSING
    connectNodePorts<1>(mSrc.out,                    mParser.in);
    connectNodePorts<1>(mParser.out,                 mInvByteStuff.in);
    connectNodePorts<1>(mParser.outCtrlImage,        mSink.inCtrlImage);
    connectNodePorts<1>(mParser.outCodedHuffTbl,     mHuffDecoder.inCodedHuffTbl);
    
    connectNodePorts<1>(mInvByteStuff.out,           mHuffDecoder.in);
    
    connectNodePorts<1>(mHuffDecoder.out,            mInvZrl.in);
    
    connectNodePorts<1>(mInvZrl.out,                 mDcDecoding.in);
    connectNodePorts<1>(mDcDecoding.out,             mInvQuant.in);
    connectNodePorts<1>(mInvQuant.out,               mCtrlSieve.in);
    connectNodePorts<1>(mCtrlSieve.out,              mInvZigZag.in);

    //InvZigZag -> IDCT, IDCT -> mRound

    connectNodePorts<1>(mRound.out,                  mInvLevel.in);
    connectNodePorts<1>(mInvLevel.out,               mClip.in);
    connectNodePorts<1>(mClip.out,                   mSink.in);

#endif
  }
};

#ifndef KASCPAR_PARSING
int sc_main (int argc, char **argv) {
  if (argc != 2) {
    std::cerr
      << (argv[0] != NULL ? argv[0] : "???")
      << " <jpeg filename>" << std::endl;
    exit(-1);
  }
  
  smoc_top_moc<Jpeg> jpeg("jpeg", argv[1]);
  
  sc_start(-1);
  
  return 0;
}
#endif
