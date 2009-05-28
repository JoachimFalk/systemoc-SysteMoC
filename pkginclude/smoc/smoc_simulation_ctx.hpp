// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of
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

#ifndef _INCLUDED_SMOC_SIMULATIONCTX_HPP
#define _INCLUDED_SMOC_SIMULATIONCTX_HPP

#include <ostream>

#include <systemoc/smoc_config.h>

#include "detail/IdPool.hpp"

namespace SysteMoC {

class smoc_simulation_ctx;

namespace Detail {

#ifdef SYSTEMOC_ENABLE_SGX
  namespace SGX = SystemCoDesigner::SGX;
#endif // SYSTEMOC_ENABLE_SGX

  extern smoc_simulation_ctx *currentSimCTX;

  struct SimCTXBase {
    smoc_simulation_ctx *getSimCTX()
      { return currentSimCTX; }
  };

} // namespace Detail

class smoc_simulation_ctx {
protected:
  int    argc;
  char **argv;

  bool          dumpSMXWithSim;
  std::ostream *dumpFileSMX;
  std::ostream *dumpFileTrace;
  bool          dumpFSMs;
#ifdef SYSTEMOC_ENABLE_SGX
  Detail::SGX::NetworkGraphAccess ngx;
#endif // SYSTEMOC_ENABLE_SGX
#ifdef SYSTEMOC_NEED_IDS
  Detail::IdPool                  idPool;
#endif // SYSTEMOC_NEED_IDS
public:
  smoc_simulation_ctx(int _argc, char *_argv[]);

  int    getArgc();
  char **getArgv();

#ifdef SYSTEMOC_ENABLE_SGX
  Detail::SGX::NetworkGraphAccess &getExportNGX()
    { return ngx; }
  bool isSMXDumpingPreSimEnabled()
    { return dumpFileSMX && !dumpSMXWithSim; }
  std::ostream &getSMXPreSimFile()
    { return *dumpFileSMX; }
  bool isSMXDumpingPostSimEnabled()
    { return dumpFileSMX && dumpSMXWithSim; }
  std::ostream &getSMXPostSimFile()
    { return *dumpFileSMX; }
#endif // SYSTEMOC_ENABLE_SGX
#ifdef SYSTEMOC_NEED_IDS
  Detail::IdPool &getIdPool()
    { return idPool; }
#endif // SYSTEMOC_NEED_IDS
#ifdef SYSTEMOC_ENABLE_TRACE
  bool isTraceDumpingEnabled() const
    { return dumpFileTrace != NULL; }
  std::ostream &getTraceFile()
    { return *dumpFileTrace; }
#endif // SYSTEMOC_ENABLE_TRACE
  bool isFSMDumpingEnabled() const
    { return dumpFSMs; }

  void defCurrentCTX();
  void undefCurrentCTX();

  ~smoc_simulation_ctx();
};

} // namespace SysteMoC

#endif // _INCLUDED_SMOC_SIMULATIONCTX_HPP