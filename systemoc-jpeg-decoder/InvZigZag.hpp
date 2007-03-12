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

#ifndef _INCLUDED_INV_ZIGZAG_HPP
#define _INCLUDED_INV_ZIGZAG_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

class InvZigZag: public smoc_actor {
public:
  smoc_port_in<JpegChannel_t>      in;
  smoc_port_out<JpegChannel_t>     out;
private:
  bool thisMattersMe() const {
    //FIXME: dummy stub
    return false;
  }

  void doSomething(){
    //FIXME: dummy stub

    forwardCtrl();
  }

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

  smoc_firing_state main;
public:
  InvZigZag(sc_module_name name)
    : smoc_actor(name, main) {
    main
      // ignore and forward control tokens
      = ( in(1) && JS_ISCTRL(in.getValueAt(0))       &&
          !GUARD(InvZigZag::thisMattersMe) )        >>
        out(1)                                       >>
        CALL(InvZigZag::forwardCtrl)                >> main
      | // treat and forward control tokens
        ( in(1) && JS_ISCTRL(in.getValueAt(0))       &&
          GUARD(InvZigZag::thisMattersMe) )         >>
        out(1)                                       >>
        CALL(InvZigZag::doSomething)                >> main
      | // data transformation
        ( in(1) && !JS_ISCTRL(in.getValueAt(0)) )    >>
        out(1)                                       >>
        CALL(InvZigZag::transform)                  >> main
      ;
  }
};

#endif // _INCLUDED_INV_ZIGZAG_HPP
