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

#ifndef _INCLUDED_INVHUFFMAN_HPP
#define _INCLUDED_INVHUFFMAN_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_node_types.hpp>

#include <assert.h>

#include "smoc_synth_std_includes.hpp"
#include "channels.hpp"
#include "BitSplitter.hpp"

#define HUFF_GET_CATEGORY(x) ((x) && 0x0F)
#define HUFF_GET_RUNLENGTH(x) ((x) && 0xF0)

#define HUFF_EOB 0x00
#define HUFF_RUNLENGTH_ZERO_AMPLITUDE 0xF0

/// Debug dht synchronisation
#define IS_DHT_SYNC(x) (0xF0 == (x))


#include "debug_config.h"
#include <cosupport/smoc_debug_out.hpp>
// if compiled with DBG_HUFF_DECODER create stream and include debug macros
#ifdef DBG_HUFF_DECODER
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM cerr
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif


#ifdef DBG_ENABLE
using CoSupport::Debug;
#endif // DBG_ENABLE


// return specified value from huffman table
inline uint16_t huffTableValue(const HuffTableChannelType type,
                               const int index,
                               const smoc_port_in<HuffTableChannel_t> &table)
{
  // 8 bit
  if ((type == HUFF_HUFFVAL_OFFSET16) || (type == HUFF_VALPTR_OFFSET16)) {
    if (index % 2) {
      return table[type + index/2] & 0x00ff;
    }
    else {
      return (table[type + index/2] & 0xff00) >> 8;
    }
  }
  // 16 bit
  else {
    return table[type + index];
  }
}


/******************************************************************************
 *
 *
 */
class InvHuffman: public smoc_actor {
public:
  smoc_port_in<JpegChannel_t>       in;
  smoc_port_in<HuffTableChannel_t>  inHuffTblAC0;
  smoc_port_in<HuffTableChannel_t>  inHuffTblDC0;
  smoc_port_in<HuffTableChannel_t>  inHuffTblAC1;
  smoc_port_in<HuffTableChannel_t>  inHuffTblDC1;
  smoc_port_out<JpegChannel_t>      out;

  //
  InvHuffman(sc_module_name name);

private:
  //
  bool isUseHuff() const {
    return (JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_USEHUFF);
  }

  //
  bool isNewScan() const {
    return (JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_NEWSCAN);
  }

  //
  bool isDiscardHuff() const {
    return (JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_DISCARDHUFF);
  }

  // some ctrl we are not interested in
  bool isTediousCtrl() const {
    return !(isDiscardHuff() || isNewScan() || isUseHuff());
  }

#if 0
  //
  bool isHuffTblId(HuffTblType_t type, HuffTblID_t id) const {
    /*cerr << "isHuffTblId(): type: " << JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0])
         << "; id: " << JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) << endl
         << "   is? type: " << type << "; id: " << id << endl;*/
    assert(JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_DISCARDHUFF);

    return ( (JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) == id) &&
             (JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) == type) );
  }
#endif

  //
  bool isHuffTblIdAC0() const {
    assert(JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_DISCARDHUFF);

    return ( (JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) == 0) &&
             (JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) == HUFFTBL_AC) );
  }

  //
  bool isHuffTblIdAC1() const {
    assert(JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_DISCARDHUFF);

    return ( (JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) == 1) &&
             (JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) == HUFFTBL_AC) );
  }

  //
  bool isHuffTblIdDC0() const {
    assert(JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_DISCARDHUFF);

    return ( (JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) == 0) &&
             (JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) == HUFFTBL_DC) );
  }

  //
  bool isHuffTblIdDC1() const {
    assert(JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_DISCARDHUFF);

    return ( (JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) == 1) &&
             (JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) == HUFFTBL_DC) );
  }

  //
  bool currentDcIsDc0() const;

  //
  bool currentDcIsDc1() const;

  //
  bool currentAcIsAc0() const;

  //
  bool currentAcIsAc1() const;

  //
  bool canHuffDecodeDc0() const;

  //
  bool canHuffDecodeDc1() const;

  //
  bool isBitSplitterFull() const { return (m_BitSplitter.isFull()); }

  //
  bool isBitSplitterEmpty() const { return (m_BitSplitter.isEmpty()); }

  //
  bool isData() const { return !m_BitSplitter.isEmpty(); }

  //
  bool isEnoughDcBits() const {
    return (m_BitSplitter.bitsLeft() >= m_receiveDcBits);
  }

  //
  bool isEnoughAcBits() const {
    // see F.13, p. 106
    // cast to avoid compiler warning
    return (m_BitSplitter.bitsLeft() >= (unsigned int)(m_receiveAcSymbol & 0x0f));
  }

  //
  bool canStore() const { return !m_BitSplitter.isFull(); }

  //
  bool isMoreAc() const { return (m_currentAc < 63); }

  //
  void storeData() {
    DBG_OUT("storeData(): store one byte: "
            << hex << (unsigned int)JS_DATA_GET(in[0]) << dec << endl);
    m_BitSplitter.addByte(JS_DATA_GET(in[0]));
  }

  //
  void flushBitSplitter() { m_BitSplitter.flush(); }

  // decode huffmann encoded DC bit length
  void huffDecodeDC0();

  //
  void huffDecodeDC1();

  //
  void huffDecodeAC0();

  //
  void huffDecodeAC1();

  //
  void writeDcDiff();
  
  //
  void writeAcDiff();

  // decode
  bool decodeHuff(const HuffTableType tableType,
                  DecodedSymbol_t &symbol,
                  unsigned int &numBits) const;

  //
  void setCompInterleaving();

  //
  void useHuff();

#if 0
  //
  void discardHuff(const smoc_port_in<ExpHuffTbl> &tableIn) {
    DBG_OUT("discardHuff() ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) << " ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) << " ");
    DBG_OUT("\n");
  
    DBG_OUT(tableIn[0] << endl);

    forwardCtrl();
  }
#endif

  //
  void discardHuffAC0() { forwardCtrl(); }

  //
  void discardHuffAC1() { forwardCtrl(); }

  //
  void discardHuffDC0() { forwardCtrl(); }

  //
  void discardHuffDC1() { forwardCtrl(); }

  // forward control commands from input to output
  void forwardCtrl() {
    DBG_OUT("forwardCtrl() " << in[0]  << endl);
    assert(JS_ISCTRL(in[0]));
    out[0] = in[0];
  }

  //
  void finishedBlock();

  smoc_firing_state main;
  smoc_firing_state discoverDC;
  smoc_firing_state discoverAC;
  smoc_firing_state writeAC;
  int m_compIndex;
  
  //IntCompID_t m_compInterleaving[SCANPATTERN_LENGTH];      //6
  IntCompID_t m_compInterleaving_0;
  IntCompID_t m_compInterleaving_1;
  IntCompID_t m_compInterleaving_2;
  IntCompID_t m_compInterleaving_3;
  IntCompID_t m_compInterleaving_4;
  IntCompID_t m_compInterleaving_5;

  //HuffTblID_t m_useHuffTableAc[JPEG_MAX_COLOR_COMPONENTS]; //3
  HuffTblID_t m_useHuffTableAc_0;
  HuffTblID_t m_useHuffTableAc_1;
  HuffTblID_t m_useHuffTableAc_2;

  //HuffTblID_t m_useHuffTableDc[JPEG_MAX_COLOR_COMPONENTS]; //3
  HuffTblID_t m_useHuffTableDc_0;
  HuffTblID_t m_useHuffTableDc_1;
  HuffTblID_t m_useHuffTableDc_2;
#ifdef DBG_ENABLE
  CoSupport::DebugOstream dbgout;
//CoSupport::DebugStreambuf dbgbuff;
#endif // DBG_ENABLE
  BitSplitter m_BitSplitter;
  int m_currentComp;
  DecodedSymbol_t m_receiveDcBits;
  DecodedSymbol_t m_receiveAcSymbol;
  unsigned int m_currentAc;
};


/******************************************************************************
 *
 *
 */
class HuffTblDecoder: public smoc_actor {
public:
  smoc_port_in<codeword_t>           in;
  smoc_port_out<HuffTableChannel_t>  outHuffTblAC0;
  smoc_port_out<HuffTableChannel_t>  outHuffTblDC0;
  smoc_port_out<HuffTableChannel_t>  outHuffTblAC1;
  smoc_port_out<HuffTableChannel_t>  outHuffTblDC1;

  HuffTblDecoder(sc_module_name name);

private:
  //
  bool hasMoreHUFFVAL() const {
    return m_symbolsLeft > 0;
  }

  //
  //bool isTable(const HuffTableType type) const;

  //
  bool HuffTblDecoder::isTableAC0() const {
    return (IS_TABLE_CLASS_AC(m_tcth)) && (IS_TABLE_DEST_ZERO(m_tcth));
  }

  //
  bool HuffTblDecoder::isTableAC1() const {
    return (IS_TABLE_CLASS_AC(m_tcth)) && (IS_TABLE_DEST_ONE(m_tcth));
  }

  //
  bool HuffTblDecoder::isTableDC0() const {
    return (IS_TABLE_CLASS_DC(m_tcth)) && (IS_TABLE_DEST_ZERO(m_tcth));
  }

  //
  bool HuffTblDecoder::isTableDC1() const {
    return (IS_TABLE_CLASS_DC(m_tcth)) && (IS_TABLE_DEST_ONE(m_tcth));
  }



  //
  void storeTcTh() {
    m_tcth = in[0];
    DBG(const unsigned int tc = (m_tcth >> 4) & 0x0f);
    DBG(const unsigned int th = m_tcth & 0x0f);
    DBG_OUT("storeTcTh(): TC = " << tc << "; TH = " << th << endl);
  }

  //
  void storeBITS();

  //
  void storeHUFFVAL();

#if 0
  //
  int getHUFFSIZE(const uint8_t valPtr[16], const uint8_t pos) const;
#endif
 
  //
  void finishTable();

#if 0
  // no parameter anymore
  //
  void writeTable(smoc_port_out<ExpHuffTbl> &out);
#endif

  //
  void HuffTblDecoder::writeTableToChannel(const HuffTableType tableType,
                                           const ExpHuffTbl &table);

  //
  void HuffTblDecoder::writeTableAC0();

  //
  void HuffTblDecoder::writeTableAC1();

  //
  void HuffTblDecoder::writeTableDC0();

  //
  void HuffTblDecoder::writeTableDC1();
  

  uint16_t m_symbolsLeft;
  uint8_t  m_tcth;
  //uint8_t  m_BITS[16];
  uint8_t  *m_BITS;
  ExpHuffTbl m_tmpHuff;
  unsigned int m_huffWritePos;
#ifdef DBG_ENABLE
  CoSupport::DebugOstream dbgout;
#endif // DBG_ENABLE
  smoc_firing_state waitTcTh;
  smoc_firing_state waitHUFFVAL;
  smoc_firing_state waitBITS;
  smoc_firing_state s_writeTable;
};


/******************************************************************************
 *
 *
 */
class HuffDecoder: public smoc_graph {
public:
  smoc_port_in<JpegChannel_t>  in;
  smoc_port_in<codeword_t>     inCodedHuffTbl;
  smoc_port_out<JpegChannel_t> out;

  HuffDecoder(sc_module_name name)
    : smoc_graph(name),
      mInvHuffman("mInvHuffman"),
      mHuffTblDecoder("mHuffTblDecoder")
  {
    smoc_fifo<HuffTableChannel_t> huffFifo(2 * HUFF_TABLE_SIZE16);
    for (int i = 0; i < HUFF_TABLE_SIZE16; ++i)
      huffFifo << 0x0;

    mInvHuffman.in(in);
    mHuffTblDecoder.in(inCodedHuffTbl);
    connectNodePorts(
      mHuffTblDecoder.outHuffTblAC0,
      mInvHuffman.inHuffTblAC0,
      huffFifo);
    connectNodePorts(
      mHuffTblDecoder.outHuffTblAC1,
      mInvHuffman.inHuffTblAC1,
      huffFifo);
    connectNodePorts(
      mHuffTblDecoder.outHuffTblDC0,
      mInvHuffman.inHuffTblDC0,
      huffFifo);
    connectNodePorts(
      mHuffTblDecoder.outHuffTblDC1,
      mInvHuffman.inHuffTblDC1,
      huffFifo);
    mInvHuffman.out(out);
  }

private:
  InvHuffman      mInvHuffman;
  HuffTblDecoder  mHuffTblDecoder;
};


#endif // _INCLUDED_INVHUFFMAN_HPP
