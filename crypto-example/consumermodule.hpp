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

#ifndef CONSUMERMODULE_HPP_
#define CONSUMERMODULE_HPP_

#include <stdio.h>
#include <string>
#include <map>
#include <queue>

#include "rsmodule.hpp"
#include "helper.hpp"
#include "examplenetworkpacket.hpp"

/**
 * consumer module consuming incoming packets until command line 
 * completly transferred, executes command and returns result
 */
class ConsumerModule: public RSModule{

  private:
    
    std::map<int, std::string> commands;
    // next command to be executed
    std::string command;
    // list of results still to be returned
    std::queue<std::pair<int, std::string> > results;
   
    /**
     * overloaded for logging purpose
     */
    void produceKey();

    /**
     * reads incoming data packet and construct command line out of data
     */
    void consumeData();

    /**
     * produces data packets containing result of command execution and
     * transmits them to producer
     */
    void produceData();
    
    /**
     * used to build up command line out of several data packets
     * \return id of command to execute or -1 if still further data needed
     */
    int buildCommand(ExampleNetworkPacket packet);

    /**
     * executes command using "bc"
     */
    void executeCommand(int cmdID, ExampleNetworkPacket::EncryptionAlgorithm algo);
   
    /**
     * GUARD used within state machine
     * \see RSModule::RSModule
     */ 
    bool transmitData() const;

public:
    
    ConsumerModule(sc_module_name name,
                   ExampleNetworkPacket::EncryptionAlgorithm type = ExampleNetworkPacket::EM_des3);
      
    ~ConsumerModule(){}

};

#endif // CONSUMERMODULE_HPP_
