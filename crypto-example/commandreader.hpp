#ifndef COMMANDREADER_HPP_
#define COMMANDREADER_HPP_

#include <iostream>
#include <string>

/**
 * helper class to enable reading of one command out of input file
 */
class CommandReader{
  
  private:
    
    ifstream cfile;
    int size;
    int run;
    
  public:
    
    CommandReader(const char* commandfile, int run=0): cfile(commandfile), size(0), run(run){
      // if file exists determine size
      if(this->cfile.good()){
        cfile.seekg (0, ios::end);
        size = cfile.tellg();
        cfile.clear();
        cfile.seekg (0, ios::beg);
      }
    }
    
    ~CommandReader(){
      if(this->cfile.is_open()){
        this->cfile.close();
      }
    }
    
    /**
     * \return true if there are still commands to read
     */
    bool hasCommand(){ 
      int pos = cfile.tellg();
      // if run > 0 restart read
      if(pos+1 >= size && run > 0){
        cfile.clear();
        cfile.seekg (0, ios::beg);
        run--;
        pos = cfile.tellg();
      }
      return (pos+1 < size); 
    }
    
    /**
     * reads one command out of input file and stores it into a string
     */
    std::string readCommand(){
      std::string s;
      char c = '\0';
      while(c != '\n' && this->cfile.good() && !this->cfile.eof()){
        this->cfile.get(c);

        if(c == '\\' && this->cfile.good() && !this->cfile.eof()){
          this->cfile.get(c);
          if(c == '\n'){
            c = '\0';
            continue;
          }
          s.append(1, '\\');
        }
        
        s.append(1, c); 
      }
      
      return s;
    }
};

#endif //COMMANDREADER_HPP_
