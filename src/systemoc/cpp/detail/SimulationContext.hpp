// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SMOC_DETAIL_SIMULATIONCONTEXT_HPP
#define _INCLUDED_SMOC_DETAIL_SIMULATIONCONTEXT_HPP

#include <ostream>
#include <vector>

#include <systemoc/smoc_config.h>

#include <smoc/SimulatorAPI/SimulatorInterface.hpp>
#include <smoc/detail/SimCTXBase.hpp>

#ifdef MAESTRO_ENABLE_POLYPHONIC
# include <boost/thread/mutex.hpp>
#endif //MAESTRO_ENABLE_POLYPHONIC

#include <boost/noncopyable.hpp>

#include "SMXDumper.hpp"
#include "SMXImporter.hpp"

#include "IdPool.hpp"

namespace smoc { namespace Detail {

class SimulationContext
  : private boost::noncopyable
#ifdef SYSTEMOC_ENABLE_SGX
  , public SimulationContextSMXDumping
  , public SimulationContextSMXImporting
#endif //SYSTEMOC_ENABLE_SGX
{
private:
  typedef SimulatorAPI::SimulatorInterface SI;

  std::vector<char *>  argv;
  SI                  *simulatorInterface;

#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
  std::ostream   *dumpTraceFile;
#endif // SYSTEMOC_ENABLE_TRANSITION_TRACE
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  smoc::Detail::TraceLogStream *dataflowTraceLog; 
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
#ifdef SYSTEMOC_NEED_IDS
  Detail::IdPool  idPool;
#endif // SYSTEMOC_NEED_IDS
public:
  SimulationContext(int _argc, char *_argv[]);


#ifdef MAESTRO_ENABLE_POLYPHONIC
  boost::mutex* event_mutex;
#endif

  int    getArgc();
  char **getArgv();

  SI    *getSimulatorInterface()
    { return simulatorInterface; }

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

  ~SimulationContext();
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_SIMULATIONCONTEXT_HPP */
