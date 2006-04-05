// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <hscd_tdsim_TraceLog.hpp>

#ifdef SYSTEMOC_TRACE

TraceLogStream TraceLog("test.trace");

void  TraceLogStream::traceBlockingWaitStart(){
  stream << "<waiting type=\"sleep\"/>" << std::endl;
}
void  TraceLogStream::traceBlockingWaitEnd(){
  stream << "<waiting type=\"wake up\"/>" << std::endl;
}
void  TraceLogStream::traceStartChoice(const char * actor){
  stream << "<choice type=\"begin\" name=\""<< actor << "\"/>" << std::endl;
  actors.insert(actor);
  actor_activation_count[actor]++;
  lastactor = actor;
  fifo_actor_last = actor;
}
void  TraceLogStream::traceEndChoice(const char * actor){
  stream << "<choice type=\"end\" name=\""<< actor << "\"/>" << std::endl;
  fifo_actor_last = "";
}
void  TraceLogStream::traceStartTransact(const char * actor){
  stream << "<transact type=\"begin\" name=\""<< actor << "\"/>" << std::endl;
  actors.insert(actor);
  actor_activation_count[actor]++;
  lastactor = actor;
  fifo_actor_last = actor;
}
void  TraceLogStream::traceEndTransact(const char * actor){
  stream << "<transact type=\"end\" name=\""<< actor << "\"/>" << std::endl;
  fifo_actor_last = "";
}


void TraceLogStream::traceStartActor(const char * actor){
  stream << "<actor name=\""<< actor << "\">" << std::endl;
  actors.insert(actor);
  actor_activation_count[actor]++;
  lastactor=actor;
}
void TraceLogStream::traceEndActor(const char * actor){
  stream << "</actor>" << std::endl;
}
void TraceLogStream::traceStartFunction(const char * func){
  stream << "<function name=\""<< func << "\">" << std::endl;
  function_call_count[string(lastactor)+" -> "+string(func)]++;
  functions[lastactor].insert(func);
  last_actor_function[lastactor] = func;
}
void TraceLogStream::traceEndFunction(const char * func){
  stream << "</function>" << std::endl;
}
void TraceLogStream::traceStartTryExecute(const char * actor){
  stream << "<tryexecute name=\""<< actor << "\">" << std::endl;
}
void TraceLogStream::traceEndTryExecute(const char * actor){
  stream << "</tryexecute>" << std::endl;
}
void TraceLogStream::traceCommExecIn(size_t size, const char * actor){
  stream << "<commexecin size=\""<<size<<"\" channel=\""<<actor<<"\"/>" << std::endl;
  fifo_fill_state[actor] -= size;
  if(fifo_actor_last != "")
    fifo_actor[actor].second = fifo_actor_last;
}
void TraceLogStream::traceCommExecOut(size_t size, const char * actor){
  stream << "<commexecout size=\""<<size<<"\" channel=\""<<actor<<"\"/>" << std::endl;
  fifo_fill_state[actor] += size;
  if(fifo_actor_last != "")
    fifo_actor[actor].first = fifo_actor_last;
}
void TraceLogStream::traceStartDeferredCommunication(const char * actor){
  stream << "<deferred_communication actor=\""<< actor << "\">" << std::endl;
  fifo_actor_last = actor;
}
void TraceLogStream::traceEndDeferredCommunication(const char * actor){
  stream << "</deferred_communication>" << std::endl;
  fifo_actor_last = "";
}

TraceLogStream::~TraceLogStream(){
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

  stream << "\nfifo              #" << std::endl;
  for(std::map<string, int>::const_iterator i = fifo_fill_state.begin();
      i != fifo_fill_state.end();
      i++){
    stream << i->first << "\t\t" << i->second << std::endl;
  }

  stream << "\nlast actor function" << std::endl;
  for(std::map<string, string>::const_iterator i = last_actor_function.begin();
      i != last_actor_function.end();
      i++){
    stream << i->first << "\t\t" << i->second << std::endl;
  }

  stream << "\nfifo <-> actor" << std::endl;
  for(std::map<string, std::pair<string, string> >::const_iterator i = fifo_actor.begin();
      i != fifo_actor.end();
      i++){
    stream << i->first << "\t\t"
           << (i->second.first == "" ? "?" : i->second.first) << " -> "
	   << (i->second.second == "" ? "?" : i->second.second) << std::endl;
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
}

#endif
