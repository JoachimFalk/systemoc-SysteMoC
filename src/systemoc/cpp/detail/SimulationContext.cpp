// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2018 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#include <smoc/detail/DebugOStream.hpp>
#include <smoc/detail/TraceLog.hpp>
#include <smoc/SimulatorAPI/SimulatorInterface.hpp>

#include "SimulationContext.hpp"
#include "SMXImporter.hpp"

#include <systemoc/smoc_config.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>

#include <CoSupport/Streams/AlternateStream.hpp>

#include <cstring>
#include <iostream>
#include <sstream>
#include <stdlib.h>

namespace smoc { namespace Detail {

namespace po = boost::program_options;

using SimulatorAPI::SimulatorInterface;

// This global variable will also be used in <smoc/detail/SimCTXBase.hpp>
SimulationContext *currentSimCTX = nullptr;

SimulationContext::SimulationContext(int _argc, char *_argv[])
  : simulatorInterface(nullptr)
#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
  , dumpTraceFile(nullptr)
#endif // SYSTEMOC_ENABLE_TRANSITION_TRACE
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  , dataflowTraceLog(nullptr)
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
{
#ifdef MAESTRO_ENABLE_POLYPHONIC
  event_mutex = new boost::mutex();
  sc_core::sc_get_curr_simcontext();
  sc_core::sc_curr_simcontext->event_mutex = event_mutex;
#endif

  po::options_description systemocOptions("SysteMoC options");
  po::options_description backwardCompatibilityCruftOptions;
  
  systemocOptions.add_options()
    ("systemoc-help",
     "This help message")
    ;

#ifdef SYSTEMOC_ENABLE_DEBUG
  systemocOptions.add_options()
#else //!defined(SYSTEMOC_ENABLE_DEBUG)
  backwardCompatibilityCruftOptions.add_options()
#endif //!defined(SYSTEMOC_ENABLE_DEBUG)
    ("systemoc-debug", po::value<size_t>()->default_value(0), "turn on debug mode")
    ;

#ifdef SYSTEMOC_ENABLE_SGX
  systemocOptions.add_options()
#else // !SYSTEMOC_ENABLE_SGX
  backwardCompatibilityCruftOptions.add_options()
#endif // !SYSTEMOC_ENABLE_SGX
    ("systemoc-export-smx",
     po::value<std::string>(),
     "Dump smoc-XML after elaboration")
    ("systemoc-export-sim-smx",
     po::value<std::string>(),
     "Dump smoc-XML after simulation")
    ("systemoc-export-smx-keep-going",
     "Don't stop if dumping smoc-XML after elaboration")
    ("systemoc-export-smx-no-ast",
     "Disable smoc-XML transition AST dumping")
    ("systemoc-import-smx",
     po::value<std::string>(),
     "Synchronize with specified smoc-XML")
    ;
  
#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
  systemocOptions.add_options()
#else // !SYSTEMOC_ENABLE_TRANSITION_TRACE
  backwardCompatibilityCruftOptions.add_options()
#endif // !SYSTEMOC_ENABLE_TRANSITION_TRACE
    ("systemoc-export-trace",
     po::value<std::string>(),
     "Dump execution trace")
    ;

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  systemocOptions.add_options()
#else // !SYSTEMOC_ENABLE_DATAFLOW_TRACE
  backwardCompatibilityCruftOptions.add_options()
#endif // !SYSTEMOC_ENABLE_DATAFLOW_TRACE
    ("systemoc-export-dataflow-trace",
     po::value<std::string>(),
     "Dump dataflow trace")
    ;
  
  // Backward compatibility cruft
  backwardCompatibilityCruftOptions.add_options()
    ("export-smx",
     po::value<std::string>())
    ("export-sim-smx",
     po::value<std::string>())
    ("import-smx",
     po::value<std::string>())
    ;

  for (SimulatorInterface *simulator : SimulatorInterface::getRegisteredSimulators())
    simulator->populateOptionsDescription(_argc, _argv,
        systemocOptions, backwardCompatibilityCruftOptions);
  // All options
  po::options_description od;
  od.add(systemocOptions).add(backwardCompatibilityCruftOptions);
  po::parsed_options parsed =
    po::command_line_parser(_argc, _argv).options(od).allow_unregistered().run();
  po::variables_map vm;
  po::store(parsed, vm);
  po::notify(vm);

#ifdef SYSTEMOC_ENABLE_DEBUG
  outDbg.setLevel(Debug::None);
  outDbg << Debug::High;
#endif // !SYSTEMOC_ENABLE_DEBUG
  
  bool vpc = false;

  // Create new argv from not handled options
  {
    argv.push_back(strdup(_argc >= 1 ? _argv[0] : "???"));
    for (po::basic_option<char> const &option : parsed.options)
      if (option.unregistered || option.position_key != -1) {
        vpc |= option.string_key.find("systemoc-vpc") == 0;
        for(std::string const &arg : option.original_tokens)
          argv.push_back(strdup(arg.c_str()));
      }
    argv.push_back(nullptr);
  }

  // Select the simulator back end from the list of registered simulators.
  {
    typedef std::pair<
        SimulatorInterface::EnablementStatus,
        SimulatorInterface *>
      SimulatorEnablementStatus;

    std::vector<SimulatorEnablementStatus> simulatorStates;
    for (SimulatorInterface *simulator : SimulatorInterface::getRegisteredSimulators())
      simulatorStates.push_back(std::make_pair(simulator->evaluateOptionsMap(vm), simulator));
    for (SimulatorEnablementStatus simState : simulatorStates) {
      if (simState.first == SimulatorInterface::MUSTBE_ACTIVE) {
        if (!this->simulatorInterface)
          this->simulatorInterface = simState.second;
        else {
          std::ostringstream str;
          str << "Enabling of multiple simulator back ends leads to a clash. Please only select one of them!";
          throw std::runtime_error(str.str().c_str());
        }
      }
    }
    if (!this->simulatorInterface) {
      for (SimulatorEnablementStatus simState : simulatorStates) {
        if (simState.first == SimulatorInterface::MAYBE_ACTIVE) {
          this->simulatorInterface = simState.second;
          break;
        }
      }
    }
  }
  assert(this->simulatorInterface);
  
  for (std::vector<po::basic_option<char> >::const_iterator i = parsed.options.begin();
       i != parsed.options.end();
       ++i) {
    if (i->string_key == "systemoc-help") {
      std::cerr << systemocOptions << std::endl;
      exit(0);
    } else if (i->string_key == "systemoc-debug") {
      assert(!i->value.empty());
#ifdef SYSTEMOC_ENABLE_DEBUG
      int   debugLevel = Debug::None.level - atoi(i->value.front().c_str());
      outDbg.setLevel(debugLevel < 0 ? 0 : debugLevel);
      outDbg << Debug::High;
#else  // !SYSTEMOC_ENABLE_DEBUG
      std::ostringstream str;
      str << "SysteMoC configured without debug output support: --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_DEBUG
    } else if (i->string_key == "systemoc-export-smx" ||
               i->string_key == "export-smx") {
      assert(!i->value.empty());
#ifdef SYSTEMOC_ENABLE_SGX
      // delete null pointer is allowed...
      delete dumpPreSimSMXFile;
      
      dumpPreSimSMXFile =
        new CoSupport::Streams::AOStream(std::cout, i->value.front(), "-");
#else  // !SYSTEMOC_ENABLE_SGX
      std::ostringstream str;
      str << "SysteMoC configured without sgx support: --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_SGX
    } else if (i->string_key == "systemoc-export-sim-smx" ||
               i->string_key == "export-sim-smx") {
      assert(!i->value.empty());
#ifdef SYSTEMOC_ENABLE_SGX
      // delete null pointer is allowed...
      delete dumpPostSimSMXFile;
      
      dumpPreSimSMXKeepGoing = true;
      dumpPostSimSMXFile =
        new CoSupport::Streams::AOStream(std::cout, i->value.front(), "-");
#else  // !SYSTEMOC_ENABLE_SGX
      std::ostringstream str;
      str << "SysteMoC configured without sgx support: --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_SGX
    } else if (i->string_key == "systemoc-export-smx-keep-going") {
#ifdef SYSTEMOC_ENABLE_SGX
      dumpPreSimSMXKeepGoing = true;
#else  // !SYSTEMOC_ENABLE_SGX
      std::ostringstream str;
      str << "SysteMoC configured without sgx support: --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_SGX
    } else if (i->string_key == "systemoc-export-smx-no-ast") {
#ifdef SYSTEMOC_ENABLE_SGX
      dumpSMXAST = false;
#else  // !SYSTEMOC_ENABLE_SGX
      std::ostringstream str;
      str << "SysteMoC configured without sgx support: --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_SGX
    } else if (i->string_key == "systemoc-import-smx" ||
               i->string_key == "import-smx") {
      assert(!i->value.empty());
//    std::ostringstream str;
//    str << "SysteMoC option --" << i->string_key << " is not currently supported!";
//    throw std::runtime_error(str.str().c_str());
#ifdef SYSTEMOC_ENABLE_SGX
      importSMXFile =
        new CoSupport::Streams::AIStream(std::cin, i->value.front(), "-");
#else  // !SYSTEMOC_ENABLE_SGX
      std::ostringstream str;
      str << "SysteMoC configured without sgx support: --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_SGX
    } else if (i->string_key == "systemoc-export-trace") {
      assert(!i->value.empty());
#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
      // delete null pointer is allowed...
      delete dumpTraceFile;
      
# ifdef SYSTEMOC_ENABLE_SGX
      dumpPreSimSMXKeepGoing = true;
# endif // SYSTEMOC_ENABLE_SGX
      dumpTraceFile =
        new CoSupport::Streams::AOStream(std::cout, i->value.front(), "-");
#else  // !SYSTEMOC_ENABLE_TRANSITION_TRACE
      std::ostringstream str;
      str << "SysteMoC configured without trace support: --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_TRANSITION_TRACE
    } else if (i->string_key == "systemoc-export-dataflow-trace") {
      assert(!i->value.empty());
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
      // delete null pointer is allowed...
      delete dataflowTraceLog;
      
# ifdef SYSTEMOC_ENABLE_SGX
      dumpPreSimSMXKeepGoing = true;
# endif // SYSTEMOC_ENABLE_SGX
      dataflowTraceLog = new smoc::TraceLogStream(
        new CoSupport::Streams::AOStream(std::cout, i->value.front(), "-"));
      dataflowTraceLog->init();
#else  // !SYSTEMOC_ENABLE_DATAFLOW_TRACE
      std::ostringstream str;
      str << "SysteMoC configured without dataflow trace support: --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_DATAFLOW_TRACE
/*  } else if (i->unregistered || i->position_key != -1) {
      for(std::vector<std::string>::const_iterator j = i->original_tokens.begin();
          j != i->original_tokens.end();
          ++j)
        argv.push_back(strdup(j->c_str()));
    } else {
      assert(!"WTF?! UNHANDLED OPTION!"); */
    }
  }
  if (getenv("VPCCONFIGURATION") != nullptr) {
    std::ostringstream str;
    str << "SysteMoC configured without VPC support: Support for VPCCONFIGURATION environment variable is not provided!";
    throw std::runtime_error(str.str().c_str());
  }
  if (vpc) {
    std::ostringstream str;
    str << "SysteMoC configured without VPC support: Support for --systemoc-vpc-* options is not provided!";
    throw std::runtime_error(str.str().c_str());
  }
  
  if (currentSimCTX == nullptr)
    defCurrentCTX();
}

void SimulationContext::defCurrentCTX() {
  assert(currentSimCTX == nullptr);
  currentSimCTX = this;
}

void SimulationContext::undefCurrentCTX() {
  assert(currentSimCTX == this);
  currentSimCTX = nullptr;
}

SimulationContext::~SimulationContext() {
  for (std::vector<char *>::iterator iter = argv.begin();
       iter != argv.end();
       ++iter)
    // null pointer free might not be supported!
    if (*iter != nullptr)
      free(*iter);
  
  // delete null pointer is allowed...
#ifdef SYSTEMOC_ENABLE_SGX
  delete dumpPreSimSMXFile;
  delete dumpPostSimSMXFile;
#endif // SYSTEMOC_ENABLE_SGX
#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
  delete dumpTraceFile;
#endif // SYSTEMOC_ENABLE_TRANSITION_TRACE
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  delete dataflowTraceLog;
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
  
  if (currentSimCTX == this)
    undefCurrentCTX();
}

int SimulationContext::getArgc() {
  return argv.size() - 1;
}

char **SimulationContext::getArgv() {
  return &argv[0];
}

// end of simulation call back: clean SystemC related objects here
void SimulationContext::endOfSystemcSimulation(){
  static bool called = 0;
  if (!called) {
    called = true;
    if (getSimulatorInterface())
      getSimulatorInterface()->simulationEnded();
  }
}

/// This stuff is here to pull SysteMoCSimulator.cpp into the link.
class SysteMoCSimulator;
extern SysteMoCSimulator systeMoCSimulator;
static __attribute__ ((unused))
SysteMoCSimulator *pSimulator = &systeMoCSimulator;

} } // namespace smoc::Detail
