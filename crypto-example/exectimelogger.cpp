/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 * 
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
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

