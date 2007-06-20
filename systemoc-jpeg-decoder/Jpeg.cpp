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

#include "smoc_synth_std_includes.hpp"

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_moc.hpp>

#include "channels.hpp"

#ifdef DUMP_INTERMEDIATE
# include "1D_dup2.hpp"
# include "BlockSnk.hpp"
#endif

#ifndef JPEG_SRC
# include "FileSource.hpp"
#else
# include "JpegSrc.hpp"
#endif
#include "Parser.hpp"
#include "HuffDecoder.hpp"
#include "InvZrl.hpp"
#include "DcDecoding.hpp"
#include "InvQuant.hpp"
#include "InvZigZag.hpp"
#include "CtrlSieve.hpp"
#include "MIdct2D.hpp"
#ifdef STATIC_IMAGE_SIZE
# include "FrameShuffler.hpp"
#else
# include "FrameBufferWriter.hpp"
#endif
#include "Dup.hpp"
#include "YCbCr2RGB.hpp"
#include "PGMsink.hpp"

#include <cosupport/string_convert.hpp>

class Jpeg: public smoc_graph {
private:
#ifdef JPEG_SRC
  JpegSrc                 mSrc;
#else
  FileSource              mSrc;
#endif
  Parser                  mParser;
  HuffDecoder             mHuffDecoder;
  InvZrl                  mInvZrl;
  DcDecoding              mDcDecoding;
  InvQuant                mInvQuant;
  CtrlSieve               mCtrlSieve;
  InvZigZag               mInvZigZag;
#ifdef DUMP_INTERMEDIATE
  m_1D_dup2<IDCTCoeff_t>  mDup2_1;
  m_block_sink            mBlockSnk;
#endif
  MIdct2D                 mIdct2D;
#ifdef STATIC_IMAGE_SIZE
  FrameShuffler           mShuffle;
#else
  FrameBufferWriter       mFrameBuffer;
#endif
  Dup                     mDup;
  YCrCb2RGB               mYCbCr;
  m_pgm_sink              mPGMsink;
public:
  Jpeg(sc_module_name name, char *fileName, unsigned int dimX, unsigned int dimY, unsigned int comp)
    : smoc_graph(name),
#ifdef JPEG_SRC
      mSrc("mSrc"),
#else
      mSrc("mSrc", fileName),
#endif
      mParser("mParser"),
      mHuffDecoder("mHuffDecoder"),
      mInvZrl("mInvZrl"),
      mDcDecoding("mDcDecoding"),
      mInvQuant("InvQuant"),
      mCtrlSieve("CtrlSieve"),
      mInvZigZag("InvZigZag"),
#ifdef DUMP_INTERMEDIATE
      mDup2_1("DupIDCTCoeff"),
      mBlockSnk("mBlockSnk"),
#endif
      mIdct2D("mIdct2D", 128, 0, 255),
#ifdef STATIC_IMAGE_SIZE
      mShuffle("Shuffle", dimX, dimY, comp),
#else
      mFrameBuffer("FrameBuffer"),
#endif
      mDup("mDup"),
      mYCbCr("mYCbCr"),
      mPGMsink("mPGMsink")
      
  {

    const unsigned int shuffle_fifo_buffer_slack = 1;

#ifndef KASCPAR_PARSING
    connectNodePorts<4>(mSrc.out,             mParser.in);
    connectNodePorts<4>(mParser.out,          mHuffDecoder.in);
    connectNodePorts<2>(mParser.outCtrlImage, mDup.in);    
    connectNodePorts<2>(mDup.out1,            mPGMsink.inCtrlImage);
#ifdef STATIC_IMAGE_SIZE
    connectNodePorts<2>(mDup.out2,            mShuffle.inCtrlImage);
#else
    connectNodePorts<2>(mDup.out2,            mFrameBuffer.inCtrlImage);
#endif
    connectNodePorts<32>(mParser.outCodedHuffTbl, mHuffDecoder.inCodedHuffTbl);
    
    // The +1 is required, because the parser wants to send the QT header
    // together with the discard command!
    //smoc_fifo<qt_table_t> qtFifo(JS_QT_TABLE_SIZE + 1);
    smoc_fifo<qt_table_t> qtFifo(128); //JS_QT_TABLE_SIZE + 1 = 66 -> 128 = 2^7
    for (unsigned int i = 0; i < JS_QT_TABLE_SIZE; ++i)
      qtFifo << qt_table_t();
    connectNodePorts(mParser.qt_table_0, mInvQuant.qt_table_0, qtFifo);
    connectNodePorts(mParser.qt_table_1, mInvQuant.qt_table_1, qtFifo);
    connectNodePorts(mParser.qt_table_2, mInvQuant.qt_table_2, qtFifo);
    connectNodePorts(mParser.qt_table_3, mInvQuant.qt_table_3, qtFifo);
    
    connectNodePorts<4>(mHuffDecoder.out,         mInvZrl.in);
    
    connectNodePorts<2>(mInvZrl.out,              mDcDecoding.in);
    connectNodePorts<2>(mDcDecoding.out,          mInvQuant.in);
    connectNodePorts<2>(mInvQuant.out,            mCtrlSieve.in);
    
    // FIXME: by rewriting mInvZigZag, the required buffer space can
    // be reduced
    connectNodePorts<2*JPEG_BLOCK_SIZE>(mCtrlSieve.out,mInvZigZag.in);
    
# ifdef DUMP_INTERMEDIATE
    connectNodePorts<2>(mInvZigZag.out,mDup2_1.in);
    connectNodePorts<128>(mDup2_1.out1,mIdct2D.in);
    connectNodePorts<2>(mDup2_1.out2,mBlockSnk.in);
# else
    // mInvZigZag -> IDCT -> mInvLevel
    connectNodePorts<128>(mInvZigZag.out, mIdct2D.in);
# endif   
    
# ifdef STATIC_IMAGE_SIZE
    connectNodePorts(mIdct2D.out, mShuffle.in, smoc_fifo<IDCTCoeff_t>(dimX*JPEG_BLOCK_HEIGHT*comp*shuffle_fifo_buffer_slack));
    connectNodePorts<2>(mShuffle.out, mYCbCr.in);
# else
    connectNodePorts<2>(mIdct2D.out, mFrameBuffer.in);
    connectNodePorts<2>(mFrameBuffer.out, mYCbCr.in);
# endif    
    connectNodePorts<2>(mYCbCr.out, mPGMsink.in);
#endif // KASCPAR_PARSING
  }
};

#ifndef KASCPAR_PARSING
int sc_main (int argc, char **argv) {
  char *filename = "4motion_128.jpg";
  unsigned int width = 128, height = 128, compCount = 3;
  
  if (argc != 5) {
    std::cerr
      << (argv[0] != NULL ? argv[0] : "???")
      << " <jpeg filename> <width> <height> <compCount>" << std::endl;
  } else {
    filename  = argv[1];
    width     = CoSupport::strAs<unsigned int>(argv[2]);
    height    = CoSupport::strAs<unsigned int>(argv[3]);
    compCount = CoSupport::strAs<unsigned int>(argv[4]);
  }
  
  smoc_top_moc<Jpeg> jpeg("jpeg", filename, width, height, compCount);
  
  sc_start(-1);
  
  return 0;
}
#endif
