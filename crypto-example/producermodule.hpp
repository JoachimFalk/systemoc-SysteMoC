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

#ifndef PRODUCERMODULE_HPP_
#define PRODUCERMODULE_HPP_

#include <iostream>
#include <string>

#include "rsmodule.hpp"
#include "commandreader.hpp"
#include "rlatencywriter.hpp"
#include "helper.hpp"

//#define EX_DEBUG 1

/**
 * Implementatio of producer module for creation of
 * data load packages. In each round the encryption mode of the
 * data is switched between Blowfish and Des.
 */
class ProducerModule: public RSModule {

  private:

    bool succ;
    sc_time delta;
    
    struct result{
      sc_time start;
      std::string str_result;
    };
    
    std::map<int, result> results;
    // instance to retrieve commands from input file
	  CommandReader reader;
    // instance to write request latency into output file
    RLatencyWriter rwriter;

    /**
     * overloaded for logging purpose
     */
    void produceKey();

    /**
     * used to create payload data packets for transmission
     * using command reader
     */
    void produceData();
    
    /**
     * used to print input of received packets
     */
    void consumeData();
    
    /**
     * checks if there is still data to transfer
     */
    bool transmitData() const;
  
    /**
     * performs RSModule::setNext() plus updating internal vars
     */
    void setNext();

    int buildResult(ExampleNetworkPacket packet);

  public:
  
    /**
     *  constructor
     *  \param name specifies name of actor
     *  \param type specifies encryption to be used first
     *  \param input specifies input file of data to transmit. Currently expected to be
     *  text file.
     *  \param run read file run+1 times
     */
    ProducerModule(sc_module_name name, 
                   ExampleNetworkPacket::EncryptionAlgorithm type = ExampleNetworkPacket::EM_des3,
                   const char* input=NULL, int run=0);
    
    ProducerModule(sc_module_name name, const char* input, int run=0);

    ~ProducerModule();
};

#endif // PRODUCERMODULE_HPP_
