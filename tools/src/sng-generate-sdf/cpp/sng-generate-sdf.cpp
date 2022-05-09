// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *  2021-2022 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include "config.h"

#ifdef SYSTEMOC_ENABLE_SGX
# include <sgx.hpp>
# include <sgxutils/ASTTools.hpp>
# include <sgxutils/DebugOStream.hpp>
#endif //SYSTEMOC_ENABLE_SGX

#include <smoc/sng.hpp>

#include <nganalysis/SDF.hpp>
#include <nganalysis/DropChannelVertices.hpp>

//#include <sgxutils/RecursiveProblemGraphObjVisitor.hpp>
//#include <sgxutils/BGL/P2PGraph.hpp>

#include <CoSupport/boost/graph/undirect_graph.hpp>
#include <CoSupport/boost/graph/graph_traits.hpp>

#include <CoSupport/compatibility-glue/nullptr.h>
#include <CoSupport/Random.hpp>

#include <CoSupport/Streams/AlternateStream.hpp>
#include <CoSupport/Streams/stl_output_for_map.hpp>
#include <CoSupport/Streams/stl_output_for_vector.hpp>

#include <CoSupport/String/convert.hpp>
#include <CoSupport/String/Concat.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
//#include <boost/program_options/positional_options.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#include <boost/type_traits/is_integral.hpp>

#include <boost/math/common_factor_rt.hpp>

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include <boost/property_map/property_map.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/property_maps/container_property_map.hpp>
#include <boost/property_map/function_property_map.hpp>
//#include <boost/graph/property_maps/constant_property_map.hpp>
#include <boost/graph/connected_components.hpp>
#include <boost/graph/graphviz.hpp>

#include <boost/graph/filtered_graph.hpp>

//#include <boost/graph/strong_components.hpp>
//#include <boost/graph/topological_sort.hpp>
//#include <boost/graph/push_relabel_max_flow.hpp>
//#include <boost/graph/stoer_wagner_min_cut.hpp>

//#include <stdint.h>

#include <iostream>
#include <fstream>
#include <limits>

#include <set>
#include <list>
#include <map>
#include <vector>
#include <queue>

#include <memory> // for std::unique_ptr
#include <functional>

// for ::open
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

std::string prgname = "???";

//namespace SGX = SystemCoDesigner::SGXUtils;
namespace NGA = SystemCoDesigner::NGAnalysis;
namespace po  = boost::program_options;
namespace fs  = boost::filesystem;

// Some usefull functions from CoSupport.
using ::CoSupport::String::Concat;
using ::CoSupport::Streams::Debug;

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

//using ::SystemCoDesigner::SGXUtils::outDbg;

/*
void SDF::dump(graph &g) {
#ifdef SGXUTILS_DEBUG_OUTPUT
  if (!outDbg.isVisible(Debug::Low))
    return;
  static int nr = 0;
  
  std::ostringstream name;
  
  name << "sg" << nr++ << ".dot";
  std::ofstream o(name.str().c_str());
  
  VertexPropertyWriter vpw(g);
  EdgePropertyWriter   epw(g);
  boost::write_graphviz(o, g, vpw, epw);
#endif // SGXUTILS_DEBUG_OUTPUT
}
*/

using CoSupport::Random::RandomGenerator;

using CoSupport::Random::randomSource;

// Mersenne twister pseudo randomness source
boost::random::mt19937 randomSourceTopology;

using namespace smoc::SNG;

struct ClusterOptions {
  size_t                  count = 0;
  size_t                  weight;
  std::string             type;
  RandomGenerator<size_t> repCount;
  RandomGenerator<size_t> inputCount;
  RandomGenerator<size_t> outputCount;
  RandomGenerator<size_t> actorCount;
  RandomGenerator<double> actorOutDegree;
  RandomGenerator<double> actorMulticastDegree;
  RandomGenerator<double> outputOutDegree;
  RandomGenerator<double> outputMulticastDegree;
};

typedef std::vector<VertexDescriptor> VertexMap;

struct ClusterInformation {
  ClusterInformation(ClusterOptions &clusterOptions)
    : clusterOptions(clusterOptions) {}

  ClusterOptions &clusterOptions;

  VertexMap vdInputs;
  VertexMap vdOutputs;
  VertexMap vdActors;
};

typedef std::vector<VertexMap>        ClusterMap;

Graph::vertex_descriptor addChannel(
    Graph             &g
  , std::string const &prefix
  , Graph::vertex_descriptor                     vdSrcActor
  , std::vector<Graph::vertex_descriptor> const &vdSnkActors)
{
  static size_t fifoCount = 0;

  Graph::vertex_descriptor vdChannel = add_vertex(g);

  assert(g[vdSrcActor].type == VertexInfo::ACTOR);
  size_t repSrc = g[vdSrcActor].actor.repCount;
  size_t repLcm = repSrc;
  for (Graph::vertex_descriptor vdSnkActor : vdSnkActors) {
    assert(g[vdSnkActor].type == VertexInfo::ACTOR);
    size_t repSnk = g[vdSnkActor].actor.repCount;
    repLcm = boost::math::lcm(repLcm, repSnk);
  }

  VertexInfo &vi = g[vdChannel];
  vi.type = VertexInfo::FIFO;
  vi.name = Concat(prefix)("c")(++fifoCount);
  vi.fifo.tokenSize = -1;

  size_t prod = repLcm/repSrc; // production rate
  {
    EdgeDescriptor edProd = add_edge(vdSrcActor, vdChannel, g).first;
    g[edProd].name = Concat("o")(out_degree(vdSrcActor, g));
    g[edProd].tokenSize = vi.fifo.tokenSize;
    g[edProd].tokens = prod;
  }
  size_t maxCons = 0;
  for (Graph::vertex_descriptor vdSnkActor : vdSnkActors) {
    EdgeDescriptor edCons = add_edge(vdChannel, vdSnkActor, g).first;
    size_t repSnk = g[vdSnkActor].actor.repCount;
    size_t cons = repLcm/repSnk; // consumption rate
    g[edCons].name = Concat("i")(in_degree(vdSnkActor, g));
    g[edCons].tokenSize = vi.fifo.tokenSize;
    g[edCons].tokens = cons;
    maxCons = std::max(maxCons, cons);
  }

  {
    // Use lower bound for FIFO buffer sizes
    vi.fifo.capacity = prod+maxCons-1;
    vi.fifo.delay = 0;
  }
  return vdChannel;
}

Graph::vertex_descriptor addChannel(
    Graph             &g
  , std::string const &prefix
  , Graph::vertex_descriptor vdSrcActor
  , Graph::vertex_descriptor vdSnkActor)
{
  std::vector<Graph::vertex_descriptor> vdSnkActors;
  vdSnkActors.push_back(vdSnkActor);
  return addChannel(g, prefix, vdSrcActor, vdSnkActors);
}

Graph::vertex_descriptor addInput(
    Graph             &g
  , std::string const &prefix
  , Graph::vertex_descriptor vdSnkActor)
{
  static size_t inputCount = 0;

  VertexDescriptor vdInput = add_vertex(g);
  VertexInfo &viInput = g[vdInput];
  viInput.type = VertexInfo::REGISTER;
  viInput.name = Concat(prefix)("regIn")(++inputCount);
  EdgeDescriptor edInput = add_edge(vdInput, vdSnkActor, g).first;
  EdgeInfo &eiInput = g[edInput];
  eiInput.name   = "in";
  eiInput.tokens = 1;
  return vdInput;
}

Graph::vertex_descriptor addOutput(
    Graph             &g
  , std::string const &prefix
  , Graph::vertex_descriptor vdSrcActor)
{
  static size_t outputCount = 0;

  VertexDescriptor vdOutput = add_vertex(g);
  VertexInfo &viOutput = g[vdOutput];
  viOutput.type = VertexInfo::REGISTER;
  viOutput.name = Concat(prefix)("regOut")(++outputCount);
  EdgeDescriptor edOutput = add_edge(vdSrcActor, vdOutput, g).first;
  EdgeInfo &eiOutput = g[edOutput];
  eiOutput.name   = "out";
  eiOutput.tokens = 1;
  return vdOutput;
}

ClusterInformation generateCluster(
    Graph             &g
  , std::string const &prefix
  , ClusterOptions    &clusterOptions
  , size_t            maxActors
  , bool              createDAG
  , bool              allowSelfEdges)
{
  ClusterInformation clusterInformation(clusterOptions);

  clusterOptions.count++;

  size_t clusterSize    = std::min(clusterOptions.actorCount(), maxActors);
  size_t clusterInputs  = std::min(clusterOptions.inputCount(), clusterSize*3/2);
  size_t clusterOutputs = std::min(clusterOptions.outputCount(), clusterSize*3/2);
  assert(clusterSize >= 1);
  clusterInformation.vdActors.resize(clusterSize);

  std::priority_queue<size_t> actorOutDegrees;
  std::priority_queue<size_t> actorMulticastDegrees;

  for (size_t j = 0; j < clusterSize; ++j) {
    VertexDescriptor vd = clusterInformation.vdActors[j] = add_vertex(g);
    VertexInfo &vi = g[vd];
    vi.type = VertexInfo::ACTOR;
    vi.name = Concat(prefix)(clusterOptions.type)(clusterOptions.count)(".a")(j+1);
    vi.actor.repCount = clusterOptions.repCount();
    if (j < clusterInputs)
      clusterInformation.vdInputs.push_back(vd);
    if (j >= clusterSize - clusterOutputs) {
      clusterInformation.vdOutputs.push_back(vd);
      continue;
    }
    {
      double actorOutDegree = clusterOptions.actorOutDegree();
      if (actorOutDegree > 0)
        actorOutDegrees.push(actorOutDegree);
    }
    {
      double actorMulticastDegree = clusterOptions.actorMulticastDegree();
      if (actorMulticastDegree > 1)
        actorMulticastDegrees.push(actorMulticastDegree);
    }
  }

  for (size_t j = 0; j < clusterSize; ++j) {
    VertexDescriptor vdSrc = clusterInformation.vdActors[j];

    int actorOutDegree = actorOutDegrees.empty()
        ? 0 : actorOutDegrees.top();
    if (!actorOutDegrees.empty())
      actorOutDegrees.pop();
    int actorMulticastDegree = actorMulticastDegrees.empty()
        ? 1 : actorMulticastDegrees.top();
    if (!actorMulticastDegrees.empty())
      actorMulticastDegrees.pop();
//      std::cout
//          << "actorOutDegree: " << actorOutDegree
//          << ", actorMulticastDegree: " << actorMulticastDegree << std::endl;
    std::vector<VertexDescriptor> vdSnks;
    if (createDAG) {
      for (size_t k = std::max(j+1, clusterInputs); k < clusterSize; ++k)
        vdSnks.push_back(clusterInformation.vdActors[k]);
    } else
      for (VertexDescriptor vdSnk : clusterInformation.vdActors)
        if (allowSelfEdges || vdSrc != vdSnk)
          vdSnks.push_back(vdSnk);
    while (!vdSnks.empty() && actorOutDegree > 0) {
      std::vector<VertexDescriptor> vdMulticast;
      do {
        size_t snkIndex = boost::random::uniform_int_distribution<>(0, vdSnks.size()-1)(randomSourceTopology);
        vdMulticast.push_back(vdSnks[snkIndex]);
        vdSnks.erase(vdSnks.begin() + snkIndex);
        --actorMulticastDegree;
        --actorOutDegree;
      } while (!vdSnks.empty() && actorMulticastDegree > 0 && actorOutDegree > 0);
      addChannel(g, prefix, vdSrc, vdMulticast);
    }
    if (actorOutDegree)
      std::cerr << "Warning: could not create " << actorOutDegree << " output edges for " << g[vdSrc].name << std::endl;
  }
  return clusterInformation;
}

void checkGraphDeadlockFree(Graph &g) {
#ifndef NDEBUG
  VertexNameMap     vertexNameMap  = get(&VertexInfo::name, g);
  ActorRepCountMap  actorRepCountMap(g);
  FIFODelayMap      fifoDelayMap(g);
  FIFOCapacityMap   fifoCapacityMap(g);
  EdgeTokensMap     edgeTokensMap = get(&EdgeInfo::tokens, g);

  typedef NGA::DropChannelVertices<Graph> SDF;
  SDF gSDF(g, [&g] (VertexDescriptor vd)
      { return g[vd].type != VertexInfo::ACTOR; });

//    boost::property_map<DFG, boost::vertex_index_t>::const_type flummy
//      = get(boost::vertex_index, dfg);

  SDF::EdgeConsMap<EdgeTokensMap>::type  sdfEdgeConsMap(edgeTokensMap);
  SDF::EdgeProdMap<EdgeTokensMap>::type  sdfEdgeProdMap(g, edgeTokensMap);
  SDF::EdgeNameMap<VertexNameMap>::type  sdfEdgeNameMap(g, vertexNameMap);
  SDF::EdgeDelayMap<FIFODelayMap>::type  sdfEdgeDelayMap(g, fifoDelayMap);
  SDF::EdgeCapMap<FIFOCapacityMap>::type sdfEdgeCapMap(g, fifoCapacityMap);

  typedef std::map<SDF::edge_descriptor, size_t> SDFEdgeTokensStorage;
  SDFEdgeTokensStorage sdfEdgeTokensStorage;
  boost::associative_property_map<SDFEdgeTokensStorage> sdfEdgeTokensMap(sdfEdgeTokensStorage);

  typedef std::vector<size_t> ActorRepLeftStorage;
  ActorRepLeftStorage actorRepLeftStorage(num_vertices(gSDF));
  boost::container_property_map<SDF, VertexDescriptor, ActorRepLeftStorage>
    actorRepLeftMap(actorRepLeftStorage, gSDF);

  bool noDeadlocks = SystemCoDesigner::NGAnalysis::SDF::isGraphDeadlockFree(
    gSDF,
    actorRepCountMap,
    actorRepLeftMap,
    sdfEdgeConsMap,
    sdfEdgeProdMap,
    sdfEdgeDelayMap,
    sdfEdgeCapMap,
    sdfEdgeTokensMap);
  if (!noDeadlocks) {
    std::ofstream out("deadlocks-error.dot");
    VertexPropertyWriter vpw(g);
    EdgePropertyWriter   epw(g);
    boost::write_graphviz(out, g, vpw, epw);
    assert(!"Internal error graph has deadlocks, see deadlocks-error.dot!\n");
  }
#endif //NDEBUG
}

#if 0 && defined(SYSTEMOC_ENABLE_SGX)
void connectPorts(
    SGX::Port &actorOutPort,
    SGX::Port &actorInPort,
    SGX::Fifo &channel,
    const std::string &type,
    size_t capacity, size_t delay, size_t tokenSize)
{
  channel.name() = Concat("cf_")(actorOutPort.owner()->name())("_")(actorInPort.owner()->name());
  channel.type() = type;
  channel.size() = capacity;
  if (delay > 0) {
    SGX::TokenList::Ref initialTokens = channel.initialTokens();
    for (size_t i = delay; i > 0; --i) {
      SGX::Token t;
      t.value() = "4711";
      initialTokens.push_back(t);
    }
  }
  if (tokenSize > 0) {
    SGX::Attribute attrTokenSize("smoc-token-size", tokenSize);
    channel.attributes().push_back(attrTokenSize);
  }
  SGX::Port channelInPort("in");
  SGX::Port channelOutPort("out");
  channelInPort.direction()  = SGX::Port::In;
  channelOutPort.direction() = SGX::Port::Out;
  channel.ports().push_back(channelInPort);
  channel.ports().push_back(channelOutPort);
  actorInPort.direction()    = SGX::Port::In;
  actorOutPort.direction()   = SGX::Port::Out;
  channelInPort.actorPort()  = actorOutPort.toPtr();
  channelOutPort.actorPort() = actorInPort.toPtr();
}

void saveNGX(Graph &g, std::ostream &out) {
  SGX::NetworkGraphAccess ngx;

  SGX::ProblemGraph pg;
  ngx.problemGraphPtr() = pg.toPtr();

  SGX::Actor::Ptr             actorPtrStor[num_vertices(g)];
  boost::iterator_property_map<SGX::Actor::Ptr *, SDF::PropVertexIndexMap>
                              actorPtrMap(actorPtrStor, vertexIndexMap);
  SGX::FiringTransition::Ptr  transitionPtrStor[num_vertices(g)];
  boost::iterator_property_map<SGX::FiringTransition::Ptr *, SDF::PropVertexIndexMap>
                              transitionPtrMap(transitionPtrStor, vertexIndexMap);

  SGX::ActorList::Ref   actors    = pg.actors();
  SGX::ChannelList::Ref channels  = pg.channels();

  //FIXME: generate ngx from g
  for (SDF::vertex_iterator_pair vip = vertices(g);
       vip.first != vip.second;
       ++vip.first) {
    bool isSource = g[*vip.first].name == "source";

    SGX::Actor actor(g[*vip.first].name);
    actors.push_back(actor);
    actorPtrMap[*vip.first] = actor.toPtr();
    SGX::FiringFSM        fsm;
    SGX::FiringState      srun;
    SGX::FiringTransition trun;
    fsm.states().push_back(srun);
    fsm.startState() = srun.toPtr();
    srun.outTransitions().push_back(trun);
    trun.dstState() = srun.toPtr(); // self loop

    if (isSource) {
      SGX::FiringState       sfin;
      SGX::FiringTransition  tfin;
      SGX::Function          f;

      fsm.states().push_back(sfin);
      srun.name() = "run";
      srun.outTransitions().push_back(tfin);

      {
        SGX::ASTNodeVar srcIter;
        srcIter.name() = "SRC_ITER";

        SGX::ASTNodeVar srcIters;
        srcIters.name() = "SRC_ITERS";

        SGX::ASTNodeBinOp iterCmp;
        iterCmp.opType() = SGX::OpBinT::Lt;
        iterCmp.leftNode() = &srcIter;
        iterCmp.rightNode() = &srcIters;

        trun.action()            = f.toPtr();
        // Add condition SRC_ITER < SRC_ITERS to transition trun.
        trun.activationPattern() = iterCmp.toPtr();
        // FIXME: We do not even have infrastructure for this bad hack (yet),
        // so we do some even nastier things...
        f.name() = "action|void action() { ++SRC_ITER; }";
      }
      {
        tfin.dstState() = sfin.toPtr(); // to end state

        SGX::ASTNodeVar srcIter;
        srcIter.name() = "SRC_ITER";

        SGX::ASTNodeVar srcIters;
        srcIters.name() = "SRC_ITERS";

        SGX::ASTNodeBinOp top;
        top.opType() = SGX::OpBinT::Ge;
        top.leftNode() = &srcIter;
        top.rightNode() = &srcIters;

        tfin.activationPattern() = top.toPtr();
      }
    }

    actor.schedule() = fsm.toPtr();
    transitionPtrMap[*vip.first] = trun.toPtr();
  }

  SDF::PropEdgeSizeTMap edgeTSizeMap = get(&SDF::EdgeInfo::tokenSize, g);

  for (SDF::edge_iterator_pair eip = edges(g);
       eip.first != eip.second;
       ++eip.first) {
    SGX::Fifo channel(g[*eip.first].name);

    channels.push_back(channel);

    bool isFromSource = g[source(*eip.first, g)].name == "source";

    SGX::Actor::Ptr            &srcActor      = actorPtrMap[source(*eip.first, g)];
    SGX::FiringTransition::Ptr &srcTransition = transitionPtrMap[source(*eip.first, g)];
    SGX::Actor::Ptr            &snkActor      = actorPtrMap[target(*eip.first, g)];
    SGX::FiringTransition::Ptr &snkTransition = transitionPtrMap[target(*eip.first, g)];
    SGX::Port actorInPort(Concat("in_")(g[*eip.first].name));
    SGX::Port actorOutPort(Concat("out_")(g[*eip.first].name));
    srcActor->ports().push_back(actorOutPort);
    snkActor->ports().push_back(actorInPort);
    const char *fifoTokenType = isFromSource ? "void" : "int";
    connectPorts(actorOutPort, actorInPort, channel, fifoTokenType,
        edgeCapMap[*eip.first], edgeDelayMap[*eip.first], edgeTSizeMap[*eip.first]);
    SGX::ASTTools::setCommunicate(*srcTransition,
      SGX::ASTTools::TokenComReq(actorOutPort.toPtr(), g[*eip.first].prod));
    SGX::ASTTools::setCommunicate(*snkTransition,
      SGX::ASTTools::TokenComReq(actorInPort.toPtr(), g[*eip.first].cons));
  }
  ngx.save(out);
}
#endif //SYSTEMOC_ENABLE_SGX

class NewClusterParser: public po::typed_value<std::string> {
  typedef po::typed_value<std::string> base_type;
public:
  NewClusterParser()
    : base_type(nullptr), optionBlockIdx(-1) {}
  void addClusrerOptionParser(boost::function<void ()> const &fun)
    { functions.push_back(fun); }
protected:
  unsigned min_tokens() const { return 0; }
  unsigned max_tokens() const { return 1; }

  void xparse(boost::any &value_store,
              const std::vector<std::string> &new_tokens) const {
    ++optionBlockIdx;
    for (std::vector<boost::function<void ()> >::const_iterator iter = functions.begin();
         iter != functions.end();
         ++iter)
      (*iter)();
    boost::any value_sub_store;
    base_type::xparse(value_sub_store, new_tokens);
    if (value_store.empty())
      value_store = storage_type();
    storage_type &storage = boost::any_cast<storage_type &>(value_store);
    storage[optionBlockIdx] = boost::any_cast<value_type>(value_sub_store);
  }
protected:
  typedef std::string                  value_type;
  typedef std::map<size_t, value_type> storage_type;

  std::vector<boost::function<void ()> > functions;

  mutable int optionBlockIdx;
};

template <typename T>
struct ClusterOptionParser: public po::typed_value<T> {
  typedef ClusterOptionParser<T>  this_type;
  typedef po::typed_value<T>      base_type;
public:
  ClusterOptionParser(NewClusterParser *newClusterParser)
    : base_type(nullptr), optionBlockIdx(-1), applyDefaultCalled(false)
  {
    newClusterParser->addClusrerOptionParser(boost::bind(&this_type::newOptionBlock, this));
  }

  void xparse(boost::any &value_store,
              const std::vector<std::string> &new_tokens) const {
    if (optionBlockIdx < 0)
      throw po::multiple_occurrences();
    if (value_store.empty())
      value_store = storage_type();
    storage_type *storage = boost::any_cast<storage_type>(&value_store);
    assert(storage != nullptr);
    base_type::xparse((*storage)[optionBlockIdx], new_tokens);
  }
protected:
  typedef T                             value_type;
  typedef std::map<size_t, boost::any>  storage_type;

  int                 optionBlockIdx;
  mutable bool        applyDefaultCalled;
  mutable boost::any  m_default_value;

  bool apply_default(boost::any &value_store) const {
    assert(value_store.empty());

    applyDefaultCalled = true;
    if (base_type::apply_default(m_default_value)) {
      assert(!m_default_value.empty());
      value_store = storage_type();
      return true;
    } else {
      assert(m_default_value.empty());
      return false;
    }
  }

  void notify(const boost::any &value_store) const {
    assert(!value_store.empty());
    storage_type const           &storage = boost::any_cast<storage_type>(value_store);
    std::map<size_t, value_type>  converted_storage;

    if (!applyDefaultCalled) {
      if (base_type::apply_default(m_default_value))
        assert(!m_default_value.empty());
      else
        assert(m_default_value.empty());
    }
    for (int i = 0; i <= optionBlockIdx; ++i) {
      storage_type::const_iterator iter = storage.find(i);
      if (iter != storage.end()) {
        sassert(
          converted_storage.insert(std::make_pair(
            static_cast<size_t>(i),
            boost::any_cast<value_type>(iter->second))).second);
      } else if (!m_default_value.empty()) {
        sassert(
          converted_storage.insert(std::make_pair(
            static_cast<size_t>(i),
            boost::any_cast<value_type>(m_default_value))).second);
      }
    }
    // This const_cast is a hack!
    const_cast<boost::any &>(value_store) = converted_storage;
    base_type::notify(value_store);
  }

  void newOptionBlock() {
    ++optionBlockIdx;
  }
};

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
int main(int argc, char** argv) {
  if (argc >= 1 && argv[0] != nullptr)
    prgname = basename(fs::path(argv[0]));
  
  std::stringstream sstr;
  sstr << "Usage: " << prgname << " [options]\n\n"
          "Random generators can be specified as follows:\n"
          "  <number>                              Constant number no randomness\n"
          "  uniform:<min>-<max>                   Uniform distribution from min to max\n"
          "                                        The max value is inclusive for integer\n"
          "                                        distributions, but exclusive for\n"
          "                                        floating point distributions.\n"
          "  normal:m<mean>,s<sigma>               Normal distribution with given mean and\n"
          "                                        sigma\n"
          "  urn:<value>x<weight>,...              Urn containing values with an optional\n"
          "                                        weight\n"
          "\n"
          "Available options";
  po::options_description publicOptions(sstr.str());
  po::options_description privateOptions;
  
  {
    privateOptions.add_options()
#ifdef NDEBUG
      ("debug", po::value<size_t>()->default_value(0), "turn on debug mode")
#endif //defined(NDEBUG)
      ;
  }
  {
    using CoSupport::Random::RandomConst;
    using CoSupport::Random::RandomUniform;
    using CoSupport::Random::RandomUrn;
    using CoSupport::Random::RandomNormal;

    // urn:1,2,3,6,5,10,15,12,18,30
    std::vector<std::pair<size_t, size_t> > repCounts;
    repCounts.push_back(std::make_pair(1,1));
    repCounts.push_back(std::make_pair(1,2));
    repCounts.push_back(std::make_pair(1,3));
    repCounts.push_back(std::make_pair(1,6));
    repCounts.push_back(std::make_pair(1,5));
    repCounts.push_back(std::make_pair(1,10));
    repCounts.push_back(std::make_pair(1,15));
    repCounts.push_back(std::make_pair(1,12));
    repCounts.push_back(std::make_pair(1,18));
    repCounts.push_back(std::make_pair(1,30));

#ifndef NDEBUG
    sstr.str(""); // reset string stream
    sstr << "set debug level; level 0 is off; level " << Debug::None.level << " is most verbose";
#endif //!defined(NDEBUG)

    NewClusterParser                  *ncp;
    std::unique_ptr<NewClusterParser> _ncp(ncp = new NewClusterParser());
    publicOptions.add_options()
      ("help", "produce help message")
#ifndef NDEBUG
      ("debug", po::value<size_t>()->default_value(0), sstr.str().c_str())
#endif //!defined(NDEBUG)
      ("seed", po::value<uint64_t>(), "specify random seed")
      ("dump-seed" , po::value<std::string>(), "dump used seed into file")
      ("source-actor", po::value<bool>()->default_value(false, "no")->implicit_value(true),
          "enable generation of source actor")
      ("ngx" , po::value<std::string>(), "output network graph xml file")
      ("sng" , po::value<std::string>(), "output simple network graph xml file")
      ("dot" , po::value<std::string>(), "output network graph in dot format")
      ("prefix", po::value<std::string>(), "prefix for actor and channel names")
      ("actor-count", po::value<RandomGenerator<size_t> >()
          ->default_value(RandomConst<size_t>(6), "6"),
          "random generator specifying the number of actors to generate")
      ("self-edges", po::value<bool>()->default_value(false, "no")->implicit_value(true),
          "allow generation of self edges")
      ("create-dag", po::value<bool>()->default_value(false, "no")->implicit_value(true),
          "create a directed acyclic graph")
      ("extra-delay-factor", po::value<RandomGenerator<double> >()
          ->default_value(RandomConst<double>(0.0), "0"),
          "add factor * cons(edge) extra initial tokens to the edge")
      ("extra-delay-factor-inter-cluster", po::value<RandomGenerator<double> >()
          ->default_value(RandomUniform<double>(0.3,3.5), "uniform:0.3-3.5"),
          "add factor * cons(edge) extra initial tokens to the edge")
      ("random-cluster-selection", po::value<bool>()->default_value(false, "no")->implicit_value(true),
          "randomly selects a cluster type for generation")
      ("graph-internal-read-ratio", po::value<RandomGenerator<double> >(),
           "random generator specifying the ration between data read and data written, i.e., read bytes/written bytes, for the internal communication")
      ("graph-internal-communication", po::value<RandomGenerator<size_t> >(),
           "random generator specifying the amount of internal communication in bytes during one graph iteration")
      ("graph-input-communication", po::value<RandomGenerator<size_t> >(),
          "random generator specifying the amount of input data in bytes during one graph iteration")
      ("graph-output-communication", po::value<RandomGenerator<size_t> >(),
          "random generator specifying the amount of output data in bytes during one graph iteration")
      ("graph-communication-scaling", po::value<RandomGenerator<double> >()
          ->default_value(RandomConst<double>(1), "1"),
           "random generator specifying the scaling of the token size for each fifo in the graph")
      ("new-cluster", _ncp.release()->implicit_value("a"), "begin new cluster; options (--cluster-xxx) for the new cluster must be specified after this option")
      ("cluster-weight", (new ClusterOptionParser<size_t>(ncp))
          ->default_value(1),
          "specify the weight for the instantiation of this cluster")
      ("cluster-rep-count", (new ClusterOptionParser<RandomGenerator<size_t> >(ncp))
  //      ->multitoken()
          ->default_value(RandomUrn<size_t>(repCounts), "urn:1,2,3,6,5,10,15,12,18,30"),
          "specify possible repetition counts for actors in this cluster")
      ("cluster-actor-count", (new ClusterOptionParser<RandomGenerator<size_t> >(ncp))
          ->default_value(RandomUniform<size_t>(5,15), "uniform:5-15"),
          "random generator specifying the number of actors in this cluster")
      ("cluster-input-count", (new ClusterOptionParser<RandomGenerator<size_t> >(ncp))
          ->default_value(RandomUniform<size_t>(5,15), "uniform:1-3"),
          "random generator specifying the number of input actors in this cluster")
      ("cluster-output-count", (new ClusterOptionParser<RandomGenerator<size_t> >(ncp))
          ->default_value(RandomUniform<size_t>(5,15), "uniform:1-3"),
          "random generator specifying the number of output actors in this cluster")
      ("cluster-actor-out-degree", (new ClusterOptionParser<RandomGenerator<double> >(ncp))
          ->default_value(RandomNormal<double>(2, 1), "normal:m2,s1"),
          "random generator specifying output degree of the actors in this cluster")
      ("cluster-actor-multicast-degree", (new ClusterOptionParser<RandomGenerator<double> >(ncp))
          ->default_value(RandomConst<double>(1), "1"),
          "random generator specifying multicast degree of the actors in this cluster")
      ("cluster-output-out-degree", (new ClusterOptionParser<RandomGenerator<double> >(ncp))
          ->default_value(RandomNormal<double>(2, 1), "normal:m8,s4"),
          "random generator specifying the output degree of the output actors in this cluster")
      ("cluster-output-multicast-degree", (new ClusterOptionParser<RandomGenerator<double> >(ncp))
          ->default_value(RandomConst<double>(1), "1"),
          "random generator specifying the multicast degree of the output actors in this cluster")
      ;
  }
  
//po::positional_options_description pod; 
//pod.add("ngx", 1);
//pod.add("clusters", 1);
  
  // All options
  po::options_description od;
  od.add(publicOptions);
  od.add(privateOptions);
  
  po::variables_map vm;
  
  try {
//  store(po::command_line_parser(argc, argv).options(od).positional(pod).run(), vm);
    store(po::command_line_parser(argc, argv)
      .options(od)
      .extra_parser(poNoPrefixHandler)
      .run(), vm);
    notify(vm);
    
    if (vm.count("help")) {
      std::cout << publicOptions << std::endl;
      return 0;
    }
    if (!vm.count("ngx") && !vm.count("dot")) {
      throw std::runtime_error("no output file specified for generated network graph!");
    }
    if (!vm.count("actor-count")) {
      throw std::runtime_error("number of actors in the network graph not specified!");
    }
    if (vm["cluster-weight"].as<std::map<size_t, size_t> >().empty()) {
      throw std::runtime_error("no clusters given, e.g., use --new-cluster --cluster-actor-count=3 to specify a cluster with 3 actors");
    }

#ifndef NDEBUG
    int   debugLevel = Debug::None.level - vm["debug"].as<size_t>();
    outDbg.setLevel(debugLevel < 0 ? 0 : debugLevel);
    outDbg << Debug::High;
#else  //NDEBUG
    if (vm["debug"].as<size_t>() != 0)
      std::cerr << prgname << ": Warning --debug support not compiled in!" << std::endl;
#endif //NDEBUG
    
    {
      uint64_t seed;
      if (vm.count("seed")) {
        seed = vm["seed"].as<uint64_t>();
      } else {
        int fd = ::open("/dev/urandom", O_RDONLY);
        if (fd == -1 || read(fd, &seed, sizeof(seed)) != sizeof(seed)) {
          std::cerr << prgname << ": can't snarf seed from /dev/urandom: " << strerror(errno) << std::endl;
          return -1;
        }
        close(fd);
      }
      //FIXME: The uint64_t seed does not contain enough entropy to really initialize the mersenne twister properly.
      randomSource.seed(seed); 
      randomSourceTopology.seed(seed);
      if (vm.count("dump-seed")) {
        CoSupport::Streams::AOStream out(std::cout, vm["dump-seed"].as<std::string>(), "-");
        out << "seed: " << seed << std::endl;
      }
    }

    std::string prefix = vm.count("prefix")
        ? vm["prefix"].as<std::string>() + "."
        : std::string();
    bool createDAG = vm["create-dag"].as<bool>();
    bool allowSelfEdges = vm["self-edges"].as<bool>() && !createDAG;

    Graph             g;
    VertexIndexMap    vertexIndexMap = get(::boost::vertex_index, g);
    VertexNameMap     vertexNameMap  = get(&VertexInfo::name, g);
    ActorRepCountMap  actorRepCountMap(g);
    ChannelTSizeMap   channelTSizeMap(g);
    FIFODelayMap      fifoDelayMap(g);
    FIFOCapacityMap   fifoCapacityMap(g);

    EdgeTokensMap   edgeTokensMap  = get(&EdgeInfo::tokens, g);

    typedef std::map<size_t, ClusterOptions> ClusterOptionsMap;

    ClusterOptionsMap clusterOptionsMap;
    size_t            clusterEndWeight = 0;
    {
      std::map<size_t, std::string> const              &type =
          vm["new-cluster"].as<std::map<size_t, std::string> >();
      std::map<size_t, size_t> const                   &weight =
          vm["cluster-weight"].as<std::map<size_t, size_t> >();
      std::map<size_t, RandomGenerator<size_t> > const &repCount =
          vm["cluster-rep-count"].as<std::map<size_t, RandomGenerator<size_t> > >();
      std::map<size_t, RandomGenerator<size_t> > const &inputCount =
          vm["cluster-input-count"].as<std::map<size_t, RandomGenerator<size_t> > >();
      std::map<size_t, RandomGenerator<size_t> > const &outputCount =
          vm["cluster-output-count"].as<std::map<size_t, RandomGenerator<size_t> > >();
      std::map<size_t, RandomGenerator<size_t> > const &actorCount =
          vm["cluster-actor-count"].as<std::map<size_t, RandomGenerator<size_t> > >();
      std::map<size_t, RandomGenerator<double> > const &actorOutDegree =
          vm["cluster-actor-out-degree"].as<std::map<size_t, RandomGenerator<double> > >();
      std::map<size_t, RandomGenerator<double> > const &actorMulticastDegree =
          vm["cluster-actor-multicast-degree"].as<std::map<size_t, RandomGenerator<double> > >();
      std::map<size_t, RandomGenerator<double> > const &outputOutDegree =
          vm["cluster-output-out-degree"].as<std::map<size_t, RandomGenerator<double> > >();
      std::map<size_t, RandomGenerator<double> > const &outputMulticastDegree =
          vm["cluster-output-multicast-degree"].as<std::map<size_t, RandomGenerator<double> > >();
      assert(type.size() == weight.size());
      assert(type.size() == repCount.size());
      assert(type.size() == actorCount.size());
      assert(type.size() == actorOutDegree.size());
      assert(type.size() == actorMulticastDegree.size());
      assert(type.size() == outputOutDegree.size());
      assert(type.size() == outputMulticastDegree.size());

      for (size_t i = 0; i < type.size(); ++i) {
        clusterEndWeight += weight.find(i)->second;
        ClusterOptions &clusterOptions        = clusterOptionsMap[clusterEndWeight-1];
        clusterOptions.type                   = type.find(i)->second;
        clusterOptions.weight                 = weight.find(i)->second;
        clusterOptions.repCount               = repCount.find(i)->second;
        clusterOptions.repCount.setRandomSource(&randomSourceTopology);
        clusterOptions.inputCount             = inputCount.find(i)->second;
        clusterOptions.inputCount.setRandomSource(&randomSourceTopology);
        clusterOptions.outputCount            = outputCount.find(i)->second;
        clusterOptions.outputCount.setRandomSource(&randomSourceTopology);
        clusterOptions.outputOutDegree        = outputOutDegree.find(i)->second;
        clusterOptions.outputOutDegree.setRandomSource(&randomSourceTopology);
        clusterOptions.outputMulticastDegree  = outputMulticastDegree.find(i)->second;
        clusterOptions.actorCount             = actorCount.find(i)->second;
        clusterOptions.actorCount.setRandomSource(&randomSourceTopology);
        clusterOptions.actorOutDegree         = actorOutDegree.find(i)->second;
        clusterOptions.actorOutDegree.setRandomSource(&randomSourceTopology);
        clusterOptions.actorMulticastDegree   = actorMulticastDegree.find(i)->second;
      }
    }
   
    typedef std::vector<ClusterInformation> ClusterMap;

    ClusterMap clusterMap;
    
    if (vm["random-cluster-selection"].as<bool>()) {
      boost::random::uniform_int_distribution<> distClusterTypeSelection(0, clusterEndWeight-1);

      size_t actorCount = vm["actor-count"].as<RandomGenerator<size_t> >()();

      while (actorCount > 0) {
        ClusterOptions &clusterOptions =
            clusterOptionsMap.lower_bound(static_cast<size_t>(distClusterTypeSelection(randomSourceTopology)))->second;
        clusterMap.push_back(generateCluster(
            g, prefix, clusterOptions, actorCount, createDAG, allowSelfEdges));
        ClusterInformation &ci = clusterMap.back();
        actorCount -= ci.vdActors.size();
      }
    } else {
      size_t actorCount = vm["actor-count"].as<RandomGenerator<size_t> >()();

      for (ClusterOptionsMap::iterator iter = clusterOptionsMap.begin();
           iter != clusterOptionsMap.end();
           ++iter) {
        ClusterOptions &clusterOptions = iter->second;
        while (clusterOptions.count < clusterOptions.weight && actorCount > 0) {
          clusterMap.push_back(generateCluster(
              g, prefix, clusterOptions, actorCount, createDAG, allowSelfEdges));
          ClusterInformation &ci = clusterMap.back();
          actorCount -= ci.vdActors.size();
        }
      }
    }

    std::set<VertexDescriptor> channelsBetweenClusters;

    if (clusterMap.size() > 1) {
      for (size_t srcCluster = 0; srcCluster < clusterMap.size(); ++srcCluster) {
        ClusterInformation &ci = clusterMap[srcCluster];
        ClusterOptions     &clusterOptions = ci.clusterOptions;

        std::priority_queue<size_t> outputOutDegrees;
        std::priority_queue<size_t> outputMulticastDegrees;

        for (size_t i = ci.vdOutputs.size(); i > 0; --i) {
          {
            double outputOutDegree = clusterOptions.outputOutDegree();
            if (outputOutDegree > 0)
              outputOutDegrees.push(outputOutDegree);
          }
          {
            double outputMulticastDegree = clusterOptions.outputMulticastDegree();
            if (outputMulticastDegree > 1)
              outputMulticastDegrees.push(outputMulticastDegree);
          }
        }
        for (VertexDescriptor const vdSrc : ci.vdOutputs) {
          size_t outputOutDegree = outputOutDegrees.empty()
              ? 0 : outputOutDegrees.top();
          if (!outputOutDegrees.empty())
            outputOutDegrees.pop();
          int outputMulticastDegree = outputMulticastDegrees.empty()
              ? 1 : outputMulticastDegrees.top();
          if (!outputMulticastDegrees.empty())
            outputMulticastDegrees.pop();
//        std::cout
//            << "outputOutDegree: " << outputOutDegree
//            << ", outputMulticastDegree: " << outputMulticastDegree << std::endl;
          VertexMap vdSnks;
          for (size_t snkCluster = (createDAG ? srcCluster + 1 : 0);
               snkCluster < clusterMap.size();
               ++snkCluster) {
            if (srcCluster == snkCluster)
              continue;
            vdSnks.insert(vdSnks.end()
              , clusterMap[snkCluster].vdInputs.begin()
              , clusterMap[snkCluster].vdInputs.end());
          }
          while (vdSnks.size() < outputOutDegree) {
            size_t oldSnks = vdSnks.size();
            for (size_t snkCluster = (createDAG ? srcCluster + 1 : 0);
                 snkCluster < clusterMap.size();
                 ++snkCluster) {
              if (srcCluster == snkCluster)
                continue;
              if (vdSnks.size() >= outputOutDegree)
                break;
              VertexMap vdCandidates;
              for (VertexDescriptor vdCandidate : clusterMap[snkCluster].vdActors) {
                if(std::find(vdSnks.begin(), vdSnks.end(), vdCandidate) == vdSnks.end())
                  vdCandidates.push_back(vdCandidate);
              }
              if (!vdCandidates.empty())
                vdSnks.push_back(vdCandidates
                    [boost::random::uniform_int_distribution<>(0, vdCandidates.size()-1)(randomSourceTopology)]);
            }
            if (oldSnks == vdSnks.size())
              break;
          }
          while (!vdSnks.empty() && outputOutDegree > 0) {
            std::vector<VertexDescriptor> vdMulticast;
            do {
              size_t snkIndex = boost::random::uniform_int_distribution<>(0, vdSnks.size()-1)(randomSourceTopology);
              vdMulticast.push_back(vdSnks[snkIndex]);
              vdSnks.erase(vdSnks.begin() + snkIndex);
              --outputMulticastDegree;
              --outputOutDegree;
            } while (!vdSnks.empty() && outputMulticastDegree > 0 && outputOutDegree > 0);
            addChannel(g, prefix, vdSrc, vdMulticast);
          }
          if (outputOutDegree)
            std::cerr << "Warning: could not create " << outputOutDegree << " output edges for output actor " << g[vdSrc].name << std::endl;
        }
      }
    }
    // First we ensure that the graph is connected
    {
      CoSupport::boost::undirect_graph<Graph> ug(g);
      
      size_t compStor[num_vertices(g)];
      boost::iterator_property_map<size_t *, VertexIndexMap>
             compMap(compStor, vertexIndexMap);
      size_t compNum = ::boost::connected_components(ug, compMap);
      if (compNum > 1) {
        std::vector<std::vector<VertexDescriptor> > components;
        components.resize(compNum);
        for (VertexIteratorPair vip = vertices(g);
             vip.first != vip.second;
             ++vip.first)
          if (g[*vip.first].type == VertexInfo::ACTOR)
            components[compMap[*vip.first]].push_back(*vip.first);
        while (components.size() > 1) {
          boost::random::uniform_int_distribution<> dist(0, components.size()-1);
          size_t srcComponent, snkComponent;
          srcComponent = dist(randomSourceTopology);
          do { snkComponent = dist(randomSourceTopology); } while (snkComponent == srcComponent);
          addChannel(g, prefix,
            components[srcComponent][boost::random::uniform_int_distribution<>
              (0, components[srcComponent].size()-1)(randomSourceTopology)],
            components[snkComponent][boost::random::uniform_int_distribution<>
              (0, components[snkComponent].size()-1)(randomSourceTopology)]);
          components[srcComponent].insert(components[srcComponent].end(), 
              components[snkComponent].begin(), components[snkComponent].end());
          components.erase(components.begin()+snkComponent);
        }
      }
    }
    {
      bool sourceActorRequested = vm["source-actor"].as<bool>();
      VertexDescriptor vdSource;
      bool inputDataRequested = vm.count("graph-input-communication");
      bool outputDataRequested = vm.count("graph-output-communication");

      VertexMap actorMap;

      // Then, we add a source actor if requested.
      if (sourceActorRequested) {
        vdSource = add_vertex(g);
        VertexInfo &viSource = g[vdSource];
        viSource.type = VertexInfo::ACTOR;
        viSource.name = prefix + "source";
        viSource.actor.repCount = 1;
      }
      bool inputsConnected = false;
      bool outputsConnected = false;
      for (VertexIteratorPair vip = vertices(g);
           vip.first != vip.second;
           ++vip.first) {
        if (vdSource == *vip.first)
          continue; // Skip added source vertex
        if (g[*vip.first].type != VertexInfo::ACTOR)
          continue; // Skip all channels

        actorMap.push_back(*vip.first);

        bool isSourceActor = !in_degree(*vip.first, g);
        bool isSinkActor   = !out_degree(*vip.first, g);
        
        inputsConnected  |= isSourceActor;
        outputsConnected |= isSinkActor;
        
        if (isSourceActor && sourceActorRequested)
          addChannel(g, prefix, vdSource, *vip.first);
        if (isSourceActor && inputDataRequested)
          addInput(g, prefix, *vip.first);
        
        if (isSinkActor && outputDataRequested)
          addOutput(g, prefix, *vip.first);
      }
      if (!inputsConnected && (sourceActorRequested || inputDataRequested)) {
        boost::random::uniform_int_distribution<> dist(0, actorMap.size()-1);
        VertexDescriptor vdActor = actorMap[dist(randomSourceTopology)];
        if (sourceActorRequested)
          addChannel(g, prefix, vdSource, vdActor);
        if (inputDataRequested)
          addInput(g, prefix, vdActor);
      }
      if (!outputsConnected && outputDataRequested) {
        boost::random::uniform_int_distribution<> dist(0, actorMap.size()-1);
        VertexDescriptor vdActor = actorMap[dist(randomSourceTopology)];
        if (outputDataRequested)
          addOutput(g, prefix, vdActor);
      }
    }

    // Next, we insert sufficient initial tokens to prevent deadlock
    {
      typedef NGA::DropChannelVertices<Graph> SDF;
      SDF gSDF(g, [&g] (VertexDescriptor vd)
          { return g[vd].type != VertexInfo::ACTOR; });

      SDF::EdgeConsMap<EdgeTokensMap>::type  edgeConsMap(edgeTokensMap);
      SDF::EdgeProdMap<EdgeTokensMap>::type  edgeProdMap(g, edgeTokensMap);
      SDF::EdgeNameMap<VertexNameMap>::type  edgeNameMap(g, vertexNameMap);
      SDF::EdgeDelayMap<FIFODelayMap>::type  edgeDelayMap(g, fifoDelayMap);
      SDF::EdgeCapMap<FIFOCapacityMap>::type edgeCapMap(g, fifoCapacityMap);

      typedef std::vector<size_t> VertexRepLeftStorage;
      VertexRepLeftStorage vertexRepLeftStorage(num_vertices(gSDF));
      boost::container_property_map<SDF, VertexDescriptor, VertexRepLeftStorage>
        vertexRepLeftMap(vertexRepLeftStorage, gSDF);

      for (std::pair<
               SDF::vertex_iterator
             , SDF::vertex_iterator> vip = vertices(gSDF);
           vip.first != vip.second;
           ++vip.first)
        vertexRepLeftMap[*vip.first] = actorRepCountMap[*vip.first];

      std::map<SDF::edge_descriptor, size_t> edgeTokensStorage;
      boost::associative_property_map<std::map<SDF::edge_descriptor, size_t> > edgeTokensMap(edgeTokensStorage);

      do {
        std::set<SDF::vertex_descriptor> readyActors;
        bool firingsLeft;
        boost::tie(readyActors, firingsLeft) =
          NGA::SDF::getReadyActorsSet(
            gSDF,
            edgeConsMap,
            edgeProdMap,
            edgeTokensMap,
            edgeCapMap,
            //boost::static_property_map<size_t>(-1), // EdgeChannelSizeMap is constant -1 => unlimited channel capacity
            vertexRepLeftMap
          );
        if (!firingsLeft)
          break;
        if (readyActors.empty()) {
          std::vector<SDF::vertex_descriptor> indexToVertexMap;
          
          for (std::pair<
                   SDF::vertex_iterator
                 , SDF::vertex_iterator> vip = vertices(gSDF);
               vip.first != vip.second;
               ++vip.first)
            if (vertexRepLeftMap[*vip.first])
              indexToVertexMap.push_back(*vip.first);
          assert(!indexToVertexMap.empty());
          
          boost::random::uniform_int_distribution<> dist(0, indexToVertexMap.size()-1);
          SDF::vertex_descriptor randomActor = indexToVertexMap[dist(randomSource)];
//        std::cout << "Forcing actor " << vertexNameMap[randomActor] << " ready!";
          NGA::SDF::forceReadyActor(
            gSDF,
            randomActor,
            edgeConsMap,
            edgeProdMap,
            edgeTokensMap,
            edgeCapMap);
        } else
          NGA::SDF::fireReadyActorsSet(
            gSDF,
            readyActors,
            edgeConsMap,
            edgeProdMap,
            edgeTokensMap,
            edgeCapMap,
            vertexRepLeftMap);
      } while (true);

      for (std::pair<
               SDF::edge_iterator
             , SDF::edge_iterator> eip = edges(gSDF);
           eip.first != eip.second;
           ++eip.first) {
        size_t &delay = get(edgeDelayMap, *eip.first);
        delay = std::max(delay, get(edgeTokensMap, *eip.first));
      }
    }
    checkGraphDeadlockFree(g);
    // Add additional initial tokens as specified by option
    if (vm.count("extra-delay-factor")) {
      RandomGenerator<double> extraDelayFactor = vm["extra-delay-factor"].as<RandomGenerator<double> >();

      for (std::pair<
               Graph::vertex_iterator
             , Graph::vertex_iterator> vip = vertices(g);
           vip.first != vip.second;
           ++vip.first) {
        if (g[*vip.first].type != VertexInfo::FIFO)
          continue;
        assert(in_degree(*vip.first, g) == 1);
        Graph::edge_descriptor edProd = *in_edges(*vip.first, g).first;
        size_t additionalInitialTokens =
            get(edgeTokensMap, edProd) * extraDelayFactor();
        g[*vip.first].fifo.delay    += additionalInitialTokens;
        g[*vip.first].fifo.capacity += additionalInitialTokens;
      }
      checkGraphDeadlockFree(g);
    }
    // Add additional initial tokens at edges between clusters as specified by option
    if (vm.count("extra-delay-factor-inter-cluster")) {
      RandomGenerator<double> extraDelayFactor = vm["extra-delay-factor-inter-cluster"].as<RandomGenerator<double> >();
      for (Graph::vertex_descriptor vd : channelsBetweenClusters) {
        assert(g[vd].type == VertexInfo::FIFO);
        assert(in_degree(vd, g) == 1);
        Graph::edge_descriptor edProd = *in_edges(vd, g).first;
        size_t additionalInitialTokens =
            get(edgeTokensMap, edProd) * extraDelayFactor();
        g[vd].fifo.delay    += additionalInitialTokens;
        g[vd].fifo.capacity += additionalInitialTokens;
      }
      checkGraphDeadlockFree(g);
    }

    // Set the communication size for the graph
    {
      typedef std::vector<double> ChanComScalingStorage;
      ChanComScalingStorage chanComScalingStorage(num_vertices(g));
      boost::container_property_map<Graph, VertexDescriptor, ChanComScalingStorage>
        chanComScalingMap(chanComScalingStorage, g);

      RandomGenerator<double> graphCommScaling = vm["graph-communication-scaling"].as<RandomGenerator<double> >();
      size_t internalComm = 0;
      if (vm.count("graph-internal-communication")) {
        RandomGenerator<size_t> graphInternalComm = vm["graph-internal-communication"].as<RandomGenerator<size_t> >();
        internalComm = graphInternalComm();
      }
      size_t inputComm = 0;
      if (vm.count("graph-input-communication")) {
        RandomGenerator<size_t> graphInputComm = vm["graph-input-communication"].as<RandomGenerator<size_t> >();
        inputComm = graphInputComm();
      }
      size_t outputComm = 0;
      if (vm.count("graph-output-communication")) {
        RandomGenerator<size_t> graphOutputComm = vm["graph-output-communication"].as<RandomGenerator<size_t> >();
        outputComm = graphOutputComm();
      }

      double singlecastCommScalingSum = 0;
      double inputCommScalingSum = 0;
      double outputCommScalingSum = 0;
      double multicastCommScalingSum = 0;
      double multicastAmplification = 0;
      for (std::pair<
               Graph::vertex_iterator
             , Graph::vertex_iterator> vip = vertices(g);
           vip.first != vip.second;
           ++vip.first)
        switch (g[*vip.first].type) {
          case VertexInfo::FIFO: {
            double commScaling = std::max(0.0, graphCommScaling());
            chanComScalingMap[*vip.first] = commScaling;
            size_t outputs = out_degree(*vip.first, g);
            assert(outputs >= 1);
            if (outputs > 1) {
              multicastCommScalingSum += commScaling;
              multicastAmplification  += commScaling * outputs;
            } else
              singlecastCommScalingSum += commScaling;
            break;
          }
          case VertexInfo::REGISTER: {
            double commScaling = std::max(0.0, graphCommScaling());
            chanComScalingMap[*vip.first] = commScaling;
            if (in_degree(*vip.first, g) == 0)
              inputCommScalingSum += commScaling * out_degree(*vip.first, g);
            else {
              assert(out_degree(*vip.first, g) == 0);
              outputCommScalingSum += commScaling;
            }
            break;
          }
          default:
            // Ignore actors
            assert(g[*vip.first].type == VertexInfo::ACTOR);
            break;
        }

      size_t multicastWrite;
      size_t singlecastComm;

      if (vm.count("graph-internal-read-ratio")) {
        double readRatio = vm["graph-internal-read-ratio"].as<RandomGenerator<double> >()();


        // Derived from readComm + writeComm == internalComm
        size_t writeComm = internalComm / (1+readRatio);
        // This is writeComm * readRatio;
        size_t readComm  = internalComm * readRatio / (1+readRatio);

  //    std::cout << "writeComm:    " << writeComm << std::endl;
  //    std::cout << "readComm:     " << readComm << std::endl;
  //    std::cout << "internalComm: " << (readComm+writeComm) << " (" << internalComm << ")" << std::endl;

        // Here, x denotes the factor by which multicast data is amplified,
        // i.e., multicastRead = x * multicastWrite
        double x = multicastAmplification / multicastCommScalingSum;
  //    std::cout << "x: " << x << std::endl;
        // Derived from
        //   multicastRead = x * multicastWrite
        //   readComm  = singlecastComm + multicastRead;
        //   writeComm = singlecastComm + multicastWrite
        multicastWrite = (readComm-writeComm) / (x-1);
        singlecastComm = writeComm - multicastWrite;

  //    std::cout << "multicastWrite: " << multicastWrite << std::endl;
  //    std::cout << "singlecastComm: " << singlecastComm << std::endl;
      } else {
        multicastWrite = internalComm;
        singlecastComm = internalComm;
        singlecastCommScalingSum = 2*singlecastCommScalingSum + multicastCommScalingSum + multicastAmplification;
        multicastCommScalingSum  = singlecastCommScalingSum;
      }

      for (std::pair<
               Graph::vertex_iterator
             , Graph::vertex_iterator> vip = vertices(g);
           vip.first != vip.second;
           ++vip.first)
        switch (g[*vip.first].type) {
          case VertexInfo::FIFO: {
            assert(in_degree(*vip.first, g) == 1);
            Graph::edge_descriptor   edProd = *in_edges(*vip.first, g).first;
            Graph::vertex_descriptor vdProd = source(edProd, g);
            size_t tokensTransmission =
                get(edgeTokensMap, edProd)
              * get(actorRepCountMap, vdProd);
            size_t outputs = out_degree(*vip.first, g);
            assert(outputs >= 1);
            if (outputs > 1) {
              double normFactor = chanComScalingMap[*vip.first]/multicastCommScalingSum;
              put(channelTSizeMap, *vip.first, (normFactor*multicastWrite)/tokensTransmission);
            } else {
              double normFactor = chanComScalingMap[*vip.first]/singlecastCommScalingSum;
              put(channelTSizeMap, *vip.first, (normFactor*singlecastComm)/tokensTransmission);
            }
            break;
          }
          case VertexInfo::REGISTER: {
            if (in_degree(*vip.first, g) == 0) {
              double normFactor = chanComScalingMap[*vip.first]/inputCommScalingSum;
              Graph::edge_descriptor   edCons = *out_edges(*vip.first, g).first;
              Graph::vertex_descriptor vdCons = target(edCons, g);
              size_t tokensTransmission =
                  get(edgeTokensMap, edCons)
                * get(actorRepCountMap, vdCons);
//            std::cout << "nf: " << normFactor << " ic: " << inputComm << " tt: " << tokensTransmission << std::endl;
              put(channelTSizeMap, *vip.first, (normFactor*inputComm)/tokensTransmission);
            } else {
              assert(out_degree(*vip.first, g) == 0);
              double normFactor = chanComScalingMap[*vip.first]/outputCommScalingSum;
              Graph::edge_descriptor   edProd = *in_edges(*vip.first, g).first;
              Graph::vertex_descriptor vdProd = source(edProd, g);
              size_t tokensTransmission =
                  get(edgeTokensMap, edProd)
                * get(actorRepCountMap, vdProd);
//            std::cout << "nf: " << normFactor << " oc: " << outputComm << " tt: " << tokensTransmission << std::endl;
              put(channelTSizeMap, *vip.first, (normFactor*outputComm)/tokensTransmission);
            }
            break;
          }
          default:
            // Ignore actors
            assert(g[*vip.first].type == VertexInfo::ACTOR);
            break;
        }
    }

    if (vm.count("dot")) {
      CoSupport::Streams::AOStream out(std::cout, vm["dot"].as<std::string>(), "-");
      VertexPropertyWriter vpw(g);
      EdgePropertyWriter   epw(g);
      boost::write_graphviz(out, g, vpw, epw);
    }

    if (1) {
      size_t internalBytesRead    = 0;
      size_t internalBytesWritten = 0;
      size_t inputBytesRead       = 0;
      size_t outputBytesWritten   = 0;

      for (std::pair<
               Graph::vertex_iterator
             , Graph::vertex_iterator> vip = vertices(g);
           vip.first != vip.second;
           ++vip.first) {
        if (g[*vip.first].type == VertexInfo::ACTOR)
          continue;

        bool isInput = in_degree(*vip.first, g) == 0;
        for (std::pair<
                 Graph::out_edge_iterator
               , Graph::out_edge_iterator> eip = out_edges(*vip.first, g);
             eip.first != eip.second;
             ++eip.first)
        {
          Graph::edge_descriptor   edCons = *eip.first;
          Graph::vertex_descriptor vdCons = target(*eip.first, g);
          size_t cons      = get(edgeTokensMap, edCons);
          size_t repCount  = get(actorRepCountMap, vdCons);
          size_t tokenSize = get(channelTSizeMap, *vip.first);
          if (isInput)
            inputBytesRead += cons * repCount * tokenSize;
          else
            internalBytesRead += cons * repCount * tokenSize;
        }

        if (isInput) {
          assert(g[*vip.first].type == VertexInfo::REGISTER);
          assert(out_degree(*vip.first, g) > 0);
        } else {
          assert(in_degree(*vip.first, g) == 1);
          Graph::edge_descriptor   edProd = *in_edges(*vip.first, g).first;
          Graph::vertex_descriptor vdProd = source(edProd, g);
          size_t prod      = get(edgeTokensMap, edProd);
          size_t repCount  = get(actorRepCountMap, vdProd);
          size_t tokenSize = get(channelTSizeMap, *vip.first);
          if (out_degree(*vip.first, g) == 0) {
            assert(g[*vip.first].type == VertexInfo::REGISTER);
            outputBytesWritten += prod * repCount * tokenSize;
          } else {
            assert(g[*vip.first].type == VertexInfo::FIFO);
            internalBytesWritten += prod * repCount * tokenSize;
          }
        }
      }
      std::cout << "internalBytesRead:    " << internalBytesRead << std::endl;
      std::cout << "internalBytesWritten: " << internalBytesWritten << std::endl;
      std::cout << "internalBytesTotal:   " << (internalBytesRead+internalBytesWritten) << std::endl;
      std::cout << "internalReadRation:   " << (1.0*internalBytesRead) / internalBytesWritten << std::endl;
      std::cout << "inputBytesRead:       " << inputBytesRead << std::endl;
      std::cout << "outputBytesWritten:   " << outputBytesWritten << std::endl;
    }
    if (vm.count("sng")) {
      CoSupport::Streams::AOStream out(std::cout, vm["sng"].as<std::string>(), "-");
      saveSNG(g, out);
    }
#if 0 && defined(SYSTEMOC_ENABLE_SGX)
    if (vm.count("ngx")) {
      CoSupport::Streams::AOStream out(std::cout, vm["ngx"].as<std::string>(), "-");
      saveNGX(g, out);
    }
#endif //SYSTEMOC_ENABLE_SGX
  } catch (std::exception &e) {
    std::cerr << prgname << ": " << e.what() << std::endl;
    return 1;
  }
  return 0;
}
