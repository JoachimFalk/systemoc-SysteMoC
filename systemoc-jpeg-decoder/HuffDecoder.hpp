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
#define NEXTBIT_MAX_CLAIM 3

/// Debug dht synchronisation
#define IS_DHT_SYNC(x) (0xF0 == (x))

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

#ifdef DBG_ENABLE
using CoSupport::Debug;
#endif // DBG_ENABLE

enum HuffTableType {
  AC0,
  AC1,
  DC0,
  DC1
};


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
    // ignore fill bits
    //assert(m_BitSplitter.isEmpty());
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
  bool currentDcIsDc0(void) const {
    assert(m_currentComp <  3);
    switch (m_currentComp) {
      case 0:
        return m_useHuffTableDc_0 == 0;
        break;
      case 1:
        return m_useHuffTableDc_1 == 0;
        break;
      case 2:
        return m_useHuffTableDc_2 == 0;
        break;
      default:
        assert(0);
        return false;
        break;
    }
  }

  //
  bool currentDcIsDc1(void) const {
    assert(m_currentComp <  3);
    switch (m_currentComp) {
      case 0:
        return m_useHuffTableDc_0 == 1;
        break;
      case 1:
        return m_useHuffTableDc_1 == 1;
        break;
      case 2:
        return m_useHuffTableDc_2 == 1;
        break;
      default:
        assert(0);
        return false;
        break;
    }
  }

  //
  bool currentAcIsAc0(void) const {
    assert(m_currentComp <  3);
    switch (m_currentComp) {
      case 0:
        return m_useHuffTableAc_0 == 0;
        break;
      case 1:
        return m_useHuffTableAc_1 == 0;
        break;
      case 2:
        return m_useHuffTableAc_2 == 0;
        break;
      default:
        assert(0);
        return false;
        break;
    }
  }

  //
  bool currentAcIsAc1(void) const {
    assert(m_currentComp <  3);
    switch (m_currentComp) {
      case 0:
        return m_useHuffTableAc_0 == 1;
        break;
      case 1:
        return m_useHuffTableAc_1 == 1;
        break;
      case 2:
        return m_useHuffTableAc_2 == 1;
        break;
      default:
        assert(0);
        return false;
        break;
    }
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
    // cast to avoid compiler warning
    return (m_BitSplitter.bitsLeft() >= (size_t)(m_receiveAcSymbol & 0x0f));
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

  // FIXME use reference!
  //const smoc_port_in<ExpHuffTbl> &getCurrentAcTable(void) const {
  const ExpHuffTbl getCurrentAcTable(void) const {
    switch (m_currentComp) {
      case 0:
        if (m_useHuffTableAc_0 == 0)
          return inHuffTblAC0[0];
        else
          return inHuffTblAC1[0];
        break;
      case 1:
        if (m_useHuffTableAc_1 == 0)
          return inHuffTblAC0[0];
        else
          return inHuffTblAC1[0];
        break;
      case 2:
        if (m_useHuffTableAc_2 == 0)
          return inHuffTblAC0[0];
        else
          return inHuffTblAC1[0];
        break;
    }

    assert(0);
    /*
    if (m_useHuffTableAc[m_currentComp] == 0)
      return inHuffTblAC0[0];
    else {
      assert(m_useHuffTableAc[m_currentComp] == 1);
      return inHuffTblAC1[0];
    }*/
  }

  // FIXME use reference!
  const ExpHuffTbl getCurrentDcTable(void) const {
    switch (m_currentComp) {
      case 0:
        if (m_useHuffTableDc_0 == 0)
          return inHuffTblDC0[0];
        else
          return inHuffTblDC1[0];
        break;
      case 1:
        if (m_useHuffTableDc_1 == 0)
          return inHuffTblDC0[0];
        else
          return inHuffTblDC1[0];
        break;
      case 2:
        if (m_useHuffTableDc_2 == 0)
          return inHuffTblDC0[0];
        else
          return inHuffTblDC1[0];
        break;
    }

    assert(0);
    /*
    if (m_useHuffTableDc[m_currentComp] == 0)
      return inHuffTblDC0[0];
    else {
      assert(m_useHuffTableDc[m_currentComp] == 1);
      return inHuffTblDC1[0];
    }*/
  }

  // decode
  bool decodeHuff(const ExpHuffTbl &in,
                  DecodedSymbol_t &symbol,
                  size_t &numBits) const;

  //
  void setCompInterleaving(){
    DBG_OUT("setCompInterleaving()");
    /*for (int i = 0; i < SCANPATTERN_LENGTH; ++i) {
      m_compInterleaving[i] = JS_CTRL_NEWSCAN_GETCOMP(in[0],i);
      DBG_OUT(" " << m_compInterleaving[i]);
    }*/
    
    assert(SCANPATTERN_LENGTH == 6);
    m_compInterleaving_0 = JS_CTRL_NEWSCAN_GETCOMP(in[0],0);
    m_compInterleaving_1 = JS_CTRL_NEWSCAN_GETCOMP(in[0],1);
    m_compInterleaving_2 = JS_CTRL_NEWSCAN_GETCOMP(in[0],2);
    m_compInterleaving_3 = JS_CTRL_NEWSCAN_GETCOMP(in[0],3);
    m_compInterleaving_4 = JS_CTRL_NEWSCAN_GETCOMP(in[0],4);
    m_compInterleaving_5 = JS_CTRL_NEWSCAN_GETCOMP(in[0],5);

    switch (m_compIndex) {
      case 0:
        m_currentComp = m_compInterleaving_0;
        break;
      case 1:
        m_currentComp = m_compInterleaving_1;
        break;
      case 2:
        m_currentComp = m_compInterleaving_2;
        break;
      case 3:
        m_currentComp = m_compInterleaving_3;
        break;
      case 4:
        m_currentComp = m_compInterleaving_4;
        break;
      case 5:
        m_currentComp = m_compInterleaving_5;
        break;
    }
    //m_currentComp = m_compInterleaving[m_compIndex];

    DBG_OUT(endl);

    forwardCtrl();
  }

  //
  void useHuff() {
    IntCompID_t cmp = JS_CTRL_USEHUFF_GETCOMP(in[0]);
    HuffTblID_t  dc = JS_CTRL_USEHUFF_GETDCTBL(in[0]);
    HuffTblID_t  ac = JS_CTRL_USEHUFF_GETACTBL(in[0]);
    DBG_OUT("useHuff() c: " << cmp << " dc: " << dc << " ac: " << ac << endl);
    switch (cmp) {
      case 0:
        m_useHuffTableAc_0 = ac;
        m_useHuffTableDc_0 = dc;
        break;
      case 1:
        m_useHuffTableAc_1 = ac;
        m_useHuffTableDc_1 = dc;
        break;
      case 2:
        m_useHuffTableAc_2 = ac;
        m_useHuffTableDc_2 = dc;
        break;
    }
    //m_useHuffTableAc[cmp] = ac;
    //m_useHuffTableDc[cmp] = dc;

    forwardCtrl();
  }

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
  void discardHuffAC0() {
    DBG_OUT("discardHuff() ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) << " ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) << " ");
    DBG_OUT("\n");
  
    DBG_OUT(inHuffTblAC0[0] << endl);

    forwardCtrl();
  }

  //
  void discardHuffAC1() {
    DBG_OUT("discardHuff() ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) << " ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) << " ");
    DBG_OUT("\n");
  
    DBG_OUT(inHuffTblAC1[0] << endl);

    forwardCtrl();
  }

  //
  void discardHuffDC0() {
    DBG_OUT("discardHuff() ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) << " ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) << " ");
    DBG_OUT("\n");
  
    DBG_OUT(inHuffTblDC0[0] << endl);

    forwardCtrl();
  }

  //
  void discardHuffDC1() {
    DBG_OUT("discardHuff() ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) << " ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) << " ");
    DBG_OUT("\n");
  
    DBG_OUT(inHuffTblDC1[0] << endl);

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
    m_compIndex = (m_compIndex + 1) % SCANPATTERN_LENGTH;
    m_currentAc = 0;

    switch (m_compIndex) {
      case 0:
        m_currentComp = m_compInterleaving_0;
        break;
      case 1:
        m_currentComp = m_compInterleaving_1;
        break;
      case 2:
        m_currentComp = m_compInterleaving_2;
        break;
      case 3:
        m_currentComp = m_compInterleaving_3;
        break;
      case 4:
        m_currentComp = m_compInterleaving_4;
        break;
      case 5:
        m_currentComp = m_compInterleaving_5;
        break;
    }
    //m_currentComp = m_compInterleaving[m_compIndex];
  }

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
    DBG(const size_t tc = (m_tcth >> 4) & 0x0f);
    DBG(const size_t th = m_tcth & 0x0f);
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
  size_t m_huffWritePos;
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
  }

private:
  InvHuffman      mInvHuffman;
  HuffTblDecoder  mHuffTblDecoder;
};


#endif // _INCLUDED_INVHUFFMAN_HPP
