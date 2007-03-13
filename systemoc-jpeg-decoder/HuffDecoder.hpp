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

struct ExpHuffTbl {
  //FIXME: implement stub
  int dummy;
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

/*****************************************************************************/


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
      dbgout(std::cerr)
  {
    CoSupport::Header myHeader("HuffTblDecoder> ");

    dbgout << myHeader;

    DBG_OUT("testout\n");
    
    main
      // collect data
      = (in(1) && !GUARD(HuffTblDecoder::sufficientData) ) >>
        CALL(HuffTblDecoder::collect)                      >> main
      | // transform collected data
        ( GUARD(HuffTblDecoder::sufficientData) )           >>
        (outHuffTblAC0(1) && outHuffTblDC0(1) &&
         outHuffTblAC1(1) && outHuffTblDC1(1))                >>
        CALL(HuffTblDecoder::transform)                    >> main
      ;
  }

private:
  bool sufficientData() const {
    //FIXME: dummy stub
    return false;
  }

  void collect(){
    DBG_OUT("collect() 0x" << hex << (unsigned int)in[0] << dec << endl);
    //FIXME: dummy stub
  }

  void transform(){
    //FIXME: dummy stub
  }
  
  CoSupport::DebugOstream dbgout;
  smoc_firing_state main;
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
