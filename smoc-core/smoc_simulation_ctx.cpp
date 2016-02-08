// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#include <cstring>
#include <iostream>
#include <sstream>
#include <stdlib.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>

#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/Streams/AlternateStream.hpp>

#include <systemoc/smoc_config.h>

#include <smoc/smoc_simulation_ctx.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <vpc.hpp>
#endif //SYSTEMOC_ENABLE_VPC

#include <smoc/detail/TraceLog.hpp>


namespace po = boost::program_options;

namespace smoc {

namespace Detail {
  smoc_simulation_ctx *currentSimCTX = nullptr;
} // namespace Detail

smoc_simulation_ctx::smoc_simulation_ctx(int _argc, char *_argv[])
  :
#ifdef SYSTEMOC_ENABLE_SGX
    dumpPreSimSMXKeepGoing(false),
    dumpSMXAST(true),
    dumpPreSimSMXFile(nullptr),
    dumpPostSimSMXFile(nullptr),
#endif // SYSTEMOC_ENABLE_SGX
#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
    dumpTraceFile(nullptr),
#endif // SYSTEMOC_ENABLE_TRANSITION_TRACE
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    dataflowTraceLog(nullptr),
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
    dummy(false),
    vpcScheduling(false)
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
     "This help message");

  systemocOptions.add_options()( "systemoc-vpc-scheduling" , po::value( &vpcScheduling )->zero_tokens() );

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
     "Synchronize with specified smoc-XML");
  
#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
  systemocOptions.add_options()
#else // !SYSTEMOC_ENABLE_TRANSITION_TRACE
  backwardCompatibilityCruftOptions.add_options()
#endif // !SYSTEMOC_ENABLE_TRANSITION_TRACE
    ("systemoc-export-trace",
     po::value<std::string>(),
     "Dump execution trace");

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  systemocOptions.add_options()
#else // !SYSTEMOC_ENABLE_DATAFLOW_TRACE
  backwardCompatibilityCruftOptions.add_options()
#endif // !SYSTEMOC_ENABLE_DATAFLOW_TRACE
    ("systemoc-export-dataflow-trace",
     po::value<std::string>(),
     "Dump dataflow trace");
  
#ifdef SYSTEMOC_ENABLE_VPC
  systemocOptions.add_options()
#else // !SYSTEMOC_ENABLE_VPC
  backwardCompatibilityCruftOptions.add_options()
#endif // !SYSTEMOC_ENABLE_VPC
    ("systemoc-vpc-config",
     po::value<std::string>(),
     "use specified SystemC-VPC configuration file");
  
  // Backward compatibility cruft
  backwardCompatibilityCruftOptions.add_options()
    ("export-smx",
     po::value<std::string>())
    ("export-sim-smx",
     po::value<std::string>())
    ("import-smx",
     po::value<std::string>())
    ("vpc-config",
     po::value<std::string>());
  // All options
  po::options_description od;
  od.add(systemocOptions).add(backwardCompatibilityCruftOptions);
  po::parsed_options parsed =
    po::command_line_parser(_argc, _argv).options(od).allow_unregistered().run();
  
  argv.push_back(strdup(_argc >= 1 ? _argv[0] : "???"));
  
  for (std::vector<po::basic_option<char> >::const_iterator i = parsed.options.begin();
       i != parsed.options.end();
       ++i) {
    if (i->string_key == "systemoc-help") {
      std::cerr << systemocOptions << std::endl;
      exit(0);
    } else if (i->string_key == "systemoc-vpc-scheduling" ) {
        vpcScheduling = true;
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
//#ifdef SYSTEMOC_ENABLE_SGX
//    
//    CoSupport::Streams::AIStream in(std::cin, i->value.front(), "-");
//    smoc::Detail::NGXConfig::getInstance().loadNGX(in);
//#else  // !SYSTEMOC_ENABLE_SGX
      std::ostringstream str;
      str << "SysteMoC configured without sgx support: --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
//#endif // !SYSTEMOC_ENABLE_SGX
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
      dataflowTraceLog = new smoc::Detail::TraceLogStream(
        new CoSupport::Streams::AOStream(std::cout, i->value.front(), "-"));
      dataflowTraceLog->init();
#else  // !SYSTEMOC_ENABLE_DATAFLOW_TRACE
      std::ostringstream str;
      str << "SysteMoC configured without dataflow trace support: --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_DATAFLOW_TRACE
    } else if (i->string_key == "systemoc-vpc-config" ||
               i->string_key == "vpc-config") {
      assert(!i->value.empty());
#ifdef SYSTEMOC_ENABLE_VPC
# ifdef _MSC_VER
      std::string env="VPCCONFIGURATION";
      env += i->value.front().c_str();
      putenv(env.c_str());
# else
      setenv("VPCCONFIGURATION", i->value.front().c_str(), 1);
# endif // _MSC_VER
#else  // !SYSTEMOC_ENABLE_VPC
      std::ostringstream str;
      str << "SysteMoC configured without vpc support: --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_VPC
    } else if (i->unregistered || i->position_key != -1) {
      for(std::vector<std::string>::const_iterator j = i->original_tokens.begin();
          j != i->original_tokens.end();
          ++j)
        argv.push_back(strdup(j->c_str()));
    } else {
      assert(!"WTF?! UNHANDLED OPTION!");
    }
  }
  if (getenv("VPCCONFIGURATION") != nullptr) {
#ifndef SYSTEMOC_ENABLE_VPC
    std::ostringstream str;
    str << "SysteMoC configured without vpc support: Support for VPCCONFIGURATION env variable not provided!";
    throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_VPC
  }
  
  argv.push_back(nullptr);
  
#ifdef SYSTEMOC_ENABLE_VPC
  SystemC_VPC::Director::getInstance();
#endif
  
  if (Detail::currentSimCTX == nullptr)
    defCurrentCTX();
}

void smoc_simulation_ctx::defCurrentCTX() {
  assert(Detail::currentSimCTX == nullptr);
  Detail::currentSimCTX = this;
}

void smoc_simulation_ctx::undefCurrentCTX() {
  assert(Detail::currentSimCTX == this);
  Detail::currentSimCTX = nullptr;
}

smoc_simulation_ctx::~smoc_simulation_ctx() {
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
  
  if (Detail::currentSimCTX == this)
    undefCurrentCTX();
}

int smoc_simulation_ctx::getArgc() {
  return argv.size() - 1;
}

char **smoc_simulation_ctx::getArgv() {
  return &argv[0];
}

// end of simulation call back: clean SystemC related objects here
void smoc_simulation_ctx::endOfSystemcSimulation(){
  static bool called = 0;
  if (!called) {
    called = true;
#ifdef SYSTEMOC_ENABLE_STATE_TRACE
    // stateTrace contains sc_signals
    // delete it right before loosing the SystemC simulation context
    delete stateTracer;
#endif // SYSTEMOC_ENABLE_STATE_TRACE
#ifdef SYSTEMOC_ENABLE_VPC
  SystemC_VPC::Director::endOfSystemcSimulation();
#endif
  }
}

} // namespace smoc
