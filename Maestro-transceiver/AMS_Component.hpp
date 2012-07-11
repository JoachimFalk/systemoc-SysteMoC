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

#ifndef TRANSCEIVERM_AMS_COMPONENT_HPP_
#define TRANSCEIVERM_AMS_COMPONENT_HPP_


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
 * change: rosales      -       27-June-2012 -       Analog Component v1.0 - Scripted Power and Timing models
 *
 *
 * */

#include <cstdlib>
#include <stdlib.h>
#include <helpers.hpp>
#include <AspectInterfaces.hpp>
#include <AspectModels.hpp>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>

#include <time.h>


#define LOG

using namespace std;

/**
 * Simple Amplifier
 *
 * It can be enabled/disabled
 */
class Amplifier_Functionality
: public smoc_actor {
private:
  smoc_firing_state enabled;
  smoc_firing_state disabled;
  double gain;

public:

  smoc_port_in<double> input;
  smoc_port_out<double> output;

  smoc_port_in<Request<bool> > inDisable;

  Amplifier_Functionality( sc_module_name name)
  : smoc_actor(name,enabled)

  {

    enabled =
        (inDisable(1) && GUARD(Amplifier_Functionality::isDisable))>>
        CALL(Amplifier_Functionality::Disable) >>
        disabled
        |
        (inDisable(1) && !GUARD(Amplifier_Functionality::isDisable))>>
        enabled
        |
        (input(1) && output(1)) >>
        CALL(Amplifier_Functionality::Amplify) >>
        enabled
        ;

    disabled =
        (inDisable(1) && GUARD(Amplifier_Functionality::isDisable))>>
        disabled
        |
        (inDisable(1) && !GUARD(Amplifier_Functionality::isDisable))>>
        CALL(Amplifier_Functionality::Enable) >>
        enabled
        ;

    gain = 20;

  }

  void Amplify()
  {
    log->showAndLogMessage("Amplifying. Input: " + to_string(input[0]) + " Gain: " + to_string(gain),3);
    output[0]=input[0]*gain;
    wait(1,SC_NS);//time taken. No timing model created to simplify dummy design
    // Note: If functionality never moves in time, then it will forever compute it at time = 0 s, and other components will never reach their
    // execution time
  }

  bool isDisable() const
  {
    Request<bool> rqst = inDisable[0];
    return rqst.object;
  }

  void Disable()
  {
    log->showAndLogMessage("Amplifier disabled",3);
  }

  void Enable()
  {
    log->showAndLogMessage("Amplifier enabled",3);
  }
};






/**
 * Control logic for DEMO purposes
 *
 * Sample Control Logic driving the power state of the power model
 *
 * This sample does not receive commands from a system power manager, but simple switches off and on.
 * It disables the functionality of the analog component and notifies the power model if a transition is under progress or if it is completed
 *
 *
  */
class AmplifierControlLogic
: public smoc_actor {
private:
  smoc_firing_state start;
  smoc_firing_state Offtransitioning;
  smoc_firing_state waitON;
  smoc_firing_state waitOFF;
  smoc_firing_state Ontransitioning;
  smoc_firing_state toOn;

public:

  smoc_port_out<Request<string> > outStartTransition;
  smoc_port_out<Request<string> > outCompleteTransition;
  smoc_port_out<Request<bool> > outDisableFunctionality;

  AmplifierControlLogic( sc_module_name name)
  : smoc_actor(name,start)

  {

    start=
        outStartTransition(1) >>
        CALL(AmplifierControlLogic::notifyTransitionToOFFBegin)>> //notify powermodel that a transition is about to start
        Offtransitioning;

    Offtransitioning=
            (outCompleteTransition(1) && outDisableFunctionality(1)) >>
            CALL(AmplifierControlLogic::PowerOff)>> // perform the transition, consuming the annotated value (done by scheduler/IC behavior)
            waitON;

    waitON=
        CALL(AmplifierControlLogic::doWaitON) >>
        toOn;

    toOn=
        outStartTransition(1) >>
            CALL(AmplifierControlLogic::notifyTransitionToONBegin)>> //notify powermodel that a transition is about to start
            Ontransitioning;

    Ontransitioning=
        (outCompleteTransition(1) && outDisableFunctionality(1)) >>
           CALL(AmplifierControlLogic::PowerON)>> // perform the transition, consuming the annotated value (done by scheduler/IC behavior)
            waitOFF;

    waitOFF=
            CALL(AmplifierControlLogic::doWaitOFF) >>
            start;
  }

  void doWaitON()
  {
    log->showAndLogMessage("Waited a timespan before turning ON again...",4);
  }

  void doWaitOFF()
    {
      log->showAndLogMessage("Waited a timespan before turning OFF again...",4);
    }
  void notifyTransitionToOFFBegin()
    {
    log->showAndLogMessage("notifyTransitionToOFFBegin",4);
    string nextState = "off";
    Request<string> rqst(nextState);
    outStartTransition[0]=rqst;
    }

  void notifyTransitionToONBegin()
      {
    log->showAndLogMessage("notifyTransitionToONBegin",4);
      string nextState = "on";
      Request<string> rqst(nextState);
      outStartTransition[0]=rqst;
      }

  void PowerOff()
  {
    log->showAndLogMessage("PowerOff",4);
    //Disable Functionality
    bool disable= true;
    Request<bool> rqst(disable);
    outDisableFunctionality[0] = rqst;

    //Notify power model that transition is over
    string nextState = "off";
    Request<string> rqst2(nextState);//nextState parameter is redundant information here, consider removing it if no re-check is necessary
    outCompleteTransition[0]=rqst2;
  }

  void PowerON()
    {
    log->showAndLogMessage("PowerON",4);
      //Enable Functionality
      bool disable= false;
      Request<bool> rqst(disable);
      outDisableFunctionality[0] = rqst;

      //Notify power model that transition is over
      string nextState = "on";
      Request<string> rqst2(nextState);//nextState parameter is redundant information here, consider removing it if no re-check is necessary
      outCompleteTransition[0]=rqst2;
    }


};







/**
 * Model of a dummy "Analog" amplifier
 *
 * Aspects:
 * Functionality
 * PowerModel
 * PowerMonitor
 *
 */
class AnalogAmplifier
: public smoc_graph {
public:
  Amplifier_Functionality       functionalModel;

  AMS_PowerModel         powerModel;
  PowerMonitor           powerMonitor;
  Broadcast1_2<double>   powerModelDist; //Distributes data from power model to two different destinations

  AnalogAmplifier( sc_module_name name)
  : smoc_graph(name),
   powerMonitor("PM"),
   powerModel("PowerModel"),
   powerModelDist("PowerModelDist"),
   functionalModel("FunctionalModel")
  {
    //Connect PowerModel to PowerMonitor
    connectNodePorts(powerModel.outPowerConsumption, powerModelDist.in);
    connectNodePorts(powerModelDist.out1,powerMonitor.power);

    //Connection to TopPowerMonitor pending. Implement on Top component...

    //TODO: if required
    //connectNodePorts(powerModel.outPowerState, powerMonitor.inPowerState);

  }


};


/**
 * Sample Amplifier component
 *
 * It models the following aspects of an amplifier:
 *
 * Analog part:
 * functionality (discrete time)
 * power model (power state machine based)
 * power monitor (producing power traces)
 *
 * Control part:
 * digital control logic (controling the power state)
 * timing model for the control logic (assigning timing values to power state transitions)
  *
 */
class Amplifier
: public smoc_graph {


private:

/**
 * Firmware controlling the analog component
 *
 * This functionality is mapped onto the DigitalControl subcomponent
 */
  AmplifierControlLogic controlLogic;


public:
  /**
   * Analog part implementing the analog functionality
   */
  AnalogAmplifier       analogAmplifier;

  /*
   * Digital part executing the firmware control of this component
   */
  DigitalControl        controlIC;

  //smoc_port_in<double> input;
  //smoc_port_out<double> output;

  Amplifier( sc_module_name name)
  : smoc_graph(name),
   controlIC("DigitalControl"),
   controlLogic("ControlLogic"),
   analogAmplifier("AnalogAmplifier")
  {
    //Expose amplifier ports
    //connectNodePorts(this->input,functionalModel.input);
    //connectNodePorts(this->output,functionalModel.output);

    //Connect ControlLogic to AMS.PowerModel
    connectNodePorts(controlLogic.outStartTransition, analogAmplifier.powerModel.inStartTransition);
    connectNodePorts(controlLogic.outCompleteTransition, analogAmplifier.powerModel.inCompleteTransition);

    //Connect ControlLogic to FunctionalModel
    connectNodePorts(controlLogic.outDisableFunctionality, analogAmplifier.functionalModel.inDisable);


  }
};

/**
 * test bench source actor
 */
class Src
: public smoc_actor {
private:

  double i;
  smoc_firing_state start;

public:
  smoc_port_out<double> output;


  Src( sc_module_name name)
  : smoc_actor(name,start)

  {
      i=0;
      start= output(1) >>
          CALL(Src::Create)>>
          start;
  }

  void Create()
  {
    log->showAndLogMessage("Input value " + to_string(i),4);
    output[0]= i;
    i++;
    if(i > 10)
      {
        log->showAndLogMessage("Exiting simulation. Dummy code produces only 10 values.",4);
      sc_stop();
      }
  }
};

/**
 * test bench sink actor
 */
class Snk
: public smoc_actor {
private:
  int c;
  smoc_firing_state start;

public:
  smoc_port_in<double> input;


  Snk( sc_module_name name)
  : smoc_actor(name,start)

  {
    c=0;
    start= input(1)>>
    CALL(Snk::print)>>
    start;
  }

  void print()
  {
    double value = input[0];
    log->showAndLogMessage("Index:" + to_string(c) + " Output value: " + to_string(value),4);
    c++;
  }
};

#endif /*TRANSCEIVERM_AMS_COMPONENT_HPP_*/
