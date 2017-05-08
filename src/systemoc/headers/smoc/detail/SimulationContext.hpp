// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_DETAIL_SIMULATIONCONTEXT_HPP
#define _INCLUDED_SMOC_DETAIL_SIMULATIONCONTEXT_HPP

#include <CoSupport/compatibility-glue/nullptr.h>

#include <ostream>
#include <vector>

#include <systemoc/smoc_config.h>

#include "IdPool.hpp"
#include "SimCTXBase.hpp"

#ifdef MAESTRO_ENABLE_POLYPHONIC
#include <boost/thread/mutex.hpp>
#endif

namespace smoc { namespace Detail {

class SimulationContext {
private: //protected:
  std::vector<char *> argv;

#ifdef SYSTEMOC_ENABLE_SGX
  bool            dumpPreSimSMXKeepGoing;
  bool            dumpSMXAST;
  std::ostream   *dumpPreSimSMXFile;
  std::ostream   *dumpPostSimSMXFile;
#endif // SYSTEMOC_ENABLE_SGX
#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
  std::ostream   *dumpTraceFile;
#endif // SYSTEMOC_ENABLE_TRANSITION_TRACE
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  smoc::Detail::TraceLogStream *dataflowTraceLog; 
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
#ifdef SYSTEMOC_NEED_IDS
  Detail::IdPool  idPool;
#endif // SYSTEMOC_NEED_IDS
  bool dummy;
  bool vpcScheduling;
public:
  SimulationContext(int _argc, char *_argv[]);

#ifdef MAESTRO_ENABLE_POLYPHONIC
  boost::mutex* event_mutex;
#endif

  int    getArgc();
  char **getArgv();

  bool isVpcSchedulingEnabled() const {
    return vpcScheduling;
  }
#ifdef SYSTEMOC_ENABLE_SGX
  bool isSMXDumpingASTEnabled() const
    { return dumpSMXAST; }
  bool isSMXDumpingPreSimEnabled() const
    { return dumpPreSimSMXFile; }
  std::ostream &getSMXPreSimFile() const
    { return *dumpPreSimSMXFile; }
  bool isSMXDumpingPreSimKeepGoing() const
    { return dumpPreSimSMXKeepGoing; }
  bool isSMXDumpingPostSimEnabled() const
    { return dumpPostSimSMXFile; }
  std::ostream &getSMXPostSimFile() const
    { return *dumpPostSimSMXFile; }
#endif // SYSTEMOC_ENABLE_SGX
#ifdef SYSTEMOC_NEED_IDS
  Detail::IdPool &getIdPool()
    { return idPool; }
#endif // SYSTEMOC_NEED_IDS
#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
  bool isTraceDumpingEnabled() const
    { return dumpTraceFile != nullptr; }
  std::ostream &getTraceFile() const
    { return *dumpTraceFile; }
#endif // SYSTEMOC_ENABLE_TRANSITION_TRACE
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  bool isDataflowTracingEnabled() const
    { return dataflowTraceLog != nullptr; }
  smoc::Detail::TraceLogStream *getDataflowTraceLog() const
    { return dataflowTraceLog; }
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE

  void defCurrentCTX();
  void undefCurrentCTX();

  // end of simulation call back: clean SystemC related objects here
  void endOfSystemcSimulation();

  ~SimulationContext();

private:
  SimulationContext( const SimulationContext & toCopy ) {}
};

} } // namespace smoc::Detail

#endif // _INCLUDED_SMOC_DETAIL_SIMULATIONCONTEXT_HPP
