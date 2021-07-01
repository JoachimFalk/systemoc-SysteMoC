// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *  2021 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <smoc/sng.hpp>

#include <iostream>
#include <sstream>
#include <string>

#include <cstdio>

#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/AlternateStream.hpp>
#include <CoSupport/Streams/NullStreambuf.hpp>

#include <CoSupport/sassert.h>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>

#include <boost/graph/graphviz.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

std::string prgname = "???";

namespace po = ::boost::program_options;
namespace fs = ::boost::filesystem;

using ::CoSupport::Streams::Debug;
//using ::CoSupport::Streams::ScopedIndent;

typedef
#ifdef NDEBUG
  CoSupport::Streams::NullStreambuf::Stream<
#endif //NDEBUG
    CoSupport::Streams::DebugStreambuf::Stream<
      CoSupport::Streams::IndentStreambuf::Stream<
        CoSupport::Streams::HeaderFooterStreambuf::Stream<
    > > >
#ifdef NDEBUG
  >
#endif //NDEBUG
  DebugOStream;

/// Debug output stream
DebugOStream outDbg(std::cerr);

std::pair<std::string, std::string> poNoPrefixHandler(const std::string &s) {
  if (s.find("--no-") == 0) {
    return std::make_pair(s.substr(5), std::string("false"));
  } else {
    return std::make_pair(std::string(), std::string());
  }
}

/*
 * program [options] <network_graph>
 */
int main(int argc, char** argv)
{
  if (argc >= 1 && argv[0] != nullptr)
    prgname = basename(fs::path(argv[0]));
  
  std::stringstream sstr;
  
  sstr << "Usage: " << prgname << " [options] <input> <output>" << std::endl;
  sstr << "Available options";
  
  po::options_description publicOptions(sstr.str());
  
#ifndef NDEBUG
  sstr.str(""); // reset string stream
  sstr << "set debug level; level 0 is off; level " << Debug::None.level << " is most verbose";
#endif //SGXUTILS_DEBUG_OUTPUT
  
  publicOptions.add_options()
    ("help", "produce help message")
#ifndef NDEBUG
    ("debug", po::value<size_t>()->default_value(0), sstr.str().c_str())
#endif //SGXUTILS_DEBUG_OUTPUT
    ("transform", po::value<smoc::SNG::Transform>()->default_value(smoc::SNG::Transform::FIFOS_NO_MERGING), "one of "
        "FIFOS_NO_MERGING (default), "
        "FIFOS_SAME_CONTENT_MERGING, "
        "FIFOS_SAME_PRODUCER_MERGING, "
        "CHANS_ARE_DROPPED_NO_MERGING, or "
        "CHANS_ARE_DROPPED_SAME_CONTENT_MERGING")
    ("sng" , po::value<std::string>()->default_value("-"),
        "input sng file")
    ("dot" , po::value<std::string>()->default_value("-"),
        "output dot file")
    ;

  po::options_description privateOptions;
  privateOptions.add_options()
#ifndef SGXUTILS_DEBUG_OUTPUT
    ("debug", po::value<size_t>()->default_value(0), "turn on debug mode")
#endif //SGXUTILS_DEBUG_OUTPUT
    ;
  
  po::positional_options_description pod;
  pod.add("sng", 1);
  pod.add("dot", 1);
  
  // All options
  po::options_description od;
  od.add(publicOptions);
  od.add(privateOptions);
  
  po::variables_map vm;
  
  try {
    store(po::command_line_parser(argc, argv)
      .options(od)
      .positional(pod)
      .extra_parser(poNoPrefixHandler)
      .run(), vm);
    notify(vm);
  
    if (vm.count("help")) {
      std::cout << publicOptions << std::endl;
      return 0;
    }
    if (!vm.count("sng")) {
      throw std::runtime_error("no sng file for input specified");
    }
    if (!vm.count("dot")) {
      throw std::runtime_error("no dot file for output specified");
    }
    
#ifndef NDEBUG
    int   debugLevel = Debug::None.level - vm["debug"].as<size_t>();
    outDbg.setLevel(debugLevel < 0 ? 0 : debugLevel);
    outDbg << Debug::High;
#else  //NDEBUG
    if (vm["debug"].as<size_t>() != 0)
      std::cerr << prgname << ": Warning --debug support not compiled in!" << std::endl;
#endif //NDEBUG
    
    CoSupport::Streams::AIStream in(std::cin, vm["sng"].as<std::string>(), "-");
    CoSupport::Streams::AOStream out(std::cout, vm["dot"].as<std::string>(), "-");
    
    smoc::SNG::Graph sngGraph = smoc::SNG::loadSNG(in);

    smoc::SNG::Transform transform = vm["transform"].as<smoc::SNG::Transform>();

    if (transform != smoc::SNG::Transform::FIFOS_NO_MERGING)
      sngGraph = smoc::SNG::transform(sngGraph, transform);

    smoc::SNG::VertexPropertyWriter vpw(sngGraph);
    smoc::SNG::EdgePropertyWriter   epw(sngGraph);
    boost::write_graphviz(out, sngGraph, vpw, epw);
  } catch (std::exception &e) {
    std::cerr << prgname << ": " << e.what() << std::endl;
    return 1;
  }
  
  return 0;
}
