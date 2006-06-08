#ifndef LATENCYWRITER_HPP_
#define LATNECYWRITER_HPP_

#include <systemc.h>
#include <iostream>
#include <string>

/**
 * helper class to enable writing of request latencies to log_file
 */
class RLatencyWriter {

  private:

    ofstream ofile;

  public:

    RLatencyWriter(const char* latencyfile): ofile(latencyfile){
    }

    ~RLatencyWriter(){
      if(this->ofile.is_open()){
        this->ofile.close();
      }
    }

    void writeLatency(int id, sc_time time, sc_time delta){
      this->ofile << id << "\t" << time << "\t" << delta << std::endl;
    }

};

#endif //LATENCYWRITER_HPP_
