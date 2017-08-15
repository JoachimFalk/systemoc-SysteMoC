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

#include <CoSupport/compatibility-glue/nullptr.h>
#include <smoc/detail/NodeBase.hpp>
#include <time.h>
#include <sstream>
#include <cassert>
#include <cmath>
#include <systemc>

#include <systemoc/smoc_config.h>

#include <smoc/detail/TraceLog.hpp>
#include <smoc/detail/ChanBase.hpp>
#include <smoc/detail/PortBase.hpp>

#include <systemoc/detail/smoc_chan_if.hpp>
#include <systemoc/detail/smoc_port_registry.hpp>
#include <systemoc/detail/smoc_func_call.hpp>

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE

#define READABLE(e) do {} while(0)
//#define READABLE(e) e

namespace smoc { namespace Detail {

using std::string;

// class for generating caption indices for fifos
class Sequence
{
public:
  // constructs object for generating "count" indices,
  // with each place ranging from "start" to "end"
  Sequence(unsigned int count, char start, char end);
  // returns current sequence
  const string& current() const;
  // resets to starting sequence
  void reset();
  // advances to next sequence, returns true if successful
  bool next();
private:
  unsigned int cnt;
  unsigned int gen;
  char start;
  char end;
  string seq;         
};

/**
 * Sequence definitions
 */

Sequence::Sequence(unsigned int count, char start, char end) :
  cnt(count),
  gen(0),
  start(start),
  end(end)
{
  assert(start < end);
  reset();
}

const string& Sequence::current() const {
  assert(gen > 0);
  return seq;
}

void Sequence::reset() {
  gen = 0;
}

bool Sequence::next() {
  bool ok = false;
  
  if(gen == 0 && cnt > 0) {
    int num = 1;  
    // If more than one index should be generated, needed digits must
    // be calculated
    if(cnt > 1) {
      double base = (double)(end - start + 1);
      num = (int)std::floor(std::log((double)(cnt-1))/std::log(base)) + 1;
    }
    seq.assign(num, start);
    ok = true;
  }
  else if(gen < cnt) {
    int pos = seq.size() - 1;
    while(seq[pos] == end) {
      seq[pos] = start;
      --pos;
    }
    ++seq[pos];
    ok = true;
  }
  
  if(ok) ++gen;
  return ok;
}

TraceLogStream::TraceLogStream(std::ostream *st) : stream(*st) {
  stream.insert(indenter);
}

void TraceLogStream::init(){
  stream << "<?xml version=\"1.0\"?>\n<systemoc_trace>" << std::endl;
  stream << CoSupport::Streams::Indent::Up;
}



void TraceLogStream::traceStartActor(const smoc::Detail::NamedIdedObj *actor,
                                     const char *mode) {

  if( !getSimCTX()->isDataflowTracingEnabled() )
    return;

  lastactor=actor->name();
  size_t id = namePool.registerId(lastactor, actor->getId());
  stream << "<a n=\"" << id << "\" m=\"" << mode << "\" t=\""
         << sc_time_stamp() << "\">";
  READABLE(stream << " <i n=\"" << lastactor << "\">");
  stream << std::endl;
  actors.insert(lastactor);
  actor_activation_count[lastactor]++;
  stream << CoSupport::Streams::Indent::Up;
}

void TraceLogStream::traceEndActor(const smoc::Detail::NamedIdedObj *actor){

  if( !getSimCTX()->isDataflowTracingEnabled() )
    return;
  lastactor="";

  stream << CoSupport::Streams::Indent::Down;
  stream << "</a>" << std::endl;
}

void TraceLogStream::traceStartFunction(const smoc_func_call *func){

  if( !getSimCTX()->isDataflowTracingEnabled() )
    return;

  const char * name = func->getFuncName();
  size_t id = namePool.registerId(name, func->getId());

  stream << "<f n=\""<< id << "\" t=\"" << sc_time_stamp() << "\">";
  READABLE(stream << " <i n=\"" << name << "\">");
  stream << std::endl;
  function_call_count[string(lastactor)+" -> "+string(name)]++;
  functions[lastactor].insert(name);
  last_actor_function[lastactor] = name;
  stream << CoSupport::Streams::Indent::Up;
}

void TraceLogStream::traceEndFunction(const smoc_func_call *func){

  if( !getSimCTX()->isDataflowTracingEnabled() )
    return;

  stream << CoSupport::Streams::Indent::Down;
  stream << "</f>" << std::endl;
}

void TraceLogStream::traceCommExecIn(const smoc_root_chan *chan, size_t size) {

  if( !getSimCTX()->isDataflowTracingEnabled() )
    return;

  const char *actor = chan->name();
  
  size_t id = namePool.registerId(actor, chan->getId());
  
  stream << "<i s=\"" << size << "\" c=\"" << id
         << "\" t=\"" << sc_time_stamp() << "\"/>";
  READABLE(stream << " <i n=\"" << actor << "\">");
  stream << std::endl;
  fifo_info[actor].size -= size;
  if(lastactor != "") {
    fifo_info[actor].to.name = lastactor;
    fifo_info[actor].to.unknown = false;
  }
}

void TraceLogStream::traceCommExecOut(const smoc_root_chan *chan, size_t size) {

  if( !getSimCTX()->isDataflowTracingEnabled() )
    return;

  const char *actor = chan->name();
  
  size_t id = namePool.registerId(actor, chan->getId());

  stream << "<o s=\"" << size << "\" c=\"" << id
         << "\" t=\"" << sc_time_stamp() << "\"/>";
  READABLE(stream << " <i n=\"" << actor << "\">");
  stream << std::endl;
  fifo_info[actor].size += size;
  if(lastactor != "") {
    fifo_info[actor].from.name = lastactor;
    fifo_info[actor].from.unknown = false;
  }
}

void TraceLogStream::traceCommSetup(const smoc_root_chan *chan, size_t req) {

  if( !getSimCTX()->isDataflowTracingEnabled() )
    return;

  const char *fifo = chan->name();
  
  size_t id = namePool.registerId(fifo, chan->getId());
  stream << "<r c=\"" << id << "\" s=\"" << req << "\"/>";
  READABLE(stream << " <i n=\"" << fifo << "\">");
  stream << std::endl;
}

void TraceLogStream::traceTransition(size_t id) {

  if( !getSimCTX()->isDataflowTracingEnabled() )
    return;

  stream << "<t id=\"" << id << "\"/>" << std::endl;
}

void printConnectedActors(const smoc_root_chan *chan, CoSupport::Streams::FilterOStream& stream){
  const smoc_port_registry::EntryMap& entries = chan->getEntries();
  for (smoc_port_registry::EntryMap::const_iterator iter = entries.begin();
      iter != entries.end();
      ++iter ) {
    smoc_sysc_port const *p  = dynamic_cast<smoc_sysc_port *>(iter->second);
    sc_port_base   const *ap = p != nullptr ? p->getActorPort() : iter->second;
    stream << "<source actor=\""
        << ap->get_parent()->name()
        << "\"/>" << std::endl;
  }

  const smoc_port_registry::OutletMap& outlets= chan->getOutlets();
  for (smoc_port_registry::OutletMap::const_iterator iter = outlets.begin();
      iter != outlets.end();
      ++iter ) {
    smoc_sysc_port const *p  = dynamic_cast<smoc_sysc_port *>(iter->second);
    sc_port_base   const *ap = p != nullptr ? p->getActorPort() : iter->second;
    stream << "<sink actor=\""
        << ap->get_parent()->name()
        << "\"/>" << std::endl;
  }
}

void TraceLogStream::traceInitialTokens(const smoc_root_chan *chan,
                                        size_t size,
                                        size_t capacity) {

  if( !getSimCTX()->isDataflowTracingEnabled() )
    return;

  stream << "<it cap=\"" << capacity << "\">" << std::endl;
  stream << CoSupport::Streams::Indent::Up;
  this->traceCommExecOut(chan, size);
  printConnectedActors(chan, stream);
  stream << CoSupport::Streams::Indent::Down;
  stream << "</it>" << std::endl;
}

TraceLogStream::~TraceLogStream(){

  if( !getSimCTX()->isDataflowTracingEnabled() )
    return;

  stream << CoSupport::Streams::Indent::Down;

  const NamePool::NameMap &names = namePool.getMap();

  for(NamePool::NameMap::const_iterator iter = names.begin();
      iter != names.end();
      ++iter){
    stream << "<name id=\"" << iter->first << "\" name=\"" << iter->second
           << "\"/> " << std::endl;
  }


  stream << "<!--" << std::endl;

  stream << "function            #" << std::endl;
  for(std::map<string, int>::const_iterator i = function_call_count.begin();
      i != function_call_count.end();
      i++){
    stream << i->first << "\t\t" << i->second << std::endl;
  }

  stream << "\nactor              #" << std::endl;
  for(std::map<string, int>::const_iterator i = actor_activation_count.begin();
      i != actor_activation_count.end();
      i++){
    stream << i->first << "\t\t" << i->second << std::endl;
  }
  
  stream << "\nlast actor function" << std::endl;
  for(std::map<string, string>::const_iterator i = last_actor_function.begin();
      i != last_actor_function.end();
      i++){
    stream << i->first << "\t\t" << i->second << std::endl;
  }
  
  stream << "\nfifo info" << std::endl;
  for(std::map<string, s_fifo_info>::const_iterator i = fifo_info.begin();
      i != fifo_info.end();
      i++){
    stream << i->first << "\t\t" << i->second.size << "\t"
           << (i->second.from.unknown ? "?" : i->second.from.name) << " -> "
           << (i->second.to.unknown ? "?" : i->second.to.name) << std::endl;
  }
  
  stream << "\n<?xml version=\"1.0\"?>" << std::endl;
  stream << "<!DOCTYPE configuration SYSTEM \"cmx.dtd\">" << std::endl;
  stream << "<configuration>" << std::endl;
  stream << " <resultfile name=\"\"/>" << std::endl;
  stream << " <measurefile name=\"\"/>\n" << std::endl;

  stream << " <resources>" << std::endl;
  stream << "  <component name=\"Component1\" type=\"threaded\" scheduler=\"RoundRobin\">" << std::endl;
  stream << "   <attribute type=\"timeslice\" value=\"20ns\"/>" << std::endl;
  stream << "  </component>" << std::endl;
  stream << " </resources>\n" << std::endl;

  stream << " <mappings>" << std::endl;




  for(std::set<string>::const_iterator i = actors.begin();
      i != actors.end();
      i++){
    stream << "  <mapping source=\"" << *i << "\" target=\"Component1\">" << std::endl;
    stream << "    <attribute type=\"delay\" value=\"\"/>" << std::endl;
    std::set<string> fSet = functions[*i];
    while( fSet.size() > 0 ){
      stream << "    <attribute type=\"" << *fSet.begin() << "\" value=\"\"/>" << std::endl;
      fSet.erase(fSet.begin());
    }
    
    stream << "  </mapping>\n" << std::endl;
  }
  
  stream << " </mappings>" << std::endl;
  stream << "</configuration>\n" << std::endl;
  stream << "-->" << std::endl;
  stream << "</systemoc_trace>" << std::endl;  
  
  createFifoGraph();
}

// "Casts" any type to string, if output operator for that type
// is defined
template<class A>
std::string string_cast(const A &a) {
  std::ostringstream os;
  os << a;
  return os.str();
}

void TraceLogStream::createFifoGraph()
{
  // Get current time (in seconds)  
  time_t now;
  if(time(&now) == (time_t)-1) {
    perror("TraceLogStream::createFifoGraph()");
    return;
  }
  
  // Format current time, append suffix
  char filename[50]; 
  if(strftime(filename, 50, "(%d.%m.%Y)%T.dot", localtime(&now)) == 0) {
    perror("TraceLogStream::createFifoGraph()");
    return;
  }
  
  // Create filename: "PREFIX(DATE)TIME.dot"
  string fstring = filename;
  char *prefix = getenv("VPCTRACEFILEPREFIX");
  if(prefix != 0) fstring.insert(0, prefix);
  std::ofstream file;
  
  // Counts actors that have no name (used to generate unique name)
  int unknown = 0;
  
  // first remove empty fifos and create unique names for
  // unknown actors
  for(std::map<string, s_fifo_info>::iterator fifo = fifo_info.begin();
      fifo != fifo_info.end();)
  {
    s_fifo_info &info = fifo->second;
    s_actor_info &from = info.from;
    s_actor_info &to = info.to;
    
    if(info.size == 0) {
      fifo_info.erase(fifo++);
      continue;
    }
    
    if(from.unknown) {
      assert(from.name == "");
      from.name = "? (";
      from.name += string_cast(unknown++);
      from.name += ")";
    }
    
    if(to.unknown) {
      assert(to.name == "");
      to.name = "? (";
      to.name += string_cast(unknown++);
      to.name += ")";
    }
    
    ++fifo;   
  }
  
  // True, if fifos were removed in loop
  bool retry = false;
  
  // now remove fifos that are not in loops (but retain unknown actors)
  do {
    retry = false;
    
    for(std::map<string, s_fifo_info>::iterator fifo = fifo_info.begin();
        fifo != fifo_info.end();)
    {
      s_fifo_info &info = fifo->second;
      s_actor_info &from = info.from;
      s_actor_info &to = info.to;
      
      if(!from.unknown) {
        // Is actor some other fifo's target?
        bool found = false;
        for(std::map<string, s_fifo_info>::iterator other = fifo_info.begin();
            other != fifo_info.end();
            ++other) {
         if(other->second.to.name == from.name) {
            found = true;
            break;
         }
        }
        // If not => fifo not in loop => remove
        if(!found) {
          retry = true;
          fifo_info.erase(fifo++);
          continue;
        }
      }
      
      if(!to.unknown) {
        // Is actor some other fifo's source?
        bool found = false;
        for(std::map<string, s_fifo_info>::iterator other = fifo_info.begin();
            other != fifo_info.end();
            ++other) {
          if(other->second.from.name == to.name) {
            found = true;
            break;
          }
        }
        // If not => fifo not in loop => remove
        if(!found) {
          retry = true;
          fifo_info.erase(fifo++);
          continue;
        }
      }
    
      ++fifo;
    }
  } while(retry);
  
  // All drawn actors, mapped to unique id 
  std::map<string, int> actors;
  int id = 0;
  
  // Fifo mappings
  std::string caption;
  Sequence index(fifo_info.size(), 'A', 'Z');
  
  // Create graph
  for(std::map<string, s_fifo_info>::iterator fifo = fifo_info.begin();
      fifo != fifo_info.end();
      ++fifo)
  {
    s_fifo_info &info = fifo->second;
    
    if(!file.is_open()) {
      file.open(fstring.c_str());
      file << "digraph G {" << std::endl;
      file << "graph[page=\"8.5,11\",size=\"7.5,10\",center=\"1\",ratio=\"fill\"];" << std::endl;
      file << "node[height=\"0.75\"];" << std::endl;
    }
    
    typedef std::map<string, int>::iterator Iter;
    typedef std::pair<Iter, bool> Retval;
    
    // NodeBase: From
    Iter from = actors.find(info.from.name);
    if(from == actors.end()) {
      Retval r = actors.insert(std::make_pair(info.from.name, id++));
      from = r.first;
      file << from->second << "[label=\"" << from->first << "\"];" << std::endl;
    }
    
    // NodeBase: To
    Iter to = actors.find(info.to.name);
    if(to == actors.end()) {
      Retval r = actors.insert(std::make_pair(info.to.name, id++));
      to = r.first;
      file << to->second << "[label=\"" << to->first << "\"];" << std::endl; 
    }

    // Generate next(first) index
    index.next();
    
    // Edge: From -> To
    file << from->second << "->" << to->second << "[label=\""
         << index.current() << "(" << info.size << ")\"];" << std::endl;
    
    // Update caption
    caption += index.current();
    caption += ": ";
    caption += fifo->first;
    caption += "\\n";
  }
  
  if(file.is_open()) {
    file << "}" << std::endl;
    file << "digraph C {" << std::endl;
    file << "graph[page=\"8.5,11\",size=\"7.5,10\",center=\"1\",ratio=\"fill\"];" << std::endl;
    file << "caption[label=\"" << caption << "\",shape=\"box\"];" << std::endl;
    file << "}";
    file.close();
    std::cerr << "Found cycle(s) and/or unknown actors. Dumped FIFO info to " << fstring << std::endl;
  }
}

}} // namespace smoc::Detail

#endif
