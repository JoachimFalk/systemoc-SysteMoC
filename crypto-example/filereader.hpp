#ifndef FILEREADER_HPP_
#define FILEREADER_HPP_

#include <ifstream>
#include <string>

template<int SIZE, int LINESIZE>
class LineReader : private ifstream{
  
  private:

    
      
  public:

    FileReader(const char* filename, ios::openmode mode=in) : ifstream(filename, mode){}
    FileReader() : ifstream() {}

    bool hasNext(){
      return !this->eof();
    }
    
    void readNext(char &data[SIZE]){
      memset(data, '\0', SIZE);
      char c;
      for(int i=0; i < SIZE; i++){
        c = this->get();
        
      }
    }
   
    void readLine(){
     line = this->getLine();
    } 
}

#endif //FILEREADER_HPP_
