//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif

#include "smoc_synth_std_includes.hpp"

#include "idct2d.hpp"

#ifndef REAL_BLOCK_DATA
# include "IDCTsource.hpp"
# include "IDCTsink.hpp"
#else
# include "IDCT_block_source.hpp"
# include "IDCT_block_sink.hpp"
#endif

#ifndef DEFAULT_BLOCK_COUNT
# ifdef REAL_BLOCK_DATA
#  define DEFAULT_BLOCK_COUNT ((IMAGE_WIDTH)/8*(IMAGE_HEIGHT)/8)
# else
#  define DEFAULT_BLOCK_COUNT 25
# endif
#endif

class mTopIdct2D
: public smoc_graph {
private:
#ifndef REAL_BLOCK_DATA
  m_source_idct       src;
#else
  m_block_source_idct src;
#endif
  MIdct2D             mIdct2D;
#ifndef REAL_BLOCK_DATA
  m_sink              snk;
#else
  m_block_sink        snk;
#endif

public:
  mTopIdct2D(sc_module_name name, size_t periods)
    : smoc_graph(name),
      src("src", periods),
      mIdct2D("mIdct2D"),
#ifdef REAL_BLOCK_DATA
      snk("snk",IMAGE_WIDTH, IMAGE_HEIGHT)
#else
      snk("snk")
#endif
  {
#ifndef KASCPAR_PARSING
    connectNodePorts(src.out,      mIdct2D.in,  smoc_fifo<int>(128));
    connectNodePorts(src.min,      mIdct2D.min, smoc_fifo<int>(4));
# ifndef REAL_BLOCK_DATA
    connectNodePorts(mIdct2D.out,  snk.in,     smoc_fifo<int>(128));
# else
    connectNodePorts(mIdct2D.out,  snk.in,     smoc_fifo<int>(IMAGE_WIDTH/8*64));
# endif
#endif
  }
};

#ifndef KASCPAR_PARSING
int sc_main (int argc, char **argv) {
  size_t periods            =
    (argc > 1)
    ? atoi(argv[1])
    : DEFAULT_BLOCK_COUNT;
  
  mTopIdct2D topIdct2D("topIdct2D", periods);
  
  smoc_top top(&topIdct2D);
  
  sc_start(-1);
  
  return 0;
}
#endif
