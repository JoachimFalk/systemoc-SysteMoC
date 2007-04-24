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

#include "channels.hpp"
#include "BitSplitter.hpp"

#define HUFF_GET_CATEGORY(x) ((x) && 0x0F)
#define HUFF_GET_RUNLENGTH(x) ((x) && 0xF0)

#define HUFF_EOB 0x00
#define HUFF_RUNLENGTH_ZERO_AMPLITUDE 0xF0
#define NEXTBIT_MAX_CLAIM 3

/// Debug dht synchronisation
#define IS_DHT_SYNC(x) (0xF0 == (x))

// what does 'Exp' mean?
struct ExpHuffTbl {
  uint8_t          valPtr[16];   // value pointers to first symbol of codelength
                                 //  'index'
  uint16_t         minCode[16];  // minimal code value for codewords of length
                                 //  'index'
  uint16_t         maxCode[16];  // max ...
  DecodedSymbol_t  huffVal[256]; // symbol-length assignment parameters (B.2.4.2)
};

#include "debug_config.h"
#include <cosupport/smoc_debug_out.hpp>
// if compiled with DBG_HUFF_DECODER create stream and include debug macros
#ifdef DBG_HUFF_DECODER
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif

using CoSupport::Debug;

#define IS_TABLE_CLASS_DC(v) (((v) & 0xF0) == 0x00)
#define IS_TABLE_CLASS_AC(v) (((v) & 0xF0) == 0x10)

#define IS_TABLE_DEST_ZERO(v) (((v) & 0x0F) == 0x00)
#define IS_TABLE_DEST_ONE(v) (((v) & 0x0F) == 0x01)

enum HuffTableType {
  AC0,
  AC1,
  DC0,
  DC1
};


/*****************************************************************************/

// needed by SysteMoC!
ostream &operator<<(ostream &out, const ExpHuffTbl &eht);


/******************************************************************************
 *
 *
 */
class InvHuffman: public smoc_actor {
public:
  smoc_port_in<JpegChannel_t>   in;
  smoc_port_in<ExpHuffTbl>      inHuffTblAC0;
  smoc_port_in<ExpHuffTbl>      inHuffTblDC0;
  smoc_port_in<ExpHuffTbl>      inHuffTblAC1;
  smoc_port_in<ExpHuffTbl>      inHuffTblDC1;
  smoc_port_out<JpegChannel_t>  out;

  //
  InvHuffman(sc_module_name name);

  //
  ~InvHuffman() {
    assert(m_BitSplitter.isEmpty());
  }

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

  //
  bool isHuffTblId(HuffTblType_t type, HuffTblID_t id) const {
    /*cerr << "isHuffTblId(): type: " << JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0])
         << "; id: " << JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) << endl
         << "   is? type: " << type << "; id: " << id << endl;*/
    assert(JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_DISCARDHUFF);

    return ( (JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) == id) &&
             (JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) == type) );
  }

  //
  bool currentDcIsDc0(void) const {
    return m_useHuffTableDc[m_currentComp] == 0;
  }

  //
  bool currentDcIsDc1(void) const {
    return m_useHuffTableDc[m_currentComp] == 1;
  }

  //
  bool currentAcIsAc0(void) const {
    return m_useHuffTableAc[m_currentComp] == 0;
  }

  //
  bool currentAcIsAc1(void) const {
    return m_useHuffTableAc[m_currentComp] == 1;
  }

  //
  bool canHuffDecodeDc(void) const {
    size_t dummy;
    DecodedSymbol_t symbol;
    const bool ret = decodeHuff(getCurrentDcTable(), symbol, dummy);
    return ret;
  }

  //
  bool canHuffDecodeAc(void) const {
    size_t dummy;
    DecodedSymbol_t symbol;
    const bool ret = decodeHuff(getCurrentAcTable(), symbol, dummy);
    return ret;
  }

  //
  bool isData(void) const { return !m_BitSplitter.isEmpty(); }

  //
  bool isEnoughDcBits(void) const {
    return (m_BitSplitter.bitsLeft() >= m_receiveDcBits);
  }

  //
  bool isEnoughAcBits(void) const {
    // see F.13, p. 106
    return (m_BitSplitter.bitsLeft() >=
              static_cast<size_t>(m_receiveAcSymbol & 0x0f));
  }

  //
  bool canStore(void) const { return !m_BitSplitter.isFull(); }

  //
  bool isMoreAc(void) const { return (m_currentAc < 63); }

  //
  void storeData(void) {
    DBG_OUT("storeData(): store one byte: "
            << hex << (size_t)JS_DATA_GET(in[0]) << dec << endl);
    m_BitSplitter.addByte(JS_DATA_GET(in[0]));
  }

  // decode huffmann encoded DC bit length
  void huffDecodeDC(void);

  //
  void huffDecodeAC(void);

  //
  void writeDcDiff(void);
  
  //
  void writeAcDiff(void);

  //
  const smoc_port_in<ExpHuffTbl> &getCurrentAcTable(void) const {
    if (m_useHuffTableAc[m_currentComp] == 0)
      return inHuffTblAC0;
    else {
      assert(m_useHuffTableAc[m_currentComp] == 1);
      return inHuffTblAC1;
    }
  }

  //
  const smoc_port_in<ExpHuffTbl> &getCurrentDcTable(void) const {
    if (m_useHuffTableDc[m_currentComp] == 0)
      return inHuffTblDC0;
    else {
      assert(m_useHuffTableDc[m_currentComp] == 1);
      return inHuffTblDC1;
    }
  }

  // decode
  bool decodeHuff(const smoc_port_in<ExpHuffTbl> &in,
                  DecodedSymbol_t &symbol,
                  size_t &numBits) const;

  //
  void setCompInterleaving(){
    DBG_OUT("setCompInterleaving()");
    for (int i = 0; i < SCANPATTERN_LENGTH; ++i) {
      m_compInterleaving[i] = JS_CTRL_NEWSCAN_GETCOMP(in[0],i);
      DBG_OUT(" " << m_compInterleaving[i]);
    }
    DBG_OUT(endl);

    forwardCtrl();
  }

  //
  void useHuff() {
    IntCompID_t cmp = JS_CTRL_USEHUFF_GETCOMP(in[0]);
    HuffTblID_t  dc = JS_CTRL_USEHUFF_GETDCTBL(in[0]);
    HuffTblID_t  ac = JS_CTRL_USEHUFF_GETACTBL(in[0]);
    DBG_OUT("useHuff() c: " << cmp << " dc: " << dc << " ac: " << ac << endl);
    m_useHuffTableAc[cmp] = ac;
    m_useHuffTableDc[cmp] = dc;

    forwardCtrl();
  }

  //
  void discardHuff(const smoc_port_in<ExpHuffTbl> &tableIn) {
    DBG_OUT("discardHuff() ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) << " ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) << " ");
    DBG_OUT("\n");
  
    DBG_OUT(tableIn[0] << endl);

    forwardCtrl();
  }

  // forward control commands from input to output
  void forwardCtrl() {
    DBG_OUT("forwardCtrl() " << in[0]  << endl);
    assert(JS_ISCTRL(in[0]));
    out[0] = in[0];
  }

  //
  void finishedBlock(void) {
    DBG_OUT("finishedBlock(): finished decoding block\n");
    // select next component
    compIndex = (compIndex + 1) % SCANPATTERN_LENGTH;
    m_currentAc = 0;
  }


  int compIndex;
  IntCompID_t m_compInterleaving[SCANPATTERN_LENGTH];      //6
  HuffTblID_t m_useHuffTableAc[JPEG_MAX_COLOR_COMPONENTS]; //3
  HuffTblID_t m_useHuffTableDc[JPEG_MAX_COLOR_COMPONENTS]; //3
  CoSupport::DebugOstream dbgout;
//CoSupport::DebugStreambuf dbgbuff;
  smoc_firing_state main;
  smoc_firing_state discoverDC;
  smoc_firing_state discoverAC;
  smoc_firing_state writeAC;
  BitSplitter m_BitSplitter;
  int m_currentComp;
  DecodedSymbol_t m_receiveDcBits;
  DecodedSymbol_t m_receiveAcSymbol;
  size_t m_currentAc;
};


/******************************************************************************
 *
 *
 */
class HuffTblDecoder: public smoc_actor {
public:
  smoc_port_in<codeword_t>  in;
  smoc_port_out<ExpHuffTbl> outHuffTblAC0;
  smoc_port_out<ExpHuffTbl> outHuffTblDC0;
  smoc_port_out<ExpHuffTbl> outHuffTblAC1;
  smoc_port_out<ExpHuffTbl> outHuffTblDC1;

  HuffTblDecoder(sc_module_name name);

private:
  //
  bool hasMoreHUFFVAL() const {
    return m_symbolsLeft > 0;
  }

  //
  bool isTable(const HuffTableType type) const;

  //
  void storeTcTh() {
    m_tcth = in[0];
    const size_t tc = (m_tcth >> 4) & 0x0f;
    const size_t th = m_tcth & 0x0f;
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

  //
  void writeTable(smoc_port_out<ExpHuffTbl> &out);
  

  uint16_t m_symbolsLeft;
  uint8_t  m_tcth;
  uint8_t  m_BITS[16];
  ExpHuffTbl m_tmpHuff;
  size_t m_huffWritePos;
  CoSupport::DebugOstream dbgout;
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
#ifndef KASCPAR_PARSING
    mInvHuffman.in(in);
    mHuffTblDecoder.in(inCodedHuffTbl);
    connectNodePorts(
      mHuffTblDecoder.outHuffTblAC0,
      mInvHuffman.inHuffTblAC0,
      smoc_fifo<ExpHuffTbl>(2) << ExpHuffTbl()); // Parser sends DISCARDHUFF
    connectNodePorts(
      mHuffTblDecoder.outHuffTblAC1,
      mInvHuffman.inHuffTblAC1,
      smoc_fifo<ExpHuffTbl>(2) << ExpHuffTbl()); // Parser sends DISCARDHUFF
    connectNodePorts(
      mHuffTblDecoder.outHuffTblDC0,
      mInvHuffman.inHuffTblDC0,
      smoc_fifo<ExpHuffTbl>(2) << ExpHuffTbl()); // Parser sends DISCARDHUFF
    connectNodePorts(
      mHuffTblDecoder.outHuffTblDC1,
      mInvHuffman.inHuffTblDC1,
      smoc_fifo<ExpHuffTbl>(2) << ExpHuffTbl()); // Parser sends DISCARDHUFF
    mInvHuffman.out(out);
#endif
  }

private:
  InvHuffman      mInvHuffman;
  HuffTblDecoder  mHuffTblDecoder;
};


#endif // _INCLUDED_INVHUFFMAN_HPP
