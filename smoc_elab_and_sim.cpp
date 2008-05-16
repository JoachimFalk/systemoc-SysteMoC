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

#include <systemoc/smoc_elab_and_sim.hpp>

#include "sysc/kernel/sc_cmnhdr.h"
#include "sysc/kernel/sc_externs.h"

#include <cstring>

#include <CoSupport/Streams/AlternateStream.hpp>

#include <systemoc/smoc_pggen.hpp>
#include <systemoc/smoc_ngx_sync.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

using namespace boost::program_options;

int smoc_elab_and_sim(int _argc, char* _argv[]) {
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
    ("vpc-config",
     value<std::string>(),
     "use specified SystemC-VPC configuration file");

  parsed_options parsed =
    command_line_parser(_argc, _argv).options(od).allow_unregistered().run();
  
  int argc = 1;
  char **argv = _argv;
  
  for(std::vector< basic_option<char> >::const_iterator i = parsed.options.begin();
      i != parsed.options.end();
      ++i)
  {
    if(i->string_key == "export-smx") {
      assert(smoc_modes::dumpFileSMX == NULL);      
      assert(!i->value.empty());
      
      smoc_modes::dumpFileSMX =
        new CoSupport::Streams::AOStream(std::cout, i->value.front(), "-");
    }
    else if(i->string_key == "export-sim-smx") {
      assert(smoc_modes::dumpFileSMX == NULL);
      assert(!i->value.empty());
      smoc_modes::dumpSMXWithSim = true;
      
      smoc_modes::dumpFileSMX =
        new CoSupport::Streams::AOStream(std::cout, i->value.front(), "-");
    }
    else if(i->string_key == "import-smx") {
      assert(!i->value.empty());
      
      CoSupport::Streams::AIStream in(std::cin, i->value.front(), "-");
      SysteMoC::NGXSync::NGXConfig::getInstance().loadNGX(in);
    }
    else if(i->string_key == "vpc-config") {
      assert(!i->value.empty());
      
      setenv("VPCCONFIGURATION", i->value.front().c_str(), 1);
    }
    else if(i->unregistered || i->position_key != -1) {
      for(std::vector<std::string>::const_iterator j = i->original_tokens.begin();
          j != i->original_tokens.end();
          ++j)
        if ((argv[argc++] = strdup(j->c_str())) == NULL)
          throw std::bad_alloc();
    }
  }
  
  argv[argc] = 0;
  assert(argc <= _argc);

#ifdef SYSTEMOC_ENABLE_VPC
  SystemC_VPC::Director::getInstance();
#endif

  int ret = sc_core::sc_elab_and_sim(argc, argv);  
  
  // Do not free argv[0] it was not strdupped
  for(--argc; argc >= 1; --argc)
    free(argv[argc]);

  // delete null pointer is allowed...
  delete smoc_modes::dumpFileSMX;

  return ret;
}
