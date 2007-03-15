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
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

#define HUFF_GET_CATEGORY(x) ((x) && 0x0F)
#define HUFF_GET_RUNLENGTH(x) ((x) && 0xF0)

#define HUFF_EOB 0x00
#define HUFF_RUNLENGTH_ZERO_AMPLITUDE 0xF0

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


#define IS_TABLE_CLASS_DC(v) ((v) & 0xF0 == 0x00)
#define IS_TABLE_CLASS_AC(v) ((v) & 0xF0 == 0x10)

#define IS_TABLE_DEST_ZERO(v) ((v) & 0x0F == 0x00)
#define IS_TABLE_DEST_ONE(v) ((v) & 0x0F == 0x01)

enum HuffTableType {
  AC0,
  AC1,
  DC0,
  DC1
};

/*****************************************************************************/

// needed by SysteMoC!
ostream &operator<<(ostream &out, const ExpHuffTbl &eht) {
  out << hex << "valPtr:";
  for(int i = 0; i<16; ++i){
    out << " | " << eht.valPtr[i];
  }
 
  out << " |\nminCode:";
  for(int i = 0; i<16; ++i){
    out << " | " << eht.minCode[i];
  }
 
  out << " |\nmaxCode:";
  for(int i = 0; i<16; ++i){
    out << " | " << eht.maxCode[i];
  }

  out << " |\nhuffVal:";
  for(int i = 0; i<256; ++i){
    out << " | " << eht.huffVal[i];
  }
  out << " |\n";
  out << dec;
  return out;
}


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
      dbgout(std::cerr)
  {
    main
      // ignore and forward control tokens
      = ( in(1) && JS_ISCTRL(in.getValueAt(0))       &&
          !GUARD(InvHuffman::thisMattersMe) )        >>
        out(1)                                       >>
        CALL(InvHuffman::forwardCtrl)                >> main
      | // treat and forward control tokens
        ( in(1) && JS_ISCTRL(in.getValueAt(0))       &&
          GUARD(InvHuffman::thisMattersMe) )         >>
        out(1)                                       >>
        CALL(InvHuffman::doSomething)                >> main
      | // data transformation
        ( in(1) && !JS_ISCTRL(in.getValueAt(0)) )    >>
        out(1)                                       >>
        CALL(InvHuffman::transform)                  >> main
      | // debug test
      inHuffTblAC0(1)                                >>
        CALL(InvHuffman::debugDump)(inHuffTblAC0)    >> main
       ;
  }

private:
  //
  bool thisMattersMe() const {
    //FIXME: dummy stub
    return false;
  }

  void debugDump(const smoc_port_in<ExpHuffTbl> &in) {
    DBG_OUT(in[0]);
  }
  //
  void doSomething(){
    //FIXME: dummy stub

    forwardCtrl();
  }

  //
  void transform(){
    //FIXME: dummy stub
    out[0] = in[0];
  }
  
  // forward control commands from input to output
  void forwardCtrl() {
    out[0] = in[0];
  }

  // forward data from input to output
  void forwardData() {
    out[0] = in[0];
  }

  CoSupport::DebugOstream dbgout;
  smoc_firing_state main;
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

  HuffTblDecoder(sc_module_name name)
    : smoc_actor(name, waitTcTh),
      m_symbolsLeft(0),
      m_huffWritePos(0),
      dbgout(std::cerr)
  {
    CoSupport::Header myHeader("HuffTblDecoder> ");

    dbgout << myHeader;

    waitTcTh
      // read Tc & Th (8 bit)
      = in(1)                                       >>
        CALL(HuffTblDecoder::storeTcTh)             >> waitBITS;

    waitBITS
      // read BITS table (16 x 8 bit)
      = in(16)                                      >>
        CALL(HuffTblDecoder::storeBITS)             >> waitHUFFVAL;

    waitHUFFVAL
      // read HUFFVAL (length x 8 bit)
      = ( in(1) &&
          GUARD(HuffTblDecoder::hasMoreHUFFVAL) )   >>
        CALL(HuffTblDecoder::storeHUFFVAL)          >> waitHUFFVAL
      | !GUARD(HuffTblDecoder::hasMoreHUFFVAL)      >> 
        CALL(HuffTblDecoder::finishTable)           >> s_writeTable;
  
    s_writeTable
      = GUARD(HuffTblDecoder::isTable)(AC0)         >>
        outHuffTblAC0(1)                            >>
        CALL(HuffTblDecoder::writeTable)(outHuffTblAC0) >> waitTcTh
      | GUARD(HuffTblDecoder::isTable)(AC1)         >>
        outHuffTblAC1(1)                            >>
        CALL(HuffTblDecoder::writeTable)(outHuffTblAC1) >> waitTcTh
      | GUARD(HuffTblDecoder::isTable)(DC0)         >>
        outHuffTblDC0(1)                            >>
        CALL(HuffTblDecoder::writeTable)(outHuffTblDC0) >> waitTcTh
      | GUARD(HuffTblDecoder::isTable)(DC1)         >>
        outHuffTblDC1(1)                            >>
        CALL(HuffTblDecoder::writeTable)(outHuffTblDC1) >> waitTcTh;

    /*waitDC0Data
      = in(16)

    waitTcTh
      = in(1)                                       >>
        CALL(HuffTblDecoder::interpretTcTh)         >> waitData
    waitData
      // read data
      = (in(1) && (VAR(m_length)!=1))               >>
        CALL(HuffTblDecoder::collect)(false)        >> waitData
      | // read last byte 
        (in(1) && (VAR(m_length)==1))               >>
        CALL(HuffTblDecoder::collect)(true)         >> main
      ;*/

  }

private:
  bool hasMoreHUFFVAL() const {
    return m_symbolsLeft > 0;
  }

  bool isTable(const HuffTableType type) const {
    assert(IS_TABLE_CLASS_AC(m_tcth) || IS_TABLE_CLASS_DC(m_tcth));
    assert(IS_TABLE_DEST_ZERO(m_tcth) || IS_TABLE_DEST_ONE(m_tcth));
    if (IS_TABLE_CLASS_AC(m_tcth))
      if (IS_TABLE_DEST_ZERO(m_tcth))
        return type == AC0;
      else
        return type == AC1;
    else
      if (IS_TABLE_DEST_ZERO(m_tcth))
        return type == DC0;
      else
        return type == DC1;
  }

  void storeTcTh() {
    DBG_OUT("storeTcTh()\n");
    m_tcth = in[0];
  }

  void storeBITS() {
    DBG_OUT("storeBITS()\n");
    int totalCodes = 0;
    for (int i = 0; i < 16; ++i) {
      m_BITS[i] = in[i];
      totalCodes += m_BITS[i];
      //DBG_OUT("  codewords with length " << i+1 << " occure " << (int)m_BITS[i]
      //        << " times.\n");
    }
    //DBG_OBJ("  total codewords: " << totalCodes << std::endl);

    m_symbolsLeft = totalCodes;
  }

  void storeHUFFVAL() {
    assert(m_symbolsLeft);
    --m_symbolsLeft;
    //DBG_OUT("storeHUFFVAL(): write pos: " << m_huffWritePos << "; "
    //        << m_symbolsLeft << " bytes left now\n");
    m_tmpHuff.huffVal[m_huffWritePos++] = in[0];
  }

#if 0
  //
  int getHUFFSIZE(const uint8_t valPtr[16], const uint8_t pos) const {
    int tablePos = m_BITS[0];
    int codeSize = 1;

    while (tablePos < pos) {
      codeSize++;
      assert(codeSize < 17);
      tablePos += m_BITS[codeSize - 1];
    }
    assert(pos <= tablePos);
    return codeSize;
  }
#endif
 
  //
  void finishTable() {
    DBG_OUT("finishTable()\n");

#if 0
    // precalculate valPtr for use of getHUFFSIZE() which is buggy right now.
    //  in future this could save HUFFSIZE[256] table.
    {
      int codePos = 0;
      for (int i = 0; i < 16; ++i) { // i + 1 == CodeSize
        if (m_BITS[i] != 0) {
          m_tmpHuff.valPtr[i] = codePos;
          codePos = codePos + m_BITS[i] - 1;
          ++codePos;
          DBG_OUT("  code size " << i + 1
                  << ", valPtr: " << (int)m_tmpHuff.valPtr[i]
                  << std::endl);
        }
      }
    }
#endif

    // HUFFSIZE table (huffmann code sizes) - p.51 C.1
    // FIXME: omit table and calculate this ad hoc to save mem
    uint4_t HUFFSIZE[256];

    int tablePos = 0;
    for (int i = 0; i < 16; ++i) {
      for (int j = 0; j < m_BITS[i]; ++j) {
        HUFFSIZE[tablePos++] = i + 1;
        //assert(HUFFSIZE[tablePos - 1] ==
        //                getHUFFSIZE(m_tmpHuff.valPtr, tablePos - 1));
      }
    }
    const int totalCodes = tablePos;

    // HUFFCODE table (huffmann codes table) - p.52 C.2
    uint16_t HUFFCODE[256];
    {
      uint16_t code = 0;
      uint4_t size = HUFFSIZE[0];

      for (int i = 0; i < totalCodes; ++i) {
        HUFFCODE[i] = code;
        ++code;
        if (HUFFSIZE[i + 1] != size) {
          code = code << (HUFFSIZE[i + 1] - size);
          size = HUFFSIZE[i + 1];
        }
      }
    }

    // MINCODE, MAXCODE, and VALPTR - p.108 F.15
    // NOTE: we use unsigned 16bit values for min and max! test for 0xffff in
    // decode() function in addition to code > maxcode(i) in F.16.
    int codePos = 0;
    DBG_OUT("Calculate 16-value tables\n");
    for (int i = 0; i < 16; ++i) { // i + 1 == CodeSize
      if (m_BITS[i] == 0) {
        m_tmpHuff.maxCode[i] = 0xffff;
        //DBG_OUT("  code size " << i + 1 << " - maxCode: "
        //        << m_tmpHuff.maxCode[i] << std::endl);
      }
      else {        
        m_tmpHuff.valPtr[i] = codePos;
        m_tmpHuff.minCode[i] = HUFFCODE[codePos];
        codePos = codePos + m_BITS[i] - 1;
        m_tmpHuff.maxCode[i] = HUFFCODE[codePos];
        ++codePos;
        /*DBG_OUT("  code size " << i + 1
                << " - maxCode: " << m_tmpHuff.maxCode[i]
                << ", minCode: " << m_tmpHuff.minCode[i]
                << ", valPtr: " << (int)m_tmpHuff.valPtr[i]
                << std::endl);*/
      }
    }

    /*
    // debug dump tables
    for (int i = 0; i < totalCodes; ++i) {
      DBG_OUT(" " << i << " - size: " << (int)HUFFSIZE[i]
              << ", code: " << HUFFCODE[i] << std::endl);
    }*/
  }

  //
  void writeTable(smoc_port_out<ExpHuffTbl> &out) {
    DBG_OUT("writeTable()\n");
    assert(m_symbolsLeft == 0);

    out[0] = m_tmpHuff;

    // "reset" tmpHuff
    m_huffWritePos = 0;

    DBG_OUT("writeTable(): done\n");
  }
  
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
      mInvHuffman.inHuffTblAC0);
    connectNodePorts(
      mHuffTblDecoder.outHuffTblAC1,
      mInvHuffman.inHuffTblAC1);
    connectNodePorts(
      mHuffTblDecoder.outHuffTblDC0,
      mInvHuffman.inHuffTblDC0);
    connectNodePorts(
      mHuffTblDecoder.outHuffTblDC1,
      mInvHuffman.inHuffTblDC1);
    mInvHuffman.out(out);
#endif
  }

private:
  InvHuffman      mInvHuffman;
  HuffTblDecoder  mHuffTblDecoder;
};


#endif // _INCLUDED_INVHUFFMAN_HPP
