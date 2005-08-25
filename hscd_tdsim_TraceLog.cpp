#include <hscd_tdsim_TraceLog.hpp>

TraceLogStream TraceLog("test.trace");


void TraceLogStream::traceStartActor(const char * actor){
  stream << "<actor name=\""<< actor << "\">" << std::endl;
}
void TraceLogStream::traceEndActor(const char * actor){
  stream << "</actor>" << std::endl;
}
void TraceLogStream::traceStartFunction(const char * actor){
  stream << "<function name=\""<< actor << "\">" << std::endl;
}
void TraceLogStream::traceEndFunction(const char * actor){
  stream << "</function>" << std::endl;
}
void TraceLogStream::traceStartTryExecute(const char * actor){
  stream << "<tryexecute name=\""<< actor << "\">" << std::endl;
}
void TraceLogStream::traceEndTryExecute(const char * actor){
  stream << "</tryexecute>" << std::endl;
}
void TraceLogStream::traceCommExecIn(size_t size, const char *name){
  stream << "<commexecin size=\""<<size<<"\" name= \""<<name<<"\"\>" << std::endl;
}
void TraceLogStream::traceCommExecOut(size_t size, const char *name){
  stream << "<commexecout size=\""<<size<<"\" name= \""<<name<<"\\>" << std::endl;
}
