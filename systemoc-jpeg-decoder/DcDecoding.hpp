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

#ifndef _INCLUDED_DC_DECODING_HPP
#define _INCLUDED_DC_DECODING_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

#include "debug_config.h"
#include <cosupport/smoc_debug_out.hpp>
#include <cosupport/smoc_debug_out.hpp>
// if compiled with DBG_DC_DECODING create stream and include debug macros
#ifdef DBG_DC_DECODING
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif

class DcDecoding: public smoc_actor {
public:
  smoc_port_in<JpegChannel_t>      in;
  smoc_port_out<JpegChannel_t>     out;
private:
  
  // Previous DC coefficient
  // One for each component
  QuantIDCTCoeff_t prev_DC[JPEG_MAX_COLOR_COMPONENTS];

  void forward_cmd(){
    DBG_OUT("Forward command" << endl);
    out[0] = in[0];
  }


  // reset previous DC coefficient
  void reset_DC(){
    DBG_OUT("Reset previous DC coefficient for all components" << endl);
    for(unsigned int i = 0; i < JPEG_MAX_COLOR_COMPONENTS; i++){
      prev_DC[i] = 0;
    }
    
    forward_cmd();
  }

  // stores the colour component which will follow next
  IntCompID_t comp_id;
  void store_color(){
    DBG_OUT("Store the colour component" << endl);
    
    comp_id = JS_CTRL_INTERNALCOMPSTART_GETCOMPID(in[0]);
    
    DBG_OUT(" (Expect colour component " << comp_id << ")" << endl);    

    forward_cmd();
  }

  unsigned char pixel_id;

  // Perform DC decoding
  void decode_dc(){
    DBG_OUT("Perform DC decoding (comp = " << comp_id << "):");
    DBG_OUT(" Input: " << JS_QCOEFF_GETIDCTCOEFF(in[0]));
    DBG_OUT(", Prev: " << prev_DC[comp_id]);
    
    //assert(comp_id >= 0);
    //Check, that comp_id is not signed
    IntCompID_t temp = (IntCompID_t)-1;
    assert(temp > 0);
    
    assert(comp_id < JPEG_MAX_COLOR_COMPONENTS);

    // Update DC value
    prev_DC[comp_id] += JS_QCOEFF_GETIDCTCOEFF(in[0]);

    out[0] = JS_DATA_QCOEFF_SET_CHWORD(prev_DC[comp_id]);

    pixel_id++;
    if(pixel_id >= JPEG_BLOCK_SIZE)
      pixel_id = 0;
    
    //output DC coefficient
    DBG_OUT(", DC= " << prev_DC[comp_id] << endl);
    
  }

  void forward_ac(){
    DBG_OUT("Forward AC coefficient" << endl);

    out[0] = in[0];

    pixel_id++;
    if(pixel_id >= JPEG_BLOCK_SIZE)
      pixel_id = 0;
  }

  smoc_firing_state main;

  CoSupport::DebugOstream dbgout;
public:
  DcDecoding(sc_module_name name)
    : smoc_actor(name, main),
      comp_id(0),
      pixel_id(0),
      dbgout(std::cerr)
  {

    //Set Debug ostream options
    CoSupport::Header my_header("DcDecoding> ");
    dbgout << my_header;

    //init previous DC
    for(unsigned int i = 0; i < JPEG_MAX_COLOR_COMPONENTS; i++){
      prev_DC[i] = 0;
    }

    main
      /* Process commands */
      = (in(1) && JS_ISCTRL(in.getValueAt(0)))       >>
      (JS_GETCTRLCMD(in.getValueAt(0)) == (JpegChannel_t)CTRLCMD_NEWSCAN)        >>
      (out(1)) >>
      CALL(DcDecoding::reset_DC)                >> main
      | (in(1) && JS_ISCTRL(in.getValueAt(0)))       >>
      (JS_GETCTRLCMD(in.getValueAt(0)) == (JpegChannel_t)CTRLCMD_SCANRESTART)        >>
      (out(1)) >>
      CALL(DcDecoding::reset_DC)                >> main
      | (in(1) && JS_ISCTRL(in.getValueAt(0)))       >>
      (JS_GETCTRLCMD(in.getValueAt(0)) == (JpegChannel_t)CTRLCMD_INTERNALCOMPSTART)        >>
      (out(1)) >>
      CALL(DcDecoding::store_color)                >> main      

      /* Forward non-processed commands */
      | (in(1) && JS_ISCTRL(in.getValueAt(0)))       >>
      ((JS_GETCTRLCMD(in.getValueAt(0)) != (JpegChannel_t)CTRLCMD_NEWSCAN) &&
       (JS_GETCTRLCMD(in.getValueAt(0)) != (JpegChannel_t)CTRLCMD_SCANRESTART) &&
       (JS_GETCTRLCMD(in.getValueAt(0)) != (JpegChannel_t)CTRLCMD_INTERNALCOMPSTART))        >>
      (out(1)) >>
      CALL(DcDecoding::forward_cmd)                >> main

      /* Perfrom inverse DC Decoding */
      | (in(1) && (!JS_ISCTRL(in.getValueAt(0))))       >>
      (VAR(pixel_id) == 0) >> 
      (out(1)) >>
      CALL(DcDecoding::decode_dc)                >> main

      /* Forward AC coefficient */
      | (in(1) && (!JS_ISCTRL(in.getValueAt(0))))       >>
      (VAR(pixel_id) != 0) >> 
      (out(1)) >>
      CALL(DcDecoding::forward_ac)                >> main;
      
  }
};

#endif // _INCLUDED_DC_DECODING_HPP
