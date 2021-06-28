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

#include <iostream>
#include <sstream>
#include <string>

#include <cstdio>

#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/AlternateStream.hpp>

#include <CoSupport/String/color.hpp>

#include <CoSupport/boost/program_options/value_semantic.hpp>

#include <CoSupport/XML/Xerces/Handler.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#include <boost/algorithm/string.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/connected_components.hpp>
//#include <boost/graph/strong_components.hpp>
#include <boost/graph/graphviz.hpp>
//#include <boost/graph/topological_sort.hpp>
#include <CoSupport/boost/graph/undirect_graph.hpp>
#include <CoSupport/boost/graph/graph_traits.hpp>
#include <CoSupport/boost/graph/properties.hpp>

#include <boost/property_map/property_map.hpp>
#include <CoSupport/boost/property_map/compose_property_map.hpp>
//#include <boost/property_map/compose_property_map.hpp>
//#include <boost/graph/property_maps/container_property_map.hpp>

//#include <nganalysis/SDF.hpp>
//#include <nganalysis/CSDF.hpp>

#include "sng-xsd.c"

std::string prgname = "???";

namespace po  = CoSupport::boost::program_options;
namespace fs  = ::boost::filesystem;

using ::CoSupport::Streams::Debug;
using ::CoSupport::Streams::ScopedIndent;

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

/*
//typedef SGX::BGL::P2PGraph::ConverterExperiment  BGLConverter;
class BGLConverter:
  public SGX::BGL::P2PGraph::SliceCSDFAttributes<
    SGX::BGL::P2PGraph::SliceProcessChannelNames<
      SGX::BGL::P2PGraph::SliceChannelSizeAndInitialTokens<
        SGX::BGL::P2PGraph::ConverterBase
  > > >::ImplType<
    // vertex properties
    boost::property<SystemCoDesigner::NGAnalysis::SDF::vertex_repcount_t, size_t>,
    // edge properties
    boost::no_property,
    // graph properties
    boost::no_property // don't need graph properties
  >
{
  typedef SGX::BGL::P2PGraph::SliceCSDFAttributes<
    SGX::BGL::P2PGraph::SliceProcessChannelNames<
      SGX::BGL::P2PGraph::SliceChannelSizeAndInitialTokens<
        SGX::BGL::P2PGraph::ConverterBase
  > > >::ImplType<
    // vertex properties
    boost::property<SystemCoDesigner::NGAnalysis::SDF::vertex_repcount_t, size_t>,
    // edge properties
    boost::no_property,
    // graph properties
    boost::no_property // don't need graph properties
  >                     parent_type;
  typedef BGLConverter  this_type;
public:
  BGLConverter(SystemCoDesigner::SGX::ProblemGraph::Ref pg)
    : parent_type(pg) {}

  class VertexPropertyWriter
  : public parent_type::VertexPropertyWriter {
    typedef VertexPropertyWriter               this_type;
    typedef parent_type::VertexPropertyWriter  base_type;
  protected:
    const GraphPartitioning &gp;
  public:
    VertexPropertyWriter(const BGLConverter::graph &g, const GraphPartitioning &gp)
      : base_type(g), gp(gp) {}

    virtual ~VertexPropertyWriter() {}
  protected:

    /// This function is reponsible for collecting the label string
    /// belonging to the associated properties.
    /// Note that overloaded functions must call the parents
    /// so that all properties can be taken into account.
    virtual void collectAttributes(const BGLConverter::vertex_descriptor vd) {
      GraphPartitioning::const_iterator iter;

      base_type::collectAttributes(vd);

      if ((iter = gp.find(get(::boost::vertex_name, g, vd))) != gp.end()) {
        char                     colorStr[8];
        CoSupport::String::Color color = CoSupport::String::getColor(iter->second);

        std::snprintf(colorStr, sizeof(colorStr), "#%02X%02X%02X",
            static_cast<unsigned int>(color.r()),
            static_cast<unsigned int>(color.g()),
            static_cast<unsigned int>(color.b()));
        colorStr[sizeof(colorStr)-1] = '\0';
        this->attributes["style"] = "filled";
        this->attributes["color"] = colorStr;
      }
      SystemCoDesigner::NGAnalysis::MoC moc = get(SystemCoDesigner::NGAnalysis::vertex_moc, g, vd);
      if (moc == SystemCoDesigner::NGAnalysis::MoC::SDF ||
          moc == SystemCoDesigner::NGAnalysis::MoC::CSDF) {
        this->attributes["label"] += "(x" + std::to_string(get(SystemCoDesigner::NGAnalysis::SDF::vertex_repcount, g, vd)) + ")";
      }
    }
  };
};
*/

void loadSNG(std::istream &in) {

  CoSupport::XML::Xerces::Handler sng;

  sng.setTopElementName(XMLCH("networkGraph"));
  sng.setXSDUrl(XMLCH("sng.xsd"));
  sng.setXSD(sngXSD, sizeof(sngXSD));
  sng.load(in);




}


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
    ("sng" , po::value<std::string>()->default_value("-"), "input sng file")
    ("dot" , po::value<std::string>()->default_value("-"), "output dot file")
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
    
    CoSupport::XML::Xerces::Handler sng();


    loadSNG(in);

    //SGX::problemGraphToDot(ngx.problemGraph(), out);
  } catch (std::exception &e) {
    std::cerr << prgname << ": " << e.what() << std::endl;
    return 1;
  }
  
  return 0;
}
