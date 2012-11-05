//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
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

#include <cstdlib>
#ifndef XILINX_EDK_RUNTIME
# include <iostream>
#else
# include <stdlib.h>
#endif

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#include <CoSupport/Tracing/TracingFactory.hpp>


#include <cmath>
#include <cassert>

#include <vpc_api.hpp>
namespace VC = SystemC_VPC::Config;

using namespace std;

// Maximum (and default) number of Src iterations. Lower default number via
//  command line parameter.
const int NUM_MAX_ITERATIONS = 1000000;


class Src: public smoc_actor {
public:
  smoc_port_out<double> out;
  smoc_port_in<double> in;
private:
  int i, j;
  CoSupport::Tracing::PtpTracer::Ptr trace_object;

  void reset() {
    std::cout << "src: Reset i " << i << " @ " << sc_time_stamp() << std::endl;
    i = j;
  }
  void src() {
    std::cout << "src: " << i << " @ " << sc_time_stamp() << std::endl;
    trace_object->start();

#ifndef NDEBUG
# ifndef XILINX_EDK_RUNTIME
#  ifndef VAST
    //std::cout << "src: name: " << name() << " " << i  << std::endl;
#  else
    printf("src: %d\n", i);
#  endif
# else
    xil_printf("src: %u\n",i);
# endif
#endif
    out[0] = i++;
  }

  smoc_firing_state start;
public:
  Src(sc_module_name name, int from)
    : smoc_actor(name, start), i(from), j(from) {

      trace_object = ((CoSupport::Tracing::PtpTracer::Ptr)CoSupport::Tracing::TracingFactory::getInstance().createPtpTracer("ChainTrace"));

      SMOC_REGISTER_CPARAM(from);

      start =
        (VAR(i) <= NUM_MAX_ITERATIONS) >>
        out(1)                         >>
        CALL(Src::src)                 >> start |
        (VAR(i) > NUM_MAX_ITERATIONS) >>
        in(1)                         >>
        CALL(Src::reset)                >> start
      ;
  }
};


class TestClass: public smoc_periodic_actor{
  public:
  smoc_port_out<double> pulse_out;

  TestClass(sc_module_name name): smoc_periodic_actor(name, start, sc_time(2500,SC_US), sc_time(2520, SC_US)){
    start = pulse_out(1) >> CALL(TestClass::src) >> start;
  }
  void src() {
    std::cout<< this->name() << " @ " << sc_time_stamp() << std::endl;
    pulse_out[0] = 0;
  }
    smoc_firing_state start;
};

namespace VC = SystemC_VPC::Config;

class GovernorPSM: public smoc_actor {
private:

  void shutdownPowerMode(){
    VC::changePowerMode(*this, "SHUTDOWN");
    std::cout<<"PSM::shutdownPowerMode() @ " << sc_time_stamp() <<std::endl;
    VC::setCanExec(*this, false);
    }

  void wakeupPowerMode(){
    VC::changePowerMode(*this, "WAKEUP");
    std::cout<<"PSM::wakeupPowerMode() @ " << sc_time_stamp() <<std::endl;
    VC::setCanExec(*this, false);
    }


  void sleepPowerMode(){
    VC::changePowerMode(*this, "SLEEP");
    std::cout<<"PSM::sleepPowerMode() @ " << sc_time_stamp() <<std::endl;
    VC::setCanExec(*this, false);
      }

  void normalPowerMode(){
    VC::changePowerMode(*this, "SLOW");
    std::cout<<"PSM::normalPowerMode() @ " << sc_time_stamp() <<std::endl;
    VC::setCanExec(*this, true);
  }

  smoc_firing_state normal, shutdown, lpi, wakeup;

public:
  smoc_port_in<bool>wup;
  smoc_port_in<bool>sdown;
  smoc_port_in<bool>norm;
  smoc_port_in<bool>sleep;

  GovernorPSM(sc_module_name name)
      : smoc_actor( name, normal){
    VC::setActorAsPSM(name, true);
    normal = sdown(1) >>
             CALL(GovernorPSM::shutdownPowerMode) >> shutdown ;
    lpi =    wup(1) >>
             CALL(GovernorPSM::wakeupPowerMode) >> wakeup;
    shutdown = sleep(1) >>
             CALL(GovernorPSM::sleepPowerMode) >> lpi |
             wup(1) >>
             CALL(GovernorPSM::wakeupPowerMode) >> wakeup;
    wakeup = norm(1) >>
             CALL(GovernorPSM::normalPowerMode) >> normal;
    }
};

class GovernorDPMP: public smoc_actor {
private:
  void shutdownPowerMode(){
      std::cout<<"shutdownPowerMode @ " << sc_time_stamp() << std::endl;
      noReadyTasks->reset();
      sdown[0]=true;
      statechange_out[0]=true;
    }
  void shutdownPowerMode2(){
      std::cout<<"shutdownPowerMode2 @ " << sc_time_stamp() << std::endl;
      noReadyTasks->reset();
      sdown[0]=true;
      statechange_out[0]=true;
    }

  void wakeupPowerMode(){
    readyTasks->reset();
    std::cout<<"wakeupPowerMode @ " << sc_time_stamp() << std::endl;

    wup[0] = true;
    }


  void sleepPowerMode(){
    std::cout<<"sleepPowerMode @ " << sc_time_stamp() << std::endl;
      sleep[0] = true;

    }

  void normalPowerMode(){
    noReadyTasks->reset();
    //readyTasks->reset();
    std::cout<<"normalPowerMode @ " << sc_time_stamp() << std::endl;
     norm[0] = true;
  }

  void initProcess(){
    std::cout<<"InitProcess @ " << sc_time_stamp() << std::endl;
    VC::registerComponentWakeup(name(), readyTasks);
    VC::registerComponentIdle(name(), noReadyTasks);
    //norm[0] = true;
  }

  bool hasNewTasks(void) const{
    return true; //sc_time_stamp() > sc_time(1, SC_US);
    //TODO: add backlink from VPC / Component? and configure the readyTasks event_or_list
  }

  bool hasStatechangeInput(void) const{
    std::cout<<"hasStatechangeInput is " << statechange_in.numAvailable() << " @ " << sc_time_stamp() << std::endl;
    //std::cout<<" more info: " << sdown.numFree() << " " << statechange_out.numFree()<<std::endl;
    //return false;
    return statechange_in.numAvailable() >0;
    //TODO: integrated workaround for defined state-change (maybe an additional fifo is needed?)
    //return statechange_in.tokenIsValid(1);
  }


  smoc_firing_state normal, shutdown, lpi, wakeup, init;

public:
  smoc_vpc_event_p readyTasks;
  smoc_vpc_event_p noReadyTasks;
  smoc_port_out<bool>wup;
  smoc_port_out<bool>sdown;
  smoc_port_out<bool>statechange_out;
  smoc_port_in<bool>statechange_in;
  smoc_port_out<bool>norm;
  smoc_port_out<bool>sleep;
  smoc_port_out<bool>interrupted;

  GovernorDPMP(sc_module_name name)
      : smoc_actor( name, init){
    readyTasks = new smoc_vpc_event();
    noReadyTasks = new smoc_vpc_event();
    //sc_event componentCallback;
    init =  CALL(GovernorDPMP::initProcess) >> normal;
    normal = !GUARD(GovernorDPMP::hasStatechangeInput) >> Expr::till(*noReadyTasks) >> (sdown(1) && statechange_out(1)) >>
             CALL(GovernorDPMP::shutdownPowerMode) >> shutdown  |
             GUARD(GovernorDPMP::hasStatechangeInput) >> Expr::till(*noReadyTasks) >>  (sdown(1) && statechange_in(1) && statechange_out(1)) >> CALL(GovernorDPMP::shutdownPowerMode2) >> shutdown;
    lpi =    wup(1) >> /*(GUARD(GovernorDPMP::hasNewTasks)) >> */ Expr::till(*readyTasks) >>
             CALL(GovernorDPMP::wakeupPowerMode) >> wakeup;
    shutdown = (sleep(1) && statechange_in(1)) >>
             CALL(GovernorDPMP::sleepPowerMode) >> lpi |
              /*(GUARD(GovernorDPMP::hasNewTasks)) >>*/ Expr::till(*readyTasks) >>  (wup(1) && interrupted(1)) >>
             CALL(GovernorDPMP::wakeupPowerMode) >> wakeup;
    wakeup = norm(1) >>
             CALL(GovernorDPMP::normalPowerMode) >> normal;
    }
};

const int NUM_ITERATIONS_DPMP2 = 10;

class GovernorDPMP2: public smoc_actor {
public:
  smoc_port_in<bool> in;
  smoc_port_in<bool> interrupted;
  smoc_port_out<bool> out;
private:
  int counter;

  void shutdown(void) {
    counter++;
    //TODO: maybe add additional state for "detecting current activity" of DPMP2
  }

  void shutdown_restart(void) {
    std::cout<<"DPMP2 shutdown_restart"<<std::endl;
    counter = 0;
      //TODO: maybe add additional state for "detecting current activity" of DPMP2
    }

  void shutdown_finished(void){
    std::cout<<"DPMP2 shutdown_finished" << std::endl;
    out[0] = true;
    counter=NUM_ITERATIONS_DPMP2+1;
  }

  void shutdown_reset(void) {
    std::cout<<"DPMP2 shutdown_reset" << std::endl;

    counter = NUM_ITERATIONS_DPMP2+1;
      //TODO: maybe add additional state for "detecting current activity" of DPMP2
    }

  bool hasInterruptedInput(void) const{
    std::cout<<"DPMP2 hasInterruptedInput is=" << interrupted.numAvailable() <<std::endl;
      return interrupted.numAvailable();
    }

  smoc_firing_state start;
public:
  GovernorDPMP2(sc_module_name name)
    : smoc_actor(name, start) {
    counter=NUM_ITERATIONS_DPMP2+1;
    start =
        !GUARD(GovernorDPMP2::hasInterruptedInput) >> (in(1))             >>
        CALL(GovernorDPMP2::shutdown_restart)  >>
        start |
        (!GUARD(GovernorDPMP2::hasInterruptedInput) && (VAR(counter)< NUM_ITERATIONS_DPMP2))>>
        CALL(GovernorDPMP2::shutdown)  >>
        start |
        ((VAR(counter)==NUM_ITERATIONS_DPMP2))  >> out(1) >>
        CALL(GovernorDPMP2::shutdown_finished)>>
        start |
        interrupted(1) >>  CALL(GovernorDPMP2::shutdown_reset)  >>
        start;
  }


};


class Sink: public smoc_actor {
public:
  smoc_port_in<double> in;
private:
  CoSupport::Tracing::PtpTracer::Ptr trace_object;
  void sink(void) {
   // std::cout << "sink: name " << name() << " " << " @ " << sc_time_stamp() << std::endl;
    trace_object->stop();
#ifndef NDEBUG
# ifndef XILINX_EDK_RUNTIME
#  ifndef VAST
    // std::cout << "sink: " << in[0] << std::endl;
#  else
    printf("sink: %f\n", in[0]);
#  endif
# else
    xil_printf("sink: %u\n",in[0]);
# endif
#endif
  }
  
  smoc_firing_state start;
public:
  Sink(sc_module_name name)
    : smoc_actor(name, start) {
    trace_object = ((CoSupport::Tracing::PtpTracer::Ptr)CoSupport::Tracing::TracingFactory::getInstance().createPtpTracer("ChainTrace"));
    start =
        in(1)             >>
        CALL(Sink::sink)  >>
        start
      ;
  }
};

class SqrRoot
: public smoc_graph {
public:
protected:
  Src       src;
  Sink      sink;
  GovernorPSM  governor;
  GovernorDPMP dpmp;
  GovernorDPMP2 dpmp2;
  TestClass testClass;
public:
  SqrRoot( sc_module_name name, const int from = 1 )
    : smoc_graph(name),
      src("a1", from),
      sink("a2"),
      governor("a3"), dpmp("DPMP"), dpmp2("DPMP2"), testClass("Pulse") {
    VC::setActorAsPSM("sqrroot.a3", true);
    VC::setActorAsPSM("sqrroot.DPMP", true);
    smoc_fifo<double> fifo("channel", 200);

    fifo.connect(src.out).connect(sink.in);
    //connectNodePorts(src.out,sink.in);
    connectNodePorts(dpmp.sleep,governor.sleep);
    connectNodePorts(dpmp.wup,governor.wup);
    connectNodePorts(dpmp.norm,governor.norm);
    connectNodePorts(dpmp.sdown,governor.sdown);
    connectNodePorts(dpmp.statechange_out,dpmp2.in);
    connectNodePorts(dpmp2.out, dpmp.statechange_in);

    smoc_fifo<double> pulser("pulser", 5);
    pulser.connect(testClass.pulse_out).connect(src.in);
    connectNodePorts<4>(dpmp.interrupted, dpmp2.interrupted);

  }
};

int sc_main (int argc, char **argv) {
  int from = 1;
  if (argc >= 2) {
    const int iterations = atoi(argv[1]);
    assert(iterations < NUM_MAX_ITERATIONS);
    from = NUM_MAX_ITERATIONS - iterations;
  }
  size_t runtime = (argc>2?atoi(argv[2]):10);
  smoc_top_moc<SqrRoot> sqrroot("sqrroot", from);
  sc_start(sc_time(runtime,SC_MS));
  return 0;
}
