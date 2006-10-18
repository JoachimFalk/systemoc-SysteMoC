#include "exectimelogger.hpp"

std::auto_ptr<ExecTimeLogger> ExecTimeLogger::singleton(new ExecTimeLogger());

std::ostream& operator << (std::ostream& os, const ExecValue& v){
    return os << v.getMaxExecTime() << "#" << v.getMinExecTime() << "#" << v.getAverageExecTime() << "#" << v.getCallCount();
}

std::ostream& operator << (std::ostream& os, const ActorLogger& al){
  std::map<std::string, ExecValue*>::const_iterator iter;

  iter = al.values.begin();
  for(; iter != al.values.end(); iter++){
    
    os << iter->first << "\t" << *(iter->second) << std::endl;

  }
  return os;
}

/**
 * Definitions for ExecValue
 */

ExecValue::ExecValue() : min_exec(INT_MAX), max_exec(INT_MIN) {}

int ExecValue::getMaxExecTime() const{
  return this->max_exec;
}
        
int ExecValue::getMinExecTime() const{
  return this->min_exec;
}

double ExecValue::getAverageExecTime() const{
  double average = 0;

  for(std::vector<int>::const_iterator iter = this->exec_times.begin();
      iter != this->exec_times.end();
      iter++){
    average += *iter;
  }
  if(average != 0){ 
    average = average / this->exec_times.size();
  }
  
  return average;
}

int ExecValue::getCallCount() const{
  return this->exec_times.size();
}

void ExecValue::logTime(int time){

  this->exec_times.push_back(time);

  if(time < this->min_exec){
      this->min_exec = time;
  }
  if(time > this->max_exec){
    this->max_exec = time;
  }
    
}

/**
 * Definition for torLogger
 */

ActorLogger::ActorLogger() : mutex(0){}

void ActorLogger::logEnter(std::string funcname){

  //std::cerr << "ActorLogger> logEnter(" << funcname << ") with mutex=" << this->mutex << std::endl;
  
  if(this->mutex == 0){
    rusage now;
    getrusage(RUSAGE_SELF,&now);
    this->start_time = now.ru_utime;   
  }

  this->mutex++;
}

void ActorLogger::logExit(std::string funcname){

  //std::cerr << "ActorLogger> logExit(" << funcname << ") with mutex before decrement =" << this->mutex << std::endl;
  
  this->mutex--;

  if(this->mutex == 0){
    ExecValue* value = this->values[funcname];

    if(value == NULL){
      value = new ExecValue();
      this->values[funcname] = value;
    }
    
    rusage now;
    getrusage(RUSAGE_SELF,&now);
    
    int delay = 0;
    
    delay = now.ru_utime.tv_sec - this->start_time.tv_sec;
    delay *=1000000;
    delay += now.ru_utime.tv_usec - this->start_time.tv_usec;
    value->logTime(delay);

  }

  if(this->mutex < 0){
    this->mutex = 0;
  }
}

/**
 * Definitions for ExecTimeLogger
 */

void ExecTimeLogger::logEnter(std::string classname, std::string funcname){

  //std::cerr << "ExecTimeLogger> logEnter(" << classname <<", "<< funcname <<")";
  
  ActorLogger* al = this->loggers[classname];
  if(al == NULL){
    al = new ActorLogger();
    this->loggers[classname] = al;
  }

  al->logEnter(funcname);
}

void ExecTimeLogger::logExit(std::string classname, std::string funcname){

  //std::cerr << "ExecTimeLogger> logExit(" << classname <<", "<< funcname <<")";
  
  ActorLogger* al = this->loggers[classname];
  if(al == NULL){
    al = new ActorLogger();
    this->loggers[classname] = al;
  }  
  
  al->logExit(funcname);        
}

ExecTimeLogger::~ExecTimeLogger(){
  std::ofstream logfile("delay.log");
  if(logfile.is_open()){
    std::map<std::string, ActorLogger*>::iterator iter;
    for(iter = this->loggers.begin(); iter != this->loggers.end(); iter++) {
      
      logfile << iter->first << "\n" << *(iter->second) << std::endl; 
    }
  }
  logfile.flush();
  logfile.close();
}

