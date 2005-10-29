#include <hscd_tdsim_TraceLog.hpp>

#ifdef SYSTEMOC_TRACE

TraceLogStream TraceLog("test.trace");


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
void TraceLogStream::traceCommExecIn(size_t size, const char *name){
  stream << "<commexecin size=\""<<size<<"\" name= \""<<name<<"\"/>" << std::endl;
}
void TraceLogStream::traceCommExecOut(size_t size, const char *name){
  stream << "<commexecout size=\""<<size<<"\" name= \""<<name<<"/>" << std::endl;
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

  stream << "<?xml version=\"1.0\"?>" << std::endl;
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
}

#endif
