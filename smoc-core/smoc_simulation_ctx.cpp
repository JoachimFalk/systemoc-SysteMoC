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

namespace po = boost::program_options;

// Backward compatibility cruft
namespace smoc_modes {
  std::ostream *dumpFileSMX    = NULL;
} // namespace smoc_modes

namespace SysteMoC {

namespace Detail {

  smoc_simulation_ctx *currentSimCTX = NULL;

} // namespace Detail

smoc_simulation_ctx::smoc_simulation_ctx(int _argc, char *_argv[])
  :
#ifdef SYSTEMOC_ENABLE_SGX
    dumpPreSimSMXKeepGoing(false),
    dumpSMXAST(true),
    dumpPreSimSMXFile(NULL),
    dumpPostSimSMXFile(NULL),
#endif // SYSTEMOC_ENABLE_SGX
#ifdef SYSTEMOC_ENABLE_TRACE
    dumpTraceFile(NULL),
#endif // SYSTEMOC_ENABLE_TRACE
    dummy(false)
{
  po::options_description systemocOptions("SysteMoC options");
  po::options_description backwardCompatibilityCruftOptions;
  
  systemocOptions.add_options()
    ("systemoc-help",
     "This help message");
  
#ifdef SYSTEMOC_ENABLE_SGX
  systemocOptions.add_options()
#else // !SYSTEMOC_ENABLE_SGX
  backwardCompatibilityCruftOptions.add_options()
#endif // !SYSTEMOC_ENABLE_SGX
    ("systemoc-export-smx",
     po::value<std::string>(),
     "Dump SysteMoC-XML after elaboration")
    ("systemoc-export-sim-smx",
     po::value<std::string>(),
     "Dump SysteMoC-XML after simulation")
    ("systemoc-export-smx-keep-going",
     "Don't stop if dumping SysteMoC-XML after elaboration")
    ("systemoc-export-smx-no-ast",
     "Disable SysteMoC-XML transition AST dumping")
    ("systemoc-import-smx",
     po::value<std::string>(),
     "Synchronize with specified SysteMoC-XML");
  
#ifdef SYSTEMOC_ENABLE_TRACE
  systemocOptions.add_options()
#else // !SYSTEMOC_ENABLE_TRACE
  backwardCompatibilityCruftOptions.add_options()
#endif // !SYSTEMOC_ENABLE_TRACE
    ("systemoc-export-trace",
     po::value<std::string>(),
     "Dump execution trace");
  
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
    ("export-trace",
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
      str << "SysteMoC configured without sgx support --" << i->string_key << " option not provided!";
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
      str << "SysteMoC configured without sgx support --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_SGX
    } else if (i->string_key == "systemoc-export-smx-keep-going") {
#ifdef SYSTEMOC_ENABLE_SGX
      dumpPreSimSMXKeepGoing = true;
#else  // !SYSTEMOC_ENABLE_SGX
      std::ostringstream str;
      str << "SysteMoC configured without sgx support --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_SGX
    } else if (i->string_key == "systemoc-export-smx-no-ast") {
#ifdef SYSTEMOC_ENABLE_SGX
      dumpSMXAST = false;
#else  // !SYSTEMOC_ENABLE_SGX
      std::ostringstream str;
      str << "SysteMoC configured without sgx support --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_SGX
    } else if (i->string_key == "systemoc-import-smx" ||
               i->string_key == "import-smx") {
      assert(!i->value.empty());
//#ifdef SYSTEMOC_ENABLE_SGX
//    
//    CoSupport::Streams::AIStream in(std::cin, i->value.front(), "-");
//    SysteMoC::Detail::NGXConfig::getInstance().loadNGX(in);
//#else  // !SYSTEMOC_ENABLE_SGX
      std::ostringstream str;
      str << "SysteMoC configured without sgx support --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
//#endif // !SYSTEMOC_ENABLE_SGX
    } else if (i->string_key == "systemoc-export-trace" ||
               i->string_key == "export-trace") {
      assert(!i->value.empty());
#ifdef SYSTEMOC_ENABLE_TRACE
      // delete null pointer is allowed...
      delete dumpTraceFile;
      
# ifdef SYSTEMOC_ENABLE_SGX
      dumpPreSimSMXKeepGoing = true;
# endif // SYSTEMOC_ENABLE_SGX
      dumpTraceFile =
        new CoSupport::Streams::AOStream(std::cout, i->value.front(), "-");
#else  // !SYSTEMOC_ENABLE_TRACE
      std::ostringstream str;
      str << "SysteMoC configured without trace support --" << i->string_key << " option not provided!";
      throw std::runtime_error(str.str().c_str());
#endif // !SYSTEMOC_ENABLE_TRACE
    } else if (i->string_key == "systemoc-vpc-config" ||
               i->string_key == "vpc-config") {
      assert(!i->value.empty());
#ifdef SYSTEMOC_ENABLE_VPC
      setenv("VPCCONFIGURATION", i->value.front().c_str(), 1);
#else  // !SYSTEMOC_ENABLE_VPC
      std::ostringstream str;
      str << "SysteMoC configured without vpc support --" << i->string_key << " option not provided!";
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
  
  argv.push_back(NULL);
  
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
#ifdef SYSTEMOC_ENABLE_SGX
  smoc_modes::dumpFileSMX    = dumpPreSimSMXFile;
#endif // SYSTEMOC_ENABLE_SGX
}

void smoc_simulation_ctx::undefCurrentCTX() {
  assert(Detail::currentSimCTX == this);
  Detail::currentSimCTX = NULL;
  // Backward compatibility cruft
#ifdef SYSTEMOC_ENABLE_SGX
  dumpPreSimSMXFile = smoc_modes::dumpFileSMX;
#endif // SYSTEMOC_ENABLE_SGX
}

smoc_simulation_ctx::~smoc_simulation_ctx() {
  for (std::vector<char *>::iterator iter = argv.begin();
       iter != argv.end();
       ++iter)
    // null pointer free might not be supported!
    if (*iter != NULL)
      free(*iter);
  
  if (Detail::currentSimCTX == this)
    undefCurrentCTX();
  
  // delete null pointer is allowed...
#ifdef SYSTEMOC_ENABLE_SGX
  delete dumpPreSimSMXFile;
  delete dumpPostSimSMXFile;
#endif // SYSTEMOC_ENABLE_SGX
#ifdef SYSTEMOC_ENABLE_TRACE
  delete dumpTraceFile;
#endif // SYSTEMOC_ENABLE_TRACE
}

int smoc_simulation_ctx::getArgc() {
  return argv.size() - 1;
}

char **smoc_simulation_ctx::getArgv() {
  return &argv[0];
}

} // namespace SysteMoC
