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

#ifndef _INCLUDED_INV_ZRL_HPP
#define _INCLUDED_INV_ZRL_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

#include "debug_config.h"
#include <cosupport/smoc_debug_out.hpp>
// if compiled with DBG_INV_ZRL create stream and include debug macros
#ifdef DBG_INV_ZRL
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif

class InvZrl: public smoc_actor {
public:
  smoc_port_in<JpegChannel_t>      in;
  smoc_port_out<JpegChannel_t>     out;
private:

#ifdef DBG_ENABLE
  CoSupport::DebugOstream dbgout;
#endif // DBG_ENABLE

  unsigned char pixel_id;
  unsigned char runlength;

  CategoryAmplitude_t amplitude;
  Category_t category;

  /* for debugging purposes */
  bool sob; //start of block
  void check_sob(){
    if (sob){
      assert(pixel_id == 0);
      sob = false;
    }
      
  }

  // unexpected CtrlCommand
  void unexpectedCtrl(){
    DBG_OUT("Unexpected control" << endl);
    DBG_OUT("pixel_id = " << pixel_id << endl);
  }

  // Must be called, if a coefficient is written
  void update_pixel_id(){
    pixel_id++;

    if (pixel_id >= JPEG_BLOCK_SIZE){
      DBG_OUT("Start new block!" << endl);
      pixel_id = 0;
      sob = true;  //For debugging purposes
    }
  }
  

  // forwards commands
  void forwardCtrl(){    
    DBG_OUT("Forward command" << endl);
    out[0] = in[0];

    //For debugging purposes
    check_sob();
  }

  // repeat coefficient of last runlength
  void write_rl_coeff(){
    DBG_OUT("Write runlength coefficient: ");
    DBG_OUT("Runlength position: " << runlength);
    DBG_OUT(", Pixel-ID: " << pixel_id << endl);

    out[0] = JS_DATA_QCOEFF_SET_CHWORD(0);

    runlength--;

    update_pixel_id();
  }

  // Start Zero Runlength
  void start_zero_rl(){
    DBG_OUT("Start zero runlength" << endl);

    runlength = 16;    

    write_rl_coeff();
  }

  // don't output zero run-length
  void skip_run(){
    DBG_OUT("Skip zero run" << endl);

    //store for later usage

    category = JS_TUP_GETCATEGORY(in[0]);
    amplitude = JS_TUP_GETIDCTAMPLCOEFF(in[0]);
  }

  // Start a run length which will be followed by a non-zero coefficient
  void start_non_zero_rl(){
    DBG_OUT("Start run with a non-zero coefficient" << endl);

    runlength = JS_TUP_GETRUNLENGTH(in[0]);

    DBG_OUT("Runlength = " << runlength << endl);

    write_rl_coeff();

    // Store for later usage
    category = JS_TUP_GETCATEGORY(in[0]);
    amplitude = JS_TUP_GETIDCTAMPLCOEFF(in[0]);
  }

  // Finish block
  void start_eob(){
    DBG_OUT("Finish block" << endl);
    sob = true;  //For debugging purposes

    assert(pixel_id < JPEG_BLOCK_SIZE);
    runlength = JPEG_BLOCK_SIZE - pixel_id;    

    DBG_OUT("runlength = " << runlength << endl);

    write_rl_coeff();
  }

  // Standard, page F.12
  QuantIDCTCoeff_t extend_coeff(CategoryAmplitude_t amplitude,
				Category_t category){

    QuantIDCTCoeff_t return_value = 1 << (category-1);

    if (category == 0){
      return_value = 0;
    }else if (amplitude < return_value){
      return_value = (-1) << category;
      return_value++;

      return_value += amplitude;
    }else{
      return_value = amplitude;
    }

    return return_value;
  }

  // Perform DC coefficient decoding
  void wr_dc_coeff(){
    DBG_OUT("Write DC coefficient: ");
    
    Category_t category = JS_TUP_GETCATEGORY(in[0]);
    CategoryAmplitude_t amplitude = JS_TUP_GETIDCTAMPLCOEFF(in[0]);

    DBG_OUT("cat = " << (int)category << ", ampl = " << (int)amplitude << ", ");
    
    // Error checking
    CategoryAmplitude_t max_amplitude = ~0;
    max_amplitude = max_amplitude << category;
    
    // Check, that only desired bits are set
    assert((max_amplitude & amplitude) == 0);

    QuantIDCTCoeff_t dc_coeff = extend_coeff(amplitude,category);
    
    out[0] = JS_DATA_QCOEFF_SET_CHWORD(dc_coeff);    
    

    DBG_OUT("DC = " << dc_coeff << endl);    
    update_pixel_id();
    
  }

  // Perform AC coefficient decoding
  void wr_ac_coeff(){
    DBG_OUT("Write AC coefficient: ");    

    // Error checking
    assert(category > 0);
    CategoryAmplitude_t max_amplitude = ~0;
    max_amplitude = max_amplitude << category;
    
    // Check, that only desired bits are set
    assert((max_amplitude & amplitude) == 0);

    QuantIDCTCoeff_t ac_coeff = extend_coeff(amplitude,category);
    
    out[0] = JS_DATA_QCOEFF_SET_CHWORD(ac_coeff);
    DBG_OUT(ac_coeff << endl);

    update_pixel_id();
    
  }

  smoc_firing_state process_dc_coeff;
  smoc_firing_state process_ac_coeff;
  smoc_firing_state write_ac_coeff;
  smoc_firing_state zero_run;
  smoc_firing_state non_zero_run;
  smoc_firing_state stuck;

  
public:
  InvZrl(sc_module_name name)
    : smoc_actor(name, process_dc_coeff),
#ifdef DBG_ENABLE
      dbgout(std::cerr),
#endif // DBG_ENABLE
      pixel_id(0),
      runlength(0)      
  {

#ifdef DBG_ENABLE
    //Set Debug ostream options
    CoSupport::Header my_header("InvZrl> ");
    dbgout << my_header;
#endif // DBG_ENABLE

    
    process_dc_coeff =
      /* ignore and forward control tokens */
      (in(1) && JS_ISCTRL(in.getValueAt(0))) >>
      (out(1))                                       >>
      CALL(InvZrl::forwardCtrl)                >> process_dc_coeff
      
      /* Process DC coeff */
      | (in(1) && (!JS_ISCTRL(in.getValueAt(0)))) >>
      (out(1))                                  >>
      CALL(InvZrl::wr_dc_coeff)                >> process_ac_coeff;
      


    process_ac_coeff
      /* ignore and forward control tokens */
      = (in(1) && JS_ISCTRL(in.getValueAt(0))) >>
      (out(1))                                       >>
      CALL(InvZrl::unexpectedCtrl)                >> stuck

      /* Process special zero run length */
      | (in(1) && (!JS_ISCTRL(in.getValueAt(0)))) >>
      ((JS_TUP_GETRUNLENGTH(in.getValueAt(0)) == (JpegChannel_t)0xF) &&
       (JS_TUP_GETCATEGORY(in.getValueAt(0)) == (JpegChannel_t)0)) >>
      (out(1))                                       >>
      CALL(InvZrl::start_zero_rl)                >> zero_run

      /* Process end of block */
      | (in(1) && (!JS_ISCTRL(in.getValueAt(0)))) >>
      ((JS_TUP_GETRUNLENGTH(in.getValueAt(0)) == (JpegChannel_t)0) &&
       (JS_TUP_GETCATEGORY(in.getValueAt(0)) == (JpegChannel_t)0) &&
       //More than one pixel is missing
       (VAR(pixel_id) < JPEG_BLOCK_SIZE-1)) >>
      (out(1))                                       >>
      CALL(InvZrl::start_eob)                >> zero_run

      | (in(1) && (!JS_ISCTRL(in.getValueAt(0)))) >>
      ((JS_TUP_GETRUNLENGTH(in.getValueAt(0)) == (JpegChannel_t)0) &&
       (JS_TUP_GETCATEGORY(in.getValueAt(0)) == (JpegChannel_t)0) &&
       //Only one pixel is missing
       (VAR(pixel_id) >= JPEG_BLOCK_SIZE-1)) >>
      (out(1))                                       >>
      CALL(InvZrl::start_eob)                >> process_dc_coeff

      /* Process AC coeff */
      | (in(1) && (!JS_ISCTRL(in.getValueAt(0)))) >>
      ((JS_TUP_GETRUNLENGTH(in.getValueAt(0)) == (JpegChannel_t)0) &&
       (JS_TUP_GETCATEGORY(in.getValueAt(0)) != (JpegChannel_t)0)) >>
      //only AC coeff
      CALL(InvZrl::skip_run)                >> write_ac_coeff

      | (in(1) && (!JS_ISCTRL(in.getValueAt(0)))) >>
      (JS_TUP_GETRUNLENGTH(in.getValueAt(0)) == (JpegChannel_t)1) >> 
      //only one zero
      (out(1)) >>
      CALL(InvZrl::start_non_zero_rl) >> write_ac_coeff
      
      | (in(1) && (!JS_ISCTRL(in.getValueAt(0)))) >>
      ((JS_TUP_GETRUNLENGTH(in.getValueAt(0)) != (JpegChannel_t)0) &&
       ((JS_TUP_GETRUNLENGTH(in.getValueAt(0)) != (JpegChannel_t)0xF) ||
	(JS_TUP_GETCATEGORY(in.getValueAt(0)) != (JpegChannel_t)0)) &&
       (JS_TUP_GETRUNLENGTH(in.getValueAt(0)) != (JpegChannel_t)1)
       ) >>
      (out(1))                                       >>
      CALL(InvZrl::start_non_zero_rl)                >> non_zero_run;

    /* Complete zero run */
    zero_run = 
      // continue zero-run
      (VAR(runlength) != 1) >>
      (out(1)) >>      
      CALL(InvZrl::write_rl_coeff) >> zero_run
      // more than one pixel is missing for block
      | ((VAR(runlength) == 1) && (VAR(pixel_id) < JPEG_BLOCK_SIZE-1)) >>
      (out(1)) >>
      CALL(InvZrl::write_rl_coeff) >> process_ac_coeff
      // only one pixel is missing for block
      | ((VAR(runlength) == 1) && (VAR(pixel_id) >= JPEG_BLOCK_SIZE-1)) >>
      (out(1)) >>
      CALL(InvZrl::write_rl_coeff) >> process_dc_coeff;

    /* Complete non-zero run */
    non_zero_run = 
      // continue non-zero-run
      (VAR(runlength) != 1) >>
      (out(1)) >>      
      CALL(InvZrl::write_rl_coeff) >> non_zero_run
      //write last zero for run
      | (VAR(runlength) == 1) >>
      (out(1)) >>      
      CALL(InvZrl::write_rl_coeff) >> write_ac_coeff;

    write_ac_coeff =
      // more than one pixel is missing for block
      (VAR(pixel_id) < JPEG_BLOCK_SIZE-1) >>
      (out(1)) >>
      CALL(InvZrl::wr_ac_coeff) >> process_ac_coeff
      // only one pixel is missing for block
      | (VAR(pixel_id) >= JPEG_BLOCK_SIZE-1) >>
      (out(1)) >>
      CALL(InvZrl::wr_ac_coeff) >> process_dc_coeff;
      
  }
};

#endif // _INCLUDED_INV_ZRL_HPP
