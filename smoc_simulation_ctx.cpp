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

#include <cstring>
#include <iostream>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>

#include <CoSupport/Streams/AlternateStream.hpp>

#include <systemoc/smoc_config.h>

#include <smoc/smoc_simulation_ctx.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

#include <smoc/smoc_simulation_ctx.hpp>

using namespace boost::program_options;

// Backward compatibility cruft
namespace smoc_modes {
  bool          dumpSMXWithSim = false;
  std::ostream *dumpFileSMX    = NULL;
  bool          dumpFSMs       = false;
} // namespace smoc_modes

namespace SysteMoC {

namespace Detail {

  smoc_simulation_ctx *currentSimCTX = NULL;

} // namespace Detail

smoc_simulation_ctx::smoc_simulation_ctx(int _argc, char *_argv[])
  : argc(1), argv(_argv),
    dumpSMXWithSim(false),
    dumpFileSMX(NULL),
    dumpFileTrace(NULL),
    dumpFSMs(false)
{
  options_description od;
  od.add_options()
    ("export-smx",
     value<std::string>(),
     "dump SysteMoC-XML after elaboration")
    ("export-sim-smx",
     value<std::string>(),
     "dump SysteMoC-XML after simulation")
    ("import-smx",
     value<std::string>(),
     "synchronize with specified SysteMoC-XML")
    ("export-trace",
     value<std::string>(),
     "dump execution trace")
    ("vpc-config",
     value<std::string>(),
     "use specified SystemC-VPC configuration file")
    ("dump-fsm",
     "dump flattened FSMs as DOT graph");
  
  parsed_options parsed =
    command_line_parser(_argc, _argv).options(od).allow_unregistered().run();
  
  for (std::vector< basic_option<char> >::const_iterator i = parsed.options.begin();
       i != parsed.options.end();
       ++i) {
    if (i->string_key == "dump-fsm") {
      dumpFSMs = true;
    } else if(i->string_key == "export-smx") {
      assert(!i->value.empty());
#ifdef SYSTEMOC_ENABLE_SGX
      // delete null pointer is allowed...
      delete dumpFileSMX;
      
      dumpFileSMX =
        new CoSupport::Streams::AOStream(std::cout, i->value.front(), "-");
#else  // !SYSTEMOC_ENABLE_SGX
      throw std::runtime_error("SysteMoC configured without sgx support --export-smx option not provided!");
#endif // !SYSTEMOC_ENABLE_SGX
    } else if(i->string_key == "export-sim-smx") {
      assert(!i->value.empty());
#ifdef SYSTEMOC_ENABLE_SGX
      // delete null pointer is allowed...
      delete dumpFileSMX;
      
      dumpSMXWithSim = true;
      dumpFileSMX =
        new CoSupport::Streams::AOStream(std::cout, i->value.front(), "-");
#else  // !SYSTEMOC_ENABLE_SGX
      throw std::runtime_error("SysteMoC configured without sgx support --export-sim-smx option not provided!");
#endif // !SYSTEMOC_ENABLE_SGX
    } else if(i->string_key == "import-smx") {
      assert(!i->value.empty());
//#ifdef SYSTEMOC_ENABLE_SGX
//    
//    CoSupport::Streams::AIStream in(std::cin, i->value.front(), "-");
//    SysteMoC::Detail::NGXConfig::getInstance().loadNGX(in);
//#else  // !SYSTEMOC_ENABLE_SGX
      throw std::runtime_error("SysteMoC configured without sgx support --import-smx option not provided!");
//#endif // !SYSTEMOC_ENABLE_SGX
    } else if(i->string_key == "export-trace") {
      assert(!i->value.empty());
#ifdef SYSTEMOC_ENABLE_TRACE
      // delete null pointer is allowed...
      delete dumpFileTrace;
      
      dumpFileTrace =
        new CoSupport::Streams::AOStream(std::cout, i->value.front(), "-");
#else  // !SYSTEMOC_ENABLE_TRACE
      throw std::runtime_error("SysteMoC configured without trace support --export-trace option not provided!");
#endif // !SYSTEMOC_ENABLE_TRACE
    } else if(i->string_key == "vpc-config") {
      assert(!i->value.empty());
#ifdef SYSTEMOC_ENABLE_VPC
      setenv("VPCCONFIGURATION", i->value.front().c_str(), 1);
#else  // !SYSTEMOC_ENABLE_VPC
      throw std::runtime_error("SysteMoC configured without vpc support --vpc-config option not provided!");
#endif // !SYSTEMOC_ENABLE_VPC
    } else if(i->unregistered || i->position_key != -1) {
      for(std::vector<std::string>::const_iterator j = i->original_tokens.begin();
          j != i->original_tokens.end();
          ++j)
        if ((argv[argc++] = strdup(j->c_str())) == NULL)
          throw std::bad_alloc();
    } else {
      assert(!"WTF?! UNHANDLED OPTION!");
    }
  }
  
  argv[argc] = NULL;
  assert(argc <= _argc);
  
#ifdef SYSTEMOC_ENABLE_VPC
  SystemC_VPC::Director::getInstance();
#endif
  
  if (Detail::currentSimCTX == NULL)
    defCurrentCTX();
}

void smoc_simulation_ctx::defCurrentCTX() {
  assert(Detail::currentSimCTX == NULL);
  Detail::currentSimCTX = this;
  // Backward compatibility cruft
  smoc_modes::dumpSMXWithSim = dumpSMXWithSim;
  smoc_modes::dumpFileSMX    = dumpFileSMX;
  smoc_modes::dumpFSMs       = dumpFSMs;
}

void smoc_simulation_ctx::undefCurrentCTX() {
  assert(Detail::currentSimCTX == this);
  Detail::currentSimCTX = NULL;
  // Backward compatibility cruft
  dumpSMXWithSim = smoc_modes::dumpSMXWithSim;
  dumpFileSMX    = smoc_modes::dumpFileSMX;
  dumpFSMs       = smoc_modes::dumpFSMs;
}

smoc_simulation_ctx::~smoc_simulation_ctx() {
  // Do not free argv[0] it was not strdupped
  for (--argc; argc >= 1; --argc)
    free(argv[argc]);
  
  if (Detail::currentSimCTX == this)
    undefCurrentCTX();
  
  // delete null pointer is allowed...
  delete dumpFileSMX;
  delete dumpFileTrace;
}

int smoc_simulation_ctx::getArgc() {
  return argc;
}

char **smoc_simulation_ctx::getArgv() {
  return argv;
}

} // namespace SysteMoC