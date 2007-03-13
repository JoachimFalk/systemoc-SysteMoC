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

#ifndef _INCLUDED_INVBYTESTUFF_HPP
#define _INCLUDED_INVBYTESTUFF_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

/**
 * F.1.2.3 Byte stuffing
 * ---------------------
 *
 * In order to provide code space for marker codes which can be located in the
 * compressed image data without decoding, byte stuffing is used.
 *
 * Whenever, in the course of normal encoding, the byte value X’FF’ is created
 * in the code string, a X’00’ byte is stuffed into the code string.
 *
 * If a X’00’ byte is detected after a X’FF’ byte, the decoder must discard it.
 * If the byte is not zero, a marker has been detected, and shall be
 * interpreted to the extent needed to complete the decoding of the scan.
 *
 * Byte alignment of markers is achieved by padding incomplete bytes with
 * 1-bits. If padding with 1-bits creates a X’FF’ value, a zero byte is stuffed
 * before adding the marker.
 */
class InvByteStuff: public smoc_actor {
public:
  smoc_port_in<JpegChannel_t>  in;
  smoc_port_out<JpegChannel_t> out;
private:
  bool detectFF() const {
    return 0xFF == JS_DATA_GET( in[0] );
  }

  bool detect00() const {
    return 0x00 == JS_DATA_GET( in[0] );
  }

  // forward control commands from input to output
  void forwardCtrl() {
    out[0] = in[0];
  }

  // forward data from input to output
  void forwardData() {
    out[0] = in[0];
  }

  smoc_firing_state main, discardZero;
public:
  InvByteStuff(sc_module_name name)
    : smoc_actor(name, main) {
    main
      // forward control tokens
      = ( in(1) && JS_ISCTRL(in.getValueAt(0)) )     >>
        out(1)                                       >>
        CALL(InvByteStuff::forwardCtrl)              >> main
      | // detect 0xFF -> discard next 0x00 (inv. stuffing)
        ( in(1) && !JS_ISCTRL(in.getValueAt(0))      &&
          GUARD(InvByteStuff::detectFF) )            >>
        out(1)                                       >>
        CALL(InvByteStuff::forwardData)              >> discardZero
      | // forward data
        ( in(1) && !JS_ISCTRL(in.getValueAt(0))      &&
          !GUARD(InvByteStuff::detectFF) )           >>
        out(1)                                       >>
        CALL(InvByteStuff::forwardData)              >> main
      ;
    discardZero
      = // discard 0x00 (inv. stuffing)
        ( in(1) && !JS_ISCTRL(in.getValueAt(0))      &&
          GUARD(InvByteStuff::detect00) )            >> main
      | // forward marker (anything except 0x00)
        ( in(1) && !JS_ISCTRL(in.getValueAt(0))      &&
          !GUARD(InvByteStuff::detect00) )           >>
        out(1)                                       >>
      CALL(InvByteStuff::forwardData)                >> main
      ;
  }
};

#endif // _INCLUDED_INVBYTESTUFF_HPP
