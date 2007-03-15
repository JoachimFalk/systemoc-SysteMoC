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

#include <list>
#include <set>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_moc.hpp>

#include "channels.hpp"

class Testbench: public smoc_graph {
private:
public:
  Testbench(sc_module_name name)
    : smoc_graph(name)
  {
  }
};

#ifndef KASCPAR_PARSING
int sc_main (int argc, char **argv) {
#if 0
  if (argc <= 3) {
    std::cerr
      << (argv[0] != NULL ? argv[0] : "???")
      << " <width> <height> <scanpattern:idctcoeff filename>+" << std::endl;
    exit(-1);
  }

  
  size_t      width, height;
  ScanVector  scanVector;
  
  width  = atoi(argv[1]);
  height = atoi(argv[2]);
  
  for (const char *const *argIter = &argv[3]; *argIter != NULL; ++argIter) {
    size_t      pos = 0;
    const char *arg = *argIter;
    
    Scan scan;

    while (pos < SCANPATTERN_LENGTH) {
      if (arg[pos] < '0' || arg[pos] > '2') {
        std::cerr << argv[0] << ": scanpattern format error, scanpattern: [0-2]{6} !" << std::endl;
        exit (-1);
      }
      scan.scanPattern[pos] = arg[pos++] - '0';
    }
    if (arg[pos++] != ':') {
      std::cerr << argv[0] << ": missing colon after scanpattern !" << std::endl;
    }
    scan.idctCoeffFileName = &arg[pos];
    scanVector.push_back(scan);
  }

#endif
  
  smoc_top_moc<Testbench> testbench("testbench");
  
  sc_start(-1);
  
  return 0;
}
#endif
