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

#ifndef _INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP
#define _INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP


#ifdef KASCPAR_PARSING
typedef unsigned long size_t;
#else
using namespace std;
#endif

// Constants for IDCT2D_ARCH define
#define IDCT2D_FINEGRAINED    1
#define IDCT2D_COARSEGRAINED  2
#define IDCT2D_MONOLITHIC     3

// Constants for IDCT2D FIFO sizes
#define IDCT2D_FIFO_MIN_SIZE             0
#define IDCT2D_FIFO_DOUBLE_SIZE          1
#define IDCT2D_FIFO_CLUSTERING4PART_SIZE 2

#define IDCT2D_FIFO_SIZE  IDCT2D_FIFO_DOUBLE_SIZE

//#define REAL_BLOCK_DATA
#define IDCT_NULL_OUTPUT
#define DEFAULT_BLOCK_COUNT 100000

#ifdef REAL_BLOCK_DATA
// Only significant in case of REAL_BLOCK_DATA
# define SINK_BINARY_OUTPUT 
#else
// Only significant in case of !REAL_BLOCK_DATA
# define USE_COUNTER_INPUT
# ifdef EDK_XILINX_RUNTIME
    // No std::fstream in xilinx EDK !!!
#   define USE_COUNTER_INPUT
# endif
#endif

#ifndef USE_COUNTER_INPUT
# include <fstream>
# define INAMEblk "test_in.dat"
# define ONAMEblk "test_out.dat"
#endif

//#define VERBOSE_ACTOR
//#define VERBOSE_TRANSPOSE
//#define VERBOSE_IDCT_UPSAMPLE
//#define VERBOSE_MIN_DUPLEX
//#define VERBOSE_IDCT_SCALE
//#define VERBOSE_IDCT_FLY
//#define VERBOSE_IDCT_CLIP
//#define VERBOSE_IDCT_ADDSUB
//#define VERBOSE_IDCT_COL2BLOCK
//#define VERBOSE_IDCT_BLOCK2ROW

#include "callib.hpp"

#ifndef SMOC_REGISTER_CPARAM
# define SMOC_REGISTER_CPARAM(name) do {} while(0)
#endif

#endif // _INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP
