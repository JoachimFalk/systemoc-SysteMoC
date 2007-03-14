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
  // FIXME: only 16 values are needed! for easy reading codelength is index
  uint8_t         valPtr[17];   // value pointers to first symbol of codelength
                                //  'index'
  uint8_t         minCode[17];  // minimal code value for codewords of length
                                //  'index'
  uint8_t         maxCode[17];  // max ...
  DecodedSymbol_t huffVal[256]; // symbol-length assignment parameters (B.2.4.2)
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
  // FIXME: compile dummy
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
      ;
  }

private:
  //
  bool thisMattersMe() const {
    //FIXME: dummy stub
    return false;
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
    : smoc_actor(name, main),
      m_length(0),
      m_huffWritePos(0),
      dbgout(std::cerr)
  {
    CoSupport::Header myHeader("HuffTblDecoder> ");

    dbgout << myHeader;

    main
      // collect data
      = ( in(2) &&  IS_DHT_SYNC(in.getValueAt(0)) ) >>
        CALL(HuffTblDecoder::start)                 >> waitLength;

    waitLength
      //read length 
      = in(2)                                       >>
        CALL(HuffTblDecoder::readLengthField)       >> waitTcTh;

    waitTcTh
      = in(1)                                       >>
        CALL(HuffTblDecoder::storeTcTh)             >> waitBITS;

    waitBITS
      = in(16)                                      >>
        CALL(HuffTblDecoder::storeBITS)             >> waitHUFFVAL;

    waitHUFFVAL
      = ( in(1) &&
          GUARD(HuffTblDecoder::hasMoreHUFFVAL) )   >>
        CALL(HuffTblDecoder::storeHUFFVAL)          >> waitHUFFVAL
      | ( !GUARD(HuffTblDecoder::hasMoreHUFFVAL)    &&
         GUARD(HuffTblDecoder::isTable)(AC0) )      >>
        outHuffTblAC0(1)                            >>
        // FIXME: avoid parameterized functions for synthesis
        CALL(HuffTblDecoder::writeTable)(outHuffTblAC0) >> main;
    // FIXME: x4

    /*waitDC0Data
      = in(16)

    waitTcTh
      // read Tc & Th
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
    return m_length > 0;
  }

  bool isTable(const HuffTableType type) const {
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

  void start(){
  }

  void storeTcTh() {
    m_tcth = in[0];
  }

  void readLengthField() {
    // Length Field is 16 bit
    m_length = in[0]*0x100 + in[1];
    // subtract length field from total length
    m_length -= 2;
  }

  void storeBITS() {
    for (int i = 0; i < 16; ++i)
      m_BITS[i] = in[i];

    m_length -= 16;
  }

  void storeHUFFVAL() {
    m_tmpHuff.huffVal[m_huffWritePos++] = in[0];
  }

  void writeTable(smoc_port_out<ExpHuffTbl> &out) {
    assert(m_length == 0);

    // generate ValPtr
    m_tmpHuff.valPtr[0] = 0;
    int offset = 0;
    for (int i = 1; i < 16; ++i) {
      m_tmpHuff.valPtr[i - 1] = offset + in[i];
      offset += in[i];
    }

    out[0] = m_tmpHuff;

    // reset tmpHuff
    m_huffWritePos = 0;
  }

  /*void ...() {
    m_tmpHuff.valPtr[1] = 0;

    int offset = 0;
    for (int i = 2; i < 17; ++i) {
      out[0].valPtr[i] = offset + in[i - 1];
      offset += in[i - 1];
    }
  }*/

  void decLength() {
    m_length--;
  } 

  /*void collect(bool last){
    //FIXME: dummy stub
    decLength();

    DBG_OUT("| " << hex << (unsigned int)in[0] << dec);
    if(last) DBG_OUT(endl);
  }*/

  void transform(){
    //FIXME: dummy stub
  }
  
  uint16_t m_length;
  uint8_t  m_tcth;

  uint8_t  m_BITS[16];

  ExpHuffTbl m_tmpHuff;

  size_t m_huffWritePos;

  CoSupport::DebugOstream dbgout;
  smoc_firing_state main;
  smoc_firing_state waitLength;
  smoc_firing_state waitTcTh;
  smoc_firing_state waitHUFFVAL;
  smoc_firing_state waitBITS;
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
