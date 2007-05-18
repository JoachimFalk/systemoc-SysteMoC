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

#include <cstdlib>
#include <iostream>
#include <fstream>

#include "debug_off.h"

// Constants for IDCT2D_ARCH define
#define IDCT2D_FINEGRAINED    1
#define IDCT2D_COARSEGRAINED  2
#define IDCT2D_MONOLITHIC     3

#define IDCT2D_ARCH IDCT2D_COARSEGRAINED

#define STATIC_IMAGE_SIZE
//#define DUMP_INTERMEDIATE

// FIXME: will render parameter handling useless!
//#define JPEG_SRC

#include "callib.hpp"
#include "channels.hpp"
#include "BitSplitter.hpp"

#ifndef SMOC_REGISTER_CPARAM
# define SMOC_REGISTER_CPARAM(name) do {} while(0)
#endif

#define IS_TABLE_CLASS_DC(v) (((v) & 0xF0) == 0x00)
#define IS_TABLE_CLASS_AC(v) (((v) & 0xF0) == 0x10)

#define IS_TABLE_DEST_ZERO(v) (((v) & 0x0F) == 0x00)
#define IS_TABLE_DEST_ONE(v) (((v) & 0x0F) == 0x01)

#endif // _INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP
