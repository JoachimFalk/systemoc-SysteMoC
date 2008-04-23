// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
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

#include <systemoc/smoc_pggen.hpp>
#include <systemoc/smoc_root_node.hpp>
#include <systemoc/smoc_ngx_sync.hpp>

#include <acpgacc/smoc_synth_netgraph_access.hpp>

using namespace SysteMoC::NGXSync;

namespace smoc_modes {

  bool dumpSMXWithSim = false;
  std::ostream* dumpFileSMX = NULL;

  void dump( smoc_root_node &top ) {
    assert(dumpFileSMX != NULL);
    PGWriter pgw( *dumpFileSMX );
    pgw << "<?xml version=\"1.0\"?>" << std::endl;
    pgw << "<!DOCTYPE networkgraph SYSTEM \"networkgraph.dtd\">" << std::endl;
    pgw << "<networkgraph name=\"smoc_modes::dump\">" << std::endl;
    pgw.indentUp();
    top.assemble( pgw );
    pgw << "<architecturegraph name=\"architecture graph\" id=\""<< idPool.printId() << "\">" << std::endl;
    pgw << "</architecturegraph>" << std::endl;
    pgw <<  "<mappings>" << std::endl;
    pgw <<  "</mappings>" << std::endl;
    pgw.indentDown();
    pgw << "</networkgraph>" << std::endl;
  }
};
