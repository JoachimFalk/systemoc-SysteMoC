// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
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

#ifndef _INCLUDED_JPEGSOURCE_HPP
#define _INCLUDED_JPEGSOURCE_HPP

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_port.hpp>

#include "smoc_synth_std_includes.hpp"

#include "channels.hpp"

#ifdef PERFORMANCE_EVALUATION
# ifdef XILINX_EDK_RUNTIME
#  include "x_perf_eval.h"
# else
#  include <cosupport/PerformanceEvaluation.hpp>
# endif
#endif // PERFORMANCE_EVALUATION

class JpegSrc: public smoc_actor {
public:
  smoc_port_out<codeword_t> out;
private:
  size_t coeffs;
  size_t count;

  /* 
     Due to silly KASCPar limitations, we cannot declare block_data 
     as a member variable. Hence we have to do an hack for block_data_size.
   */
  size_t block_data_size;

  void process() {
#ifndef KASCPAR_PARSING    
    // ZRL coded IDCT coeffs
    const static short block_data[] = {
      //# include "array_jpeg_4motion.txt"
# include "array_institut_qcif.txt"
    };
    block_data_size =   
      sizeof(block_data)/sizeof(block_data[0]);
#endif

    short val = block_data[coeffs];
    
    while(val < 0){
      //start of image
#ifdef PERFORMANCE_EVALUATION
# ifdef XILINX_EDK_RUNTIME
      x_perf_eval_toggle_start();
# else
      PerformanceEvaluation::getInstance().startUnit();
# endif
#endif // PERFORMANCE_EVALUATION

      coeffs++;
      //Start of image must be followed by some data
      assert(coeffs < block_data_size);

      val = block_data[coeffs];
    }

    out[0] = val;
    ++coeffs;

    if( coeffs == block_data_size ) {
      --count;
      coeffs = 0;
    }
  }
 
  smoc_firing_state start;
public:
  JpegSrc(sc_module_name name)
    : smoc_actor(name, start),  coeffs(0), count(4)
  {
    block_data_size = ~size_t(0);
    start
      = (out(1) && VAR(count) > 0)  >>
        CALL(JpegSrc::process)         >> start
      ;
  }
};

#endif // _INCLUDED_JPEGSOURCE_HPP
