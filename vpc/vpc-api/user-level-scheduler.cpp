// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of
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

#include <iostream>

#include <systemoc/smoc_moc.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
#include <vpc_api.hpp>
namespace VC = SystemC_VPC::Config;
#endif // SYSTEMOC_ENABLE_VPC
static const std::string MESSAGE_HELLO = "Hello SysteMoC!";

class UserLevelScheduler
{
public:
  virtual void changeScheduling() = 0;
  virtual ~UserLevelScheduler(){}
};

class Source: public smoc_actor
{
public:
  smoc_port_out<char> out;

  Source(sc_module_name name, UserLevelScheduler * uls) :
    smoc_actor(name, start), count(0), size(MESSAGE_HELLO.size()), message(
        MESSAGE_HELLO), uls_(uls)
  {
    start = GUARD(Source::hasToken) >> out(1) >> CALL(Source::source) >> start;
  }

private:
  smoc_firing_state start;

  unsigned int count, size; // variables (functional state)
  const std::string message; //
  UserLevelScheduler * uls_;

  bool hasToken() const
  {
    return count < size;
  } // guard
  void source()
  {
    std::cout << this->name() << "> @ " << sc_time_stamp() << "\tsend: \'"
        << message[count] << "\'" << std::endl;
    {

#ifdef SYSTEMOC_ENABLE_VPC
      //VC::getComponent("CPU1")->getComponentInterface()->changePowerMode("FAST");
      uls_->changeScheduling();
#endif // SYSTEMOC_ENABLE_VPC
    }

    out[0] = message[count++];
  } // action
};

class Sink: public smoc_actor
{
public:
  // ports:
  smoc_port_in<char> in;

  Sink(sc_module_name name, UserLevelScheduler * uls) // actor constructor
  :
    smoc_actor(name, start), uls_(uls)
  {
    // FSM definition:
    start = in(1) >> CALL(Sink::sink) >> start;
  }
private:
  smoc_firing_state start; // FSM states
  UserLevelScheduler * uls_;

  void sink()
  {
    std::cout << this->name() << "> @ " << sc_time_stamp() << "\trecv: \'"
        << in[0] << "\'" << std::endl;
    try {
#ifdef SYSTEMOC_ENABLE_VPC
      //VC::getComponent("CPU0")->getComponentInterface()->changePowerMode("FAST");
      uls_->changeScheduling();
#endif // SYSTEMOC_ENABLE_VPC
    } catch (std::exception & e) {
      std::cerr << " Got: " << e.what() << std::endl;
      exit(-1);
    }
  }
};

class NetworkGraph: public smoc_graph, public UserLevelScheduler
{
public:
  NetworkGraph(sc_module_name name, bool vpcConfig) // network graph constructor
   : smoc_graph(name), source("src",this), sink("snk",this)
  {
    smoc_fifo<char> fifo("queue", 42);
    fifo.connect(source.out).connect(sink.in); // connect actors

#ifdef SYSTEMOC_ENABLE_VPC
    if (vpcConfig) {
      try {
        std::cerr << " start vpc configuration" << std::endl;

        // convenience
        namespace VC = SystemC_VPC::Config;

        // configuration takes place during elaboration phase
        // the configuration may not change after elaboration
        // TODO: we should assert this in the configuration API

        //create components
        VC::Component::Ptr cpu0 = VC::createComponent("CPU0",
            VC::Scheduler::StaticPriority_NP);
        VC::Component::Ptr cpu1 = VC::createComponent("CPU1",
            VC::Scheduler::DynamicPriorityUserYield);
        std::cerr << "cid0: " << cpu0->getComponentId() << "\ncid1: "
            << cpu1->getComponentId() << std::endl;

        // configuration
        cpu0->setTransferTiming(VC::Timing(sc_time(10, SC_NS)));
        //cpu0->setScheduler(VC::Scheduler::FCFS);

        // mappings (cannot change after elaboration TODO: assert this)
        cpu1->addTask(sink);
        cpu1->addTask(source);
        //FIXME: cpu1->addTask(sink);

        // set priorities etc
        VC::setPriority(sink, 1);
        VC::setPriority(source, 2);

        // timings
        std::set<VC::Timing> cpu0Timings;
        std::set<VC::Timing> cpu1Timings;

        VC::DefaultTimingsProvider::Ptr provider0 =
            cpu0->getDefaultTimingsProvider();

        provider0->add(VC::Timing("Source::hasToken", sc_time(10, SC_NS),
            sc_time(10, SC_NS))); // dii, latency
        provider0->add(VC::Timing("Source::source", sc_time(10, SC_NS),
            sc_time(10, SC_NS))); // dii, latency
        provider0->add(VC::Timing("Sink::sink", sc_time(10, SC_NS))); // delay

        VC::DefaultTimingsProvider::Ptr provider1 =
            cpu1->getDefaultTimingsProvider();
        provider1->add(VC::Timing("Source::hasToken", sc_time(10, SC_NS),
            sc_time(10, SC_NS))); // dii, latency
        provider1->add(VC::Timing("Source::source", sc_time(100, SC_NS),
            sc_time(100, SC_NS))); // dii, latency
        provider1->add(VC::Timing("Sink::sink", sc_time(100, SC_NS))); // delay

        // optional configuration stuff
        // reuseTiming (call by value)
        //cpu1->setTimings(cpu0Timings);


        // - use existing string parser to create sc_time objects
        //sourceTimings.insert(Timing("m_h_source::source", "10 ns", "10 ns"));

        // prevent from multiple creations (exception)
        //VC::Component::Ptr foo = VC::createComponent("CPU0");

        // throw exception if not created
        //VC::Component::Ptr bar = VC::getComponent("Foo");

        // use explicit handles to VpcTask
        //      VC::VpcTask::Ptr vpcSrc = VC::getCachedTask(source);
        //      VC::VpcTask::Ptr vpcSnk = VC::getCachedTask(sink);
        //      vpcSrc->mapTo(cpu0);
        //      vpcSnk->mapTo(cpu0);
        //      vpcSrc->setPriority(0);
        //      vpcSnk->setPriority(0);

        // TODO:
        // configure routing
        VC::ignoreMissingRoutes(true);

        VC::Component::Ptr bus = VC::createComponent("Bus");
        VC::Component::Ptr mem = VC::createComponent("Memory");

        VC::Timing transfer(sc_time(20,SC_NS), sc_time(20,SC_NS));
        bus->setTransferTiming(transfer);
        mem->setTransferTiming(transfer);
        cpu0->setTransferTiming(transfer);
        cpu1->setTransferTiming(transfer);

//      VC::Route::Ptr writeRoute = VC::createRoute(getLeafPort(&source.out));
//      VC::Route::Ptr readRoute = VC::createRoute(getLeafPort(&sink.in));
        VC::Route::Ptr writeRoute = VC::createRoute(&source.out);
        VC::Route::Ptr readRoute = VC::createRoute(&sink.in);

        sc_time d(10, SC_NS);
        sc_time l(20, SC_NS);

        //        writeRoute->addHop(cpu1); // TODO: getComp(src)
        writeRoute->addHop(bus).setPriority(0).setTransferTiming(VC::Timing(d,l));
        writeRoute->addHop(mem);

        //        readRoute->addHop(mem);
        //        readRoute->addHop(bus).setPriority(1).setTransferTiming(
        //            VC::Timing(d, l));
        //        readRoute->addHop(cpu1); // TODO: getComp(snk)

      } catch (std::exception & e) {
        std::cerr << "Caught exception wile configuring VPC:\n" << e.what()
            << std::endl;
        exit(-1);
      }

      std::cerr << " end of vpc configuration" << std::endl;
    }
#endif // SYSTEMOC_ENABLE_VPC
  }
private:
  Source source; // actors
  Sink sink;

  virtual void changeScheduling(){
#ifdef SYSTEMOC_ENABLE_VPC
    static size_t four_in_a_row = 0;

    namespace VC = SystemC_VPC::Config;
    std::list<SystemC_VPC::ScheduledTask*> priorityList;

    if(four_in_a_row%8 == 3){
      priorityList.push_back(&sink);
      priorityList.push_back(&source);
      VC::getComponent("CPU1")->getComponentInterface()->setDynamicPriority(
          priorityList);
      VC::getComponent("CPU1")->getComponentInterface()->scheduleAfterTransition();
    }else if(four_in_a_row%8 == 7){
      priorityList.push_back(&source);
      priorityList.push_back(&sink);
      VC::getComponent("CPU1")->getComponentInterface()->setDynamicPriority(
          priorityList);
      VC::getComponent("CPU1")->getComponentInterface()->scheduleAfterTransition();
   }

    ++four_in_a_row;
#endif // SYSTEMOC_ENABLE_VPC
  }
};

int sc_main(int argc, char **argv)
{
  bool vpcConfig = false;
  if (argc >= 2)
    vpcConfig = std::string("--vpc-api-config") == argv[1];

  NetworkGraph top("top", vpcConfig); // create network graph
  smoc_scheduler_top sched(top);

  try {
    sc_start(); // start simulation (SystemC)
  } catch (std::exception & e) {
    std::cerr << "Exception during simulation: " << e.what() << std::endl;
    exit(-1);
  }
  return 0;
}
