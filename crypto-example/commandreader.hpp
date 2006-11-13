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
