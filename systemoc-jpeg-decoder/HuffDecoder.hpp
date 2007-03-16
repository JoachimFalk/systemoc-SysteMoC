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

#include "channels.hpp"

#define HUFF_GET_CATEGORY(x) ((x) && 0x0F)
#define HUFF_GET_RUNLENGTH(x) ((x) && 0xF0)

#define HUFF_EOB 0x00
#define HUFF_RUNLENGTH_ZERO_AMPLITUDE 0xF0
#define NEXTBIT_MAX_CLAIM 3

/// Debug dht synchronisation
#define IS_DHT_SYNC(x) (0xF0 == (x))


struct ExpHuffTbl {
  uint8_t          valPtr[16];   // value pointers to first symbol of codelength
                                 //  'index'
  uint16_t         minCode[16];  // minimal code value for codewords of length
                                 //  'index'
  uint16_t         maxCode[16];  // max ...
  DecodedSymbol_t  huffVal[256]; // symbol-length assignment parameters (B.2.4.2)
};

// if compiled with DBG_HUFF_DECODER create stream and include debug macros
#define DBG_HUFF_DECODER
#ifdef DBG_HUFF_DECODER
  #include <cosupport/smoc_debug_out.hpp>
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

  InvHuffman(sc_module_name name)
    : smoc_actor(name, main),
      compIndex(0),
      nextBitIndex(0),
      dbgout(std::cerr),
    dbgbuff(Debug::Low)
  {
  CoSupport::Header myHeader("InvHuffman> ");

  dbgout << myHeader;
  dbgout.insert(dbgbuff);

    main
      // ignore and forward control tokens
      = ( in(1) && JS_ISCTRL(in.getValueAt(0))                    &&
          !GUARD(InvHuffman::isUseHuff)                           &&
          !GUARD(InvHuffman::isDiscardHuff)                       &&
          !GUARD(InvHuffman::isNewScan) )                         >>
        out(1)                                                    >>
        CALL(InvHuffman::forwardCtrl)                             >> main
      | // treat and forward USEHUFF CTRLs                                    
        ( in(1) && JS_ISCTRL(in.getValueAt(0))                    &&
          GUARD(InvHuffman::isUseHuff) )                          >>
        out(1)                                                    >>
        CALL(InvHuffman::useHuff)                                 >> main
      | // treat and forward NEWSCAN CTRLs                                    
        ( in(1) && JS_ISCTRL(in.getValueAt(0))                    &&
          GUARD(InvHuffman::isNewScan) )                          >>
        out(1)                                                    >>
        CALL(InvHuffman::setCompInterleaving)                     >> main
      | // decode DC
        ( in(0, 1) && !JS_ISCTRL(in.getValueAt(0)) ) >> dcDecoding
      | // discard HuffTable AC0
        ( inHuffTblAC0(1) && in(1)                              &&
          JS_ISCTRL(in.getValueAt(0))                           &&
          GUARD(InvHuffman::isDiscardHuff)                      &&
          GUARD(InvHuffman::isHuffTblId)(HUFFTBL_AC)(0) )       >>
        out(1)                                                  >>
        CALL(InvHuffman::discardHuff)(inHuffTblAC0)             >> main
      | // discard HuffTable AC1
        ( inHuffTblAC1(1) && in(1)                              &&
          JS_ISCTRL(in.getValueAt(0))                           &&
          GUARD(InvHuffman::isDiscardHuff)                      &&
          GUARD(InvHuffman::isHuffTblId)(HUFFTBL_AC)(1) )       >>
        out(1)                                                  >>
        CALL(InvHuffman::discardHuff)(inHuffTblAC1)             >> main
      | // discard HuffTable DC0
        ( inHuffTblDC0(1) && in(1)                              &&
          JS_ISCTRL(in.getValueAt(0))                           &&
          GUARD(InvHuffman::isDiscardHuff)                      &&
          GUARD(InvHuffman::isHuffTblId)(HUFFTBL_DC)(0) )       >>
        out(1)                                                  >>
        CALL(InvHuffman::discardHuff)(inHuffTblDC0)             >> main
      | // discard HuffTable DC1
        ( inHuffTblDC1(1)  && in(1)                             &&
          JS_ISCTRL(in.getValueAt(0))                           &&
          GUARD(InvHuffman::isDiscardHuff)                      &&
          GUARD(InvHuffman::isHuffTblId)(HUFFTBL_DC)(1) )       >>
        out(1)                                                  >>
        CALL(InvHuffman::discardHuff)(inHuffTblDC1)             >> main
      ;

    dcDecoding 
      = // decodeDC
      ( in(0, NEXTBIT_MAX_CLAIM) && inHuffTblDC0(0,1)           && 
        !JS_ISCTRL(in.getValueAt(0))                            &&
        !GUARD(InvHuffman::needToClaimBits) )                   >>
      out(1)                                                    >>
      CALL(InvHuffman::decodeDC)(inHuffTblDC0)                  >> acDecoding
      | // consum token if 8 or more bits are processed 
      ( in(1)                                                   && 
        !JS_ISCTRL(in.getValueAt(0))                            &&
        GUARD(InvHuffman::needToClaimBits) )                    >>
        CALL(InvHuffman::claimBits)                             >> dcDecoding
      | // go back to main if CTRL is found
      ( in(0,1)                                                 &&
        JS_ISCTRL(in.getValueAt(0)) )                           >> main;
 
    acDecoding // decodeAC
      = ( in(0, NEXTBIT_MAX_CLAIM) && inHuffTblAC0(0,1)         &&
          !JS_ISCTRL(in.getValueAt(0))                          &&
          !GUARD(InvHuffman::needToClaimBits) )                 >>
        out(1)                                                  >>
        CALL(InvHuffman::decodeAC)(inHuffTblAC0)                >> acDecoding
      | // consum token if 8 or more bits are processed 
      ( in(1)                                                   && 
        !JS_ISCTRL(in.getValueAt(0))                            &&
        GUARD(InvHuffman::needToClaimBits) )                    >>
        CALL(InvHuffman::claimBits)                             >> acDecoding
      | // go back to main if CTRL is found
      ( in(0,1)                                                 &&
        JS_ISCTRL(in.getValueAt(0)) )                           >> main
      | // after reading 63 codeworts or R == 15
      ( in(0,1) &&
        GUARD(InvHuffman::isNextBlock) )  >>
        CALL(InvHuffman::nextBlock) >> dcDecoding
      ;
  }

private:
  //
  bool isNextBlock() const {
    //FIXME:
    return false;
  }

  //
  bool isUseHuff() const {
    return (JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_USEHUFF);
  }

  // consum token if 8 or more bits are processed 
  bool needToClaimBits() const {
    return (nextBitIndex > 7);
  }

  //
  bool isNewScan() const {
    cerr << "isNewScan() "
         << (JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_NEWSCAN)
         << endl;

    return (JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_NEWSCAN);
  }

  //
  bool isDiscardHuff() const {
    cerr << "isDiscardHuff() "
         << ( (JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_DISCARDHUFF))
         << endl;

    return (JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_DISCARDHUFF);
  }

  //
  bool isHuffTblId(HuffTblType_t type, HuffTblID_t id) const {
    cerr << "isHuffTblId() "
         << ( (JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) == id) &&
              (JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) == type) )
         << endl;
    assert(JS_GETCTRLCMD(in[0]) == (JpegChannel_t)CTRLCMD_DISCARDHUFF);

    return ( (JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) == id) &&
             (JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) == type) );
  }

  // decodeDC
  void decodeDC(const smoc_port_in<ExpHuffTbl> &table){
    DBG_OUT("decodeDC() " << in[0] << endl);
    bit_t b = nextBit();
    //FIXME: dummy stub
    out[0] = in[0];

  }

  //
  bit_t nextBit() {
    DBG_OUT("nextBit()\n");
    int fifoIdx = nextBitIndex/8;
    int wordIdx = nextBitIndex%8;
    assert(fifoIdx < NEXTBIT_MAX_CLAIM);
    bit_t b = UDEMASK(in[fifoIdx], wordIdx, 1);
    ++nextBitIndex;
    return b;
  }

  void nextBlock() {
    // select next component
    compIndex = (compIndex+1)%SCANPATTERN_LENGTH;

    // select next HuffTables??
  }

  // decodeAC
  void decodeAC(const smoc_port_in<ExpHuffTbl> &table){
    DBG_OUT("decodeAC() " << in[0] << endl);
    bit_t b = nextBit();
    //FIXME: dummy stub
    out[0] = in[0];
  }

  // decode
  void decode(const smoc_port_in<ExpHuffTbl> &in){
    
  }

  //
  void claimBits(){
    DBG_OUT("claimBits() " << endl);
    nextBitIndex-=8;
  }

  //
  void setCompInterleaving(){
    // forward CTRL
    out[0] = in[0];

    DBG_OUT("setCompInterleaving()");
    for(int i = 0; i < SCANPATTERN_LENGTH; ++i){
      m_compInterleaving[i] = JS_CTRL_NEWSCAN_GETCOMP(in[0],i);
      DBG_OUT(" " << m_compInterleaving[i]);
    }
    DBG_OUT(endl);
  }

  //
  void useHuff() {
    // forward CTRL
    out[0] = in[0];

    IntCompID_t cmp = JS_CTRL_USEHUFF_GETCOMP(in[0]);
    HuffTblID_t  dc = JS_CTRL_USEHUFF_GETDCTBL(in[0]);
    HuffTblID_t  ac = JS_CTRL_USEHUFF_GETACTBL(in[0]);
    DBG_OUT("useHuff() c: " << cmp << " dc: " << dc << " ac: " << ac << endl);
    m_useHuffTableAc[cmp] = ac;
    m_useHuffTableDc[cmp] = dc;
  }

  //
  void discardHuff(const smoc_port_in<ExpHuffTbl> &tableIn) {
    // forward CTRL
    out[0] = in[0];

    DBG_OUT("discardHuff() ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETHUFFID(in[0]) << " ");
    DBG_OUT(JS_CTRL_DISCARDHUFFTBL_GETTYPE(in[0]) << " ");
    DBG_OUT("\n");
  
    DBG_OUT(tableIn[0] << endl);
  }

  // forward control commands from input to output
  void forwardCtrl() {
    DBG_OUT("forwardCtrl() " << in[0]  << endl);
    out[0] = in[0];
  }

  int compIndex;
  int nextBitIndex;
  IntCompID_t m_compInterleaving[SCANPATTERN_LENGTH];      //6
  HuffTblID_t m_useHuffTableAc[JPEG_MAX_COLOR_COMPONENTS]; //3
  HuffTblID_t m_useHuffTableDc[JPEG_MAX_COLOR_COMPONENTS]; //3
  CoSupport::DebugOstream dbgout;
  CoSupport::DebugStreambuf dbgbuff;
  smoc_firing_state main;
  smoc_firing_state acDecoding;
  smoc_firing_state dcDecoding;
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
  bool hasMoreHUFFVAL() const {
    return m_symbolsLeft > 0;
  }

  bool isTable(const HuffTableType type) const;

  void storeTcTh() {
    DBG_OUT("storeTcTh()\n");
    m_tcth = in[0];
  }

  void storeBITS();

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
  CoSupport::DebugStreambuf dbgbuff;
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
