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

#ifndef _INCLUDED_INV_QUANT_HPP
#define _INCLUDED_INV_QUANT_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

#include "debug_config.h"
#include <cosupport/smoc_debug_out.hpp>
// if compiled with DBG_INV_QT create stream and include debug macros
#ifdef DBG_INV_QT
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif


class InvQuant: public smoc_actor {
public:
  smoc_port_in<JpegChannel_t>      in;
  smoc_port_out<JpegChannel_t>     out;

  smoc_port_in<qt_table_t>        qt_table_0;
  smoc_port_in<qt_table_t>        qt_table_1;
  smoc_port_in<qt_table_t>        qt_table_2;
  smoc_port_in<qt_table_t>        qt_table_3;

private:

  void discard_qt_table_0(){
    DBG_OUT("Discard QT table with ID 0" << endl);

    //forward control word
    forward_command();
  }

  void discard_qt_table_1(){
    DBG_OUT("Discard QT table with ID 1" << endl);

    //forward control word
    forward_command();
  }

  void discard_qt_table_2(){
    DBG_OUT("Discard QT table with ID 2" << endl);

    //forward control word
    forward_command();
  }

  void discard_qt_table_3(){
    DBG_OUT("Discard QT table with ID 3" << endl);

    //forward control word
    forward_command();
  }

  void illegal_qt_id(){    
    DBG_OUT("Found illegal QT table"  << endl);
  }

  /// Stores for each colour component which QT table to use
  // unsigned char qmap_id[JPEG_MAX_COLOR_COMPONENTS]; // Arrays not supported for Synthesis
  unsigned char qmap_id_0;
  unsigned char qmap_id_1;
  unsigned char qmap_id_2;
  unsigned char qtbl_id;

  void use_qt_table() {
    DBG_OUT("Store QT index which has to be used" << endl);
    DBG_OUT(CoSupport::Indent::Up);
    
    IntCompID_t comp_id = JS_CTRL_USEQT_GETCOMPID(in[0]);
    
    //assert(comp_id >= 0);
    //Check, that component ID is positive
    IntCompID_t temp = (IntCompID_t)-1;
    assert(temp > 0);
    assert(comp_id < JPEG_MAX_COLOR_COMPONENTS);
    
    // qmap_id[comp_id] = JS_CTRL_USEQT_GETQTID(in[0]); // Arrays not supported for Synthesis
    switch(comp_id) {
      case 0:
        qmap_id_0 = JS_CTRL_USEQT_GETQTID(in[0]);
        break;
      case 1:
        qmap_id_1 = JS_CTRL_USEQT_GETQTID(in[0]);
        break;
      case 2:
        qmap_id_2 = JS_CTRL_USEQT_GETQTID(in[0]);
        break;
    }
    // DBG_OUT("Component " << comp_id << ": " << qmap_id[comp_id] << endl); // Arrays not supported for Synthesis
    
    DBG_OUT(CoSupport::Indent::Down);
    
    forward_command();
  }

  /// Stores which component will follow next
  IntCompID_t comp_id;
  void store_comp_id(){
    DBG_OUT("Store which component will follow next" << endl);
    DBG_OUT(CoSupport::Indent::Up);
    
    //forward control word
    forward_command();
    
    comp_id = JS_CTRL_INTERNALCOMPSTART_GETCOMPID(in[0]);
    // qtbl_id = qmap_id[comp_id]; // Arrays not supported for Synthesis
    assert(comp_id < 3);
    switch(comp_id) {
      case 0:
        qtbl_id = qmap_id_0;
        break;
      case 1:
        qtbl_id = qmap_id_1;
        break;
      case 2:
        qtbl_id = qmap_id_2;
        break;
    }

    DBG_OUT("Next component: " << comp_id << endl);
    DBG_OUT("Next QT Table: "  << qtbl_id << endl);
    
    DBG_OUT(CoSupport::Indent::Down);
  }

  ///Forward commands
  void forward_command(){
    DBG_OUT("Forward command" << endl);
    out[0] = in[0];
  }


  /// Perform QT  
  void quantize0(){ quantize(0); }
  void quantize1(){ quantize(1); }
  void quantize2(){ quantize(2); }
  void quantize3(){ quantize(3); }
  void quantize(unsigned int qt_tb){
    DBG_OUT("Perform QT with table ID " << qt_tb  << endl);
    if (block_pixel_id == 1){
      DBG_OUT("input DC coefficient: " << JS_QCOEFF_GETIDCTCOEFF(in[0]) << std::endl);
    }

    IDCTCoeff_t temp;
    switch (qt_tb){
    case 0:
      temp = JS_QCOEFF_GETIDCTCOEFF(in[0]) * qt_table_0[block_pixel_id];
      if (block_pixel_id == 1)
	DBG_OUT("DC QT-Table value: " << qt_table_0[block_pixel_id] << std::endl);
      break;
    case 1:
      temp = JS_QCOEFF_GETIDCTCOEFF(in[0]) * qt_table_1[block_pixel_id];
      if (block_pixel_id == 1)
	DBG_OUT("DC QT-Table value: " << qt_table_1[block_pixel_id] << std::endl);
      break;
    case 2:
      temp = JS_QCOEFF_GETIDCTCOEFF(in[0]) * qt_table_2[block_pixel_id];
      if (block_pixel_id == 1)
	DBG_OUT("DC QT-Table value: " << qt_table_2[block_pixel_id] << std::endl);
      break;
    case 3:
      temp = JS_QCOEFF_GETIDCTCOEFF(in[0]) * qt_table_3[block_pixel_id];
      if (block_pixel_id == 1)
	DBG_OUT("DC QT-Table value: " << qt_table_3[block_pixel_id] << std::endl);
      break;
    default:
      DBG_OUT("Illegal QT table!" << qt_tb  << endl);
    }

    out[0] = JS_DATA_COEFF_SET_CHWORD(temp);

    if (block_pixel_id == 1)
      DBG_OUT("DC coefficient: " << temp << std::endl);

    block_pixel_id++;
    if (block_pixel_id > JPEG_BLOCK_SIZE)
      //by running the counter from 1 to 64,
      //we automatically skip the table header!
      block_pixel_id = 1;
  }

  //Which pixel to process
  unsigned char block_pixel_id;


  smoc_firing_state main;
  smoc_firing_state stuck;


#ifdef DBG_ENABLE
  CoSupport::DebugOstream dbgout;
#endif // DBG_ENABLE

  
public:
  InvQuant(sc_module_name name)
    : smoc_actor(name, main),
      comp_id(0),
      block_pixel_id(1) //counter runs from 1 to JPEG_BLOCK_SIZE
#ifdef DBG_ENABLE
      , dbgout(std::cerr)
#endif // DBG_ENABLE
  {
#ifdef DBG_ENABLE
    //Set Debug ostream options
    CoSupport::Header my_header("InvQuant> ");
    dbgout << my_header;
#endif // DBG_ENABLE
    
    //Init qmap_id[JPEG_MAX_COLOR_COMPONENTS]
    /* Arrays not supported for Synthesis
    for (unsigned int i = 0; i < JPEG_MAX_COLOR_COMPONENTS; i++) {
      qmap_id[i] = 0;
    }
    */
    qmap_id_0 = qmap_id_1 = qmap_id_2 = 0;
    
    main =
      /* discard Huffman tables */
      (( in(1) && qt_table_0(JS_QT_TABLE_SIZE) && out(1)) >>
       (JS_ISCTRL(in.getValueAt(0)) && 
	(JS_GETCTRLCMD(in.getValueAt(0)) == (JpegChannel_t)CTRLCMD_DISCARDQT) &&
	(JS_CTRL_DISCARDQT_GETQTID(in.getValueAt(0)) == (JpegChannel_t)0)) >>
       CALL(InvQuant::discard_qt_table_0)) >> main
      |(( in(1) && qt_table_1(JS_QT_TABLE_SIZE) && out(1)) >>
	(JS_ISCTRL(in.getValueAt(0)) && 
	 (JS_GETCTRLCMD(in.getValueAt(0)) == (JpegChannel_t)CTRLCMD_DISCARDQT) &&
	 (JS_CTRL_DISCARDQT_GETQTID(in.getValueAt(0)) == (JpegChannel_t)1)) >>
	CALL(InvQuant::discard_qt_table_1)) >> main
      |(( in(1) && qt_table_2(JS_QT_TABLE_SIZE) && out(1)) >>
	(JS_ISCTRL(in.getValueAt(0)) && 
	 (JS_GETCTRLCMD(in.getValueAt(0)) == (JpegChannel_t)CTRLCMD_DISCARDQT) &&
	 (JS_CTRL_DISCARDQT_GETQTID(in.getValueAt(0)) == (JpegChannel_t)2)) >>
	CALL(InvQuant::discard_qt_table_2)) >> main
      |(( in(1) && qt_table_3(JS_QT_TABLE_SIZE) && out(1)) >>
	(JS_ISCTRL(in.getValueAt(0)) && 
	 (JS_GETCTRLCMD(in.getValueAt(0)) == (JpegChannel_t)CTRLCMD_DISCARDQT) &&
	 (JS_CTRL_DISCARDQT_GETQTID(in.getValueAt(0)) == (JpegChannel_t)3)) >>
	CALL(InvQuant::discard_qt_table_3)) >> main
      // Check for illegal QT table IDs
      |(( in(1) && out(1)) >>
	(JS_ISCTRL(in.getValueAt(0)) && 
	 (JS_GETCTRLCMD(in.getValueAt(0)) == (JpegChannel_t)CTRLCMD_DISCARDQT) &&
	 (JS_CTRL_DISCARDQT_GETQTID(in.getValueAt(0)) > (JpegChannel_t)3)) >>
	CALL(InvQuant::illegal_qt_id)) >> stuck

      /* store QT table to use */
      |(( in(1) && out(1)) >>
	(JS_ISCTRL(in.getValueAt(0)) && 
	 (JS_GETCTRLCMD(in.getValueAt(0)) == (JpegChannel_t)CTRLCMD_USEQT)) >>
	CALL(InvQuant::use_qt_table)) >> main

      /* Store which colour component to process */      
      |(( in(1) && out(1)) >>
	(JS_ISCTRL(in.getValueAt(0)) && 
	 (JS_GETCTRLCMD(in.getValueAt(0)) == (JpegChannel_t)CTRLCMD_INTERNALCOMPSTART)) >>
	CALL(InvQuant::store_comp_id)) >> main
      
      /* Forward non processed commands */
      |(( in(1) && out(1)) >>
	(JS_ISCTRL(in.getValueAt(0)) && 
	 (JS_GETCTRLCMD(in.getValueAt(0)) != (JpegChannel_t)CTRLCMD_INTERNALCOMPSTART) &&
	 (JS_GETCTRLCMD(in.getValueAt(0)) != (JpegChannel_t)CTRLCMD_USEQT) &&
	 (JS_GETCTRLCMD(in.getValueAt(0)) != (JpegChannel_t)CTRLCMD_DISCARDQT)
	 ) >>
	CALL(InvQuant::forward_command)) >> main

      /* Process data value */
      |(( in(1) && out(1) && qt_table_0(0,JS_QT_TABLE_SIZE)) >>
	((!JS_ISCTRL(in.getValueAt(0)))  &&
	 (VAR(qtbl_id) == 0)) >>
	CALL(InvQuant::quantize0)) >> main

      |(( in(1) && out(1) && qt_table_1(0,JS_QT_TABLE_SIZE)) >>
	((!JS_ISCTRL(in.getValueAt(0)))  &&
	 (VAR(qtbl_id) == 1)) >>
	CALL(InvQuant::quantize1)) >> main

      |(( in(1) && out(1) && qt_table_2(0,JS_QT_TABLE_SIZE)) >>
	((!JS_ISCTRL(in.getValueAt(0)))  &&
	 (VAR(qtbl_id) == 2)) >>
	CALL(InvQuant::quantize2)) >> main

      |(( in(1) && out(1) && qt_table_3(0,JS_QT_TABLE_SIZE)) >>
	((!JS_ISCTRL(in.getValueAt(0)))  &&
	 (VAR(qtbl_id) == 3)) >>
	CALL(InvQuant::quantize3)) >> main;   

      }
};

#endif // _INCLUDED_INV_QUANT_HPP
