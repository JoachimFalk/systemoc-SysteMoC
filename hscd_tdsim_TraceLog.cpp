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

#include <time.h>
#include <sstream>
#include <cassert>
#include <cmath>
#include <systemc.h>

#include <systemoc/hscd_tdsim_TraceLog.hpp>

#ifdef SYSTEMOC_TRACE

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

TraceLogStream TraceLog("test.trace");

void  TraceLogStream::traceBlockingWaitStart(){
  stream << "<waiting type=\"sleep\" t=\"" << sc_time_stamp() << "\"/>" << std::endl;
}
void  TraceLogStream::traceBlockingWaitEnd(){
  stream << "<waiting type=\"wake up\" t=\"" << sc_time_stamp() << "\"/>" << std::endl;
}
void  TraceLogStream::traceStartChoice(const char * actor){
  stream << "<choice type=\"begin\" n=\""<< actor << "\" t=\"" << sc_time_stamp() << "\"/>" << std::endl;
  actors.insert(actor);
  actor_activation_count[actor]++;
  lastactor = actor;
  fifo_actor_last = actor;
}
void  TraceLogStream::traceEndChoice(const char * actor){
  stream << "<choice type=\"end\" n=\""<< actor << "\" t=\"" << sc_time_stamp() << "\"/>" << std::endl;
  fifo_actor_last = "";
}
void  TraceLogStream::traceStartTransact(const char * actor){
  stream << "<transact type=\"begin\" n=\""<< actor << "\" t=\"" << sc_time_stamp() << "\"/>" << std::endl;
  actors.insert(actor);
  actor_activation_count[actor]++;
  lastactor = actor;
  fifo_actor_last = actor;
}
void TraceLogStream::traceEndTransact(const char * actor){
  stream << "<transact type=\"end\" n=\""<< actor << "\" t=\"" << sc_time_stamp() << "\"/>" << std::endl;
  fifo_actor_last = "";
}

void TraceLogStream::traceCommSetup(const char * fifo, size_t req){
  size_t id = namePool.getID(fifo);
  stream << "<r s=\"" << req << "\" c=\"" << id << "\"/>" << std::endl;
}

void TraceLogStream::traceStartActor(const char * actor){
  size_t id = namePool.getID(actor);
  stream << "<a n=\"" << id << "\" t=\"" << sc_time_stamp() << "\">"
         << std::endl;
  actors.insert(actor);
  actor_activation_count[actor]++;
  lastactor=actor;
}
void TraceLogStream::traceEndActor(const char * actor){
  stream << "</a>" << std::endl;
}
void TraceLogStream::traceStartFunction(const char * func){
  size_t id = namePool.getID(func);
  stream << "<f n=\""<< id << "\" t=\"" << sc_time_stamp() << "\">"
         << std::endl;
  function_call_count[string(lastactor)+" -> "+string(func)]++;
  functions[lastactor].insert(func);
  last_actor_function[lastactor] = func;
}
void TraceLogStream::traceEndFunction(const char * func){
  stream << "</f>" << std::endl;
}
void TraceLogStream::traceStartTryExecute(const char * actor){
  stream << "<e n=\""<< namePool.getID(actor) << "\" t=\"" <<
    sc_time_stamp() << "\">" << std::endl;
}
void TraceLogStream::traceEndTryExecute(const char * actor){
  stream << "</e>" << std::endl;
}
void TraceLogStream::traceCommExecIn(size_t size, const char * actor){
  stream << "<i s=\"" << size << "\" c=\"" << namePool.getID(actor)
         << "\" t=\"" << sc_time_stamp() << "\"/>" << std::endl;
  fifo_info[actor].size -= size;
  if(fifo_actor_last != "") {
    fifo_info[actor].to.name = fifo_actor_last;
    fifo_info[actor].to.unknown = false;
  }
}
void TraceLogStream::traceCommExecOut(size_t size, const char * actor){
  stream << "<o s=\"" << size << "\" c=\"" << namePool.getID(actor)
         << "\" t=\"" << sc_time_stamp() << "\"/>" << std::endl;
  fifo_info[actor].size += size;
  if(fifo_actor_last != "") {
    fifo_info[actor].from.name = fifo_actor_last;
    fifo_info[actor].from.unknown = false;
  }
}
void TraceLogStream::traceStartDeferredCommunication(const char * actor){
  stream << "<d a=\""<< namePool.getID(actor) << "\" t=\"" << sc_time_stamp()
         << "\">" << std::endl;
  fifo_actor_last = actor;
}
void TraceLogStream::traceEndDeferredCommunication(const char * actor){
  stream << "</d>" << std::endl;
  fifo_actor_last = "";
}

TraceLogStream::~TraceLogStream(){
  const NameMap &names = namePool.getMap();

  for(NameMap::const_iterator iter = names.begin();
      iter != names.end();
      ++iter){
    stream << "<name id=\"" << iter->second << "\" name=\"" << iter->first
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
    
    // Node: From
    Iter from = actors.find(info.from.name);
    if(from == actors.end()) {
      Retval r = actors.insert(std::make_pair(info.from.name, id++));
      from = r.first;
      file << from->second << "[label=\"" << from->first << "\"];" << std::endl;
    }
    
    // Node: To
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

#endif
