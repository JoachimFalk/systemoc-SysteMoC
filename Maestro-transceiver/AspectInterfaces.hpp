//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2012 Hardware-Software-CoDesign, University of
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

#ifndef TRANSCEIVERM_ASPECTINTERFACES_HPP_
#define TRANSCEIVERM_ASPECTINTERFACES_HPP_


/*
 * Transceiver Model
 * v0.1
 *
 * started: June-2012
 * Rafael Rosales
 * Hardware-Software-Co-Design
 * University of Erlangen-Nuremberg
 *
 * change: rosales      -       22-June-2012 -       Project Created
 * change: rosales      -       26-June-2012 -       First empty AMS model
 * change: rosales      -       27-June-2012 -       PowerModel, TimingModel and Monitor Interfaces defined
 *
 *
 * */

#include <cstdlib>
#include <stdlib.h>
#include <helpers.hpp>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "boost/filesystem.hpp"   // includes all needed Boost.Filesystem declarations
#include <iostream>
using namespace boost::filesystem;
/**
 * Power State machine based Power Model for analog components
 *
 * Power consumption is a function of the current state or current transition
 */
class AMS_PowerModelI
: public smoc_actor {
protected:

  smoc_firing_state start;
  smoc_firing_state transitioning;

  string currentState;
  string nextState;
  double powerConsumption;

  void startTransitioning()
  {
    Request<string> rqst = inStartTransition[0];
    nextState = rqst.object;
    powerConsumption = calculateTransitionPowerConsumption(currentState, nextState);

    Response<double> rspPC(this->getName(),powerConsumption);
    outPowerConsumption[0] = rspPC;
  }

  void doneTransitioning()
  {
    currentState = nextState;
    powerConsumption = calculateStatePowerConsumption(currentState);

    Response<double> rspPC(this->getName(),powerConsumption);
    outPowerConsumption[0] = rspPC;

    Response<string> rspPS(currentState);
    //  outPowerState[0] = rspPS;
  }

public:
  smoc_port_in<Request<string> > inStartTransition;
  smoc_port_in<Request<string> > inCompleteTransition;

  smoc_port_out<Response<double> > outPowerConsumption;
  //smoc_port_out<Response<string> > outPowerState;

  AMS_PowerModelI( sc_module_name name)
  : smoc_actor(name,start)

  {
    start=
        (inStartTransition(1) && outPowerConsumption(1)) >>
        CALL(AMS_PowerModelI::startTransitioning) >>
        transitioning;

    transitioning=
        (inCompleteTransition(1) &&
            outPowerConsumption(1) /*&& outPowerState(1)*/) >>
            CALL(AMS_PowerModelI::doneTransitioning) >>
            start;

  }

  virtual double calculateStatePowerConsumption(string currentState)=0;


  virtual double calculateTransitionPowerConsumption(string sourceState,string destinationState)=0;



};


/**
 * Monitor of the component power consumption
 */
class IPowerMonitor: public smoc_actor {
public:

  smoc_port_in<Response<double> > power;                        //!< Incoming power consumption value

protected:

  void readPowertoken(){
    Response<double> rsp = power[0];

    pCstable= rsp.stable;
    powerConsumption= rsp.object;
    componentName = rsp.source;
  }

  /**
   * Reads
   * currentPowerConsumption
   * componentName
   */
  virtual void updatePowerConsumption()=0;

  smoc_firing_state start;

  bool pCstable;
  double powerConsumption;
  string componentName;

public:
  IPowerMonitor(sc_module_name name)
  : smoc_actor(name, start) {

    start =
        power(1)                                        >>
        CALL(IPowerMonitor::readPowertoken)           >>
        CALL(IPowerMonitor::updatePowerConsumption)     >>
        start
        ;
  }
};


/**
 * This interface defines the interaction of the Control Logic Timing model
 *
 * Specifically, it provides a timing value associated to an control action
 *
 */
class ControlLogicTimingModelI
: public smoc_actor {
protected:
  smoc_firing_state start;

  /*
   * Key = actionName
   * Value= timing
   */
  map<string,sc_time*> timings;

  sc_time* calculatedTiming;

public:

  /**
   * Request timing for a given action
   * param: action name
   */
  smoc_port_in<Request<Transition*> > inGetTiming;
  /**
   * Response to timing request
   * returns: timing value
   */
  smoc_port_out<Response<sc_time*> > outTiming;


  ControlLogicTimingModelI( sc_module_name name)
  : smoc_actor(name,start)

  {
    start= (inGetTiming(1) && outTiming(1)) >>
        CALL(ControlLogicTimingModelI::calculateTransitionTiming) >>
        CALL(ControlLogicTimingModelI::sendTransitionTiming) >>
        start;

  }

  /**
   *  This method should calculate the timing value of a transition provided as parameter on the input port
   *
   *  postconditions: writes on value on calculatedTiming variable
   */
  virtual void calculateTransitionTiming()=0;

  /**
   * This method sends the calculated timing
   *
   * preconditions: calculatedTiming has been set before calling this method
   * postconditions: actor output port sends calculatedTiming
   */
  void sendTransitionTiming()
  {
    Response<sc_time*> rsp(calculatedTiming);
    outTiming[0] = rsp;
  }

};


#endif /* TRANSCEIVERM_ASPECTINTERFACES_HPP_*/
