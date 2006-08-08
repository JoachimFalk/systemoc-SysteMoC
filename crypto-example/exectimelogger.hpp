#ifndef EXECTIMELOGGER_HPP_
#define EXECTIMELOGGER_HPP_

#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

#include <iostream>
#include <fstream>

#include <string>
#include <vector>
#include <map>

#include <limits.h>


#define LOG_METHOD_ENTER(classname, funcname) ExecTimeLogger::getInstance().logEnter(classname, funcname);
#define LOG_METHOD_EXIT(classname, funcname) ExecTimeLogger::getInstance().logExit(classname, funcname);

/**
 * \brief helper class for storing logging data for specific method call
 */
class ExecValue{

  private:
    // maximal logged execution time
    int max_exec;
    // minimal logged execution time
    int min_exec;
    // list of all logged execution times
    std::vector<int> exec_times;

  public:
    ExecValue();

    /**
     * \brief returns the max execution time recorded up to now
     */
    int getMaxExecTime() const;

    /**
     * \brief returns the min execution time recorded up to now
     */
    int getMinExecTime() const;

    /**
     * \brief returns the average execution time over all recorded time values
     */
    double getAverageExecTime() const;
   
    /**
     *
     */
    int getCallCount() const;

    void logTime(int time); 
    
    friend std::ostream& operator << (std::ostream& os, const ExecValue& v);
    
}; 

/**
 * \brief helper class to hold logged times for a specific actor
 */
class ActorLogger{
 
  public:  
    // map of logged values
    std::map<std::string, ExecValue*> values;

  private: 
    // mutex to ensure to log only upper calls
    int mutex;
    // start time of logging
    timeval start_time;
     
  public:

    ActorLogger();

    void logEnter(std::string funcname);

    void logExit(std::string funcname);

    friend std::ostream& operator << (std::ostream& os, const ActorLogger& al);
    
};

/**
 * \brief Enables logging of execution times
 */
class ExecTimeLogger {

  private:
  
    /**
     * Singleton design pattern
     */
    static std::auto_ptr<ExecTimeLogger> singleton;
    
    ExecTimeLogger(){};
    
    
    // contains all logs associated with function calls
    std::map<std::string, ActorLogger*> loggers; 

  public:

    static ExecTimeLogger& getInstance(){
      return *singleton;
    }
    
    virtual ~ExecTimeLogger();

    void logEnter(std::string classname, std::string funcname);

    void logExit(std::string classname, std::string funcname);
  
};

#endif //EXECTIMELOGGER_HPP_
