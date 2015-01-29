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

#ifndef TRANSCEIVERM_HELPERS_HPP_
#define TRANSCEIVERM_HELPERS_HPP_

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
 * change: rosales      -       27-June-2012 -       Booter-Scheduler, MessagePassing actors and Wrappers Implemented
 *
 *
 * */

#include <cstdlib>
#include <stdlib.h>

#include <xercesc/dom/DOMTreeWalker.hpp>
#include <xercesc/util/PlatformUtils.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/parsers/AbstractDOMParser.hpp>
#include <xercesc/dom/DOMImplementation.hpp>
#include <xercesc/dom/DOMImplementationLS.hpp>
#include <xercesc/dom/DOMImplementationRegistry.hpp>
#include <xercesc/dom/DOMBuilder.hpp>
#include <xercesc/dom/DOMException.hpp>
#include <xercesc/dom/DOMDocument.hpp>
#include <xercesc/dom/DOMNodeList.hpp>
#include <xercesc/dom/DOMError.hpp>
#include <xercesc/dom/DOMLocator.hpp>
#include <xercesc/dom/DOMElement.hpp>
#include <xercesc/dom/DOMNamedNodeMap.hpp>
#include <xercesc/dom/DOMErrorHandler.hpp>
#include <xercesc/util/XMLString.hpp>

#include <CoSupport/XML/Xerces/common.hpp>


using namespace CoSupport::XML::Xerces;

XERCES_CPP_NAMESPACE_USE

using namespace std;

/**
 * Wrapper for data transfer between actors
 */
template<class T>
class Request
{
public:
  string source;   //Actor requesting data
  bool stable;          //False= Current Value, i.e. within the delta cycle / superdense time. True=Stable Value, i.e. fixed point iteration converged value
  T object;            //Stores any data required for the request

  Request(T o)
  {
    this->source = "";
    this->object = o;
    this->stable=false;
  }

  Request(string s, T o)
  {
    this->source = s;
    this->object = o;
    this->stable=false;
  }

  Request(const Request& __x)
  : source(__x.source),
    object(__x.object),
    stable(__x.stable)
  { }

  Request&
  operator=(const Request& __x)
  {
    source = __x.source;
    object = __x.object;
    stable = __x.stable;
    return *this;
  }
};
//operator overload required by smoc-cosupport
template<class T>
ostream &operator<<(ostream& out, const Request<T> & rs )
{
  out << "dummy op" <<endl;
  return out ;
}

/**
 * Wrapper for data transfer between actors
 */
template<class T>
class Response
{
public:
  string source;   //Actor requesting data
  bool stable;          //False= Current Value, i.e. within the delta cycle / superdense time. True=Stable Value, i.e. fixed point iteration converged value
  T object;            //Stores any data required for the request

  Response(T o)
  {
    this->source = "";
    this->object = o;
    this->stable=false;
  }

  Response(string s, T o)
  {
    this->source = s;
    this->object = o;
    this->stable=false;
  }

  Response(const Response& __x)
  : source(__x.source),
    object(__x.object),
    stable(__x.stable)
  { }

  Response&
  operator=(const Response& __x)
  {
    source = __x.source;
    object = __x.object;
    stable = __x.stable;
    return *this;
  }
};
//operator overload required by smoc-cosupport
template<class T>
ostream &operator<<(ostream& out, const Response<T> & rs )
{
  out << "dummy op" <<endl;
  return out ;
}

/**
 * This Class Implements the Interfaces for Booting (registering functional actors) and Scheduling
 */
class Booter_Scheduler_FirmwareIC
:public smoc_actor, public BooterI,public TransitionReadyListener{

protected:
  Actor* actor;
  TransitionReadyListener* leafscheduler;

  smoc_firing_state start;

public:

  Booter_Scheduler_FirmwareIC( sc_module_name name)
  : smoc_actor(name,start)
  {

  }

  /**
   * This method will schedule the next transition execution when a transition is activated
   */
  void waitForTransition()
  {
#ifdef LOG
    log->showAndLogMessage("Sleeping until firmware actor is ready again...");
#endif
    wait();//LeafScheduler will wake up when a transition from the functional actor is activated
#ifdef LOG
    log->showAndLogMessage("Woken up, ready to execute firmware actor");
#endif
  }

  /*
   * Determines if the functional actor can be executed (at least one transition is activated)
   */
  bool canExecuteA() const
  {
    return actor->canExecute();
  }

  /*
   * Executes the functional actor transition
   */
  void executeA()
  {
    //Ellapse time span
    //wait(transitionTiming);

    //Execute transition
    actor->execute();

#ifdef LOG
    log->showAndLogMessage("Actor transition executed...");
#endif
  }


  /**
   * Maestro callback to determine that this actor will: get the list of functional actors to be scheduled and listen to functionality activity
   */
  virtual bool isBooter()
  {
    return true;
  }

  /**
   * IBooter method implementation
   *
   * Will register the functional actor to be scheduled by this actor
   *
   * Maestro will provide the mapped actor from the XML at ellaboration phase
   */
  virtual void registerActor(Actor& nActor)
  {
#ifdef LOG
    log->showAndLogMessage("Registering actor: "+ nActor.getName());
#endif
    actor=&nActor;
  }

  /**
   * IBooter method implementation
   *
   * Registers this actor as a listener of functional actors activities for optimized scheduling
   */
  virtual void registerTL()
  {
    //Register TransitionReadyListener on actor
    actor->registerTransitionReadyListener(*this);
  }

  /**
   *
   * Transitionreadylistener method implementation
   *
   * Whenever a registered actor has an enabled transition, this function will be called catching that event
   * The event will be forwarded to this actor's leafscheduler
   */
  virtual void notifyTransitionReady(Transition& transition){
#ifdef LOG
    log->showAndLogMessage("Actor ready for execution: " + transition.parentActor->getName());
#endif

    //actorReady = transition.parentActor;

    leafscheduler->notifyTransitionReady(transition);

  }

  /**
   * smoc_actor method overload
   *
   */
  virtual void registerTransitionReadyListener(TransitionReadyListener& listener)
  {
    //Register LeafScheduler of this Booter actor in local pointer
    this->leafscheduler = &listener;
    //Register LeafScheduler as listener of actor transitions
    smoc_actor::registerTransitionReadyListener(listener);
  }
};

/**
 * Broadcast 1 input to 2 outputs FIFO - rendevouz style
 */
template<class T>
class Broadcast1_2: public smoc_actor {
public:
  smoc_port_in<Response<T> > in;
  smoc_port_out<Response<T> > out1;
  smoc_port_out<Response<T> > out2;

private:

  void FwdAll() {
    Fwd1();
    Fwd2();
  }

  void Fwd1() {
      out1[0] = in[0];
    }

  void Fwd2() {
     out2[0] = in[0];
    }

  smoc_firing_state ready;

public:
  Broadcast1_2(sc_module_name name)
    : smoc_actor(name, ready) {

      ready =
        in(1)                                   >>
        out1(1)                                 >>
        out2(1)                                 >>
        CALL(Broadcast1_2::FwdAll)            >>
        ready
      ;


  }
};

//unfair merger (order will depend on the implicit systemoc priority given to the transitions
//to change it modify systemoc
//or implement as XOR state
template<class T>
class Merger2_1: public smoc_actor {
public:
  smoc_port_in<Response<T> > in1;
  smoc_port_in<Response<T> > in2;

  smoc_port_out<Response<T> > out;

private:

  smoc_firing_state ready;


  void fwd1()
    {
     out[0]=  in1[0];
    }
  void fwd2()
    {
    out[0]=  in2[0];
    }



public:
  Merger2_1(sc_module_name name)
    : smoc_actor(name, ready) {

    ready =
        in1(1)                 >>
        out(1)                     >>
        CALL(Merger2_1::fwd1)      >>
        ready
        |
        in2(1)                 >>
        out(1)                     >>
        CALL(Merger2_1::fwd2)      >>
        ready
        ;

  }
};

#endif /* TRANSCEIVERM_HELPERS_HPP_ */
