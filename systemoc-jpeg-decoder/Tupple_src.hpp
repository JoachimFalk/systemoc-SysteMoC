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

#ifndef _INCLUDED_TUPPLE_SRC_HPP
#define _INCLUDED_TUPPLE_SRC_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "channels.hpp"

// if compiled with DBG_INV_ZRL create stream and include debug macros
//#define DBG_TUPPLE_SRC
#ifdef DBG_TUPPLE_SRC
  #include <cosupport/smoc_debug_out.hpp>
  // debug macros presume some stream behind DBGOUT_STREAM. so make sure stream
  //  with this name exists when DBG.. is used. here every actor creates its
  //  own stream.
  #define DBGOUT_STREAM dbgout
  #include "debug_on.h"
#else
  #include "debug_off.h"
#endif

class TuppleSrc: public smoc_actor {
public:
  smoc_port_out<JpegChannel_t>     out;
private:

  ifstream infile;
  bool eof;

  CategoryAmplitude_t amplitude;
  Category_t category;
  RunLength_t rle;
	

  smoc_firing_state read_file;
  smoc_firing_state write_tupple;

  // writes it the the port
  void WriteTupple(){
    out[0] = JS_DATA_TUPPLED_SET_CHWORD(amplitude,rle,category);
  }	

  void ReadTupple(){
    std::string rlz;
		
    // Workaround, because >> is too silly.
    unsigned int temp;
    infile >> rlz >> temp >> amplitude;
    category = temp;
 		
    if (rlz == "DC"){
      rle = 0;
    }else{
      rle = atol(rlz.c_str());
    }

    if (!infile.good()){
      eof = true;
    }else{
      if (rlz == "DC"){
	DBG_OUT("DC ");
      }
      DBG_OUT("rle = " << rle 
	     << ", cat = " << (unsigned int)category 
	     << ", amplitude = " << amplitude 
	     << endl);
    }

  }

  CoSupport::DebugOstream dbgout;
public:
  TuppleSrc(sc_module_name name, const std::string& filename )
    : smoc_actor(name, read_file),
      infile(filename.c_str()),
      eof(false),
      dbgout(std::cout)
  {

    //Set Debug ostream options
    CoSupport::Header my_header("TuppleSrc> ");
    dbgout << my_header;

    
    read_file =
      /* ignore and forward control tokens */
      CALL(TuppleSrc::ReadTupple)                >> write_tupple;

    write_tupple =
      (!VAR(eof)) >>
      (out(1)) >>
      CALL(TuppleSrc::WriteTupple) >> read_file;
  }
};

#endif // _INCLUDED_TUPPLE_SRC_HPP
