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
#endif // SYSTEMOC_ENABLE_VPC

static const std::string MESSAGE_HELLO = "Hello SysteMoC!";

class Source : public smoc_actor
{
public:
  smoc_port_out<char> out;

  Source(sc_core::sc_module_name name) :
    smoc_actor(name, start), count(0), size(MESSAGE_HELLO.size()), message(
        MESSAGE_HELLO)
  {
    start = GUARD(Source::hasToken) >> out(1) >> CALL(Source::source) >> start;
  }

private:
  smoc_firing_state start;

  unsigned int count, size; // variables (functional state)
  const std::string message; //

  bool
  hasToken() const
  {
    return count < size;
  } // guard
  void
  source()
  {
    std::cerr << this->name() << "> @ " << sc_core::sc_time_stamp() << "\tsend: \'"
        << message[count] << "\'" << std::endl;
    out[0] = message[count++];
  } // action
};

class Sink : public smoc_actor
{
public:
  // ports:
  smoc_port_in<char> in;

  Sink(sc_core::sc_module_name name) // actor constructor
  :
    smoc_actor(name, start)
  {
    // FSM definition:
    start = in(1) >> CALL(Sink::sink) >> start;
  }
private:
  smoc_firing_state start; // FSM states

  void
  sink()
  {
    std::cout << this->name() << "> @ " << sc_core::sc_time_stamp() << "\trecv: \'"
        << in[0] << "\'" << std::endl;
  }
};

class NetworkGraph : public smoc_graph
{
public:
  NetworkGraph(sc_core::sc_module_name name) // network graph constructor
  :
    smoc_graph(name), source("Source"), // create actors
        sink("Sink")
  {
    smoc_fifo<char> fifo("queue", 4);
    fifo.connect(source.out).connect(sink.in); // connect actors

#ifdef SYSTEMOC_ENABLE_VPC
    try {
      std::cerr << " start vpc configuration" << std::endl;

      // convenience
      namespace VC = SystemC_VPC::Config;

      // configuration takes place during elaboration phase
      // the configuration may not change after elaboration
      // TODO: we should assert this in the configuration API

      //create components
      VC::Component::Ptr cpu = VC::createComponent("CPU", VC::Scheduler::StaticPriority_NP);

      // configuration
      cpu->setScheduler(VC::Scheduler::FCFS);

      // mappings (cannot change after elaboration TODO: assert this)
      cpu->addTask(source);
      cpu->addTask(sink);

      // set priorities etc
      VC::setPriority(source, 0);
      VC::setPriority(sink, 1);

      // timings
      VC::DefaultTimingsProvider::Ptr provider = cpu->getDefaultTimingsProvider();

      provider->add(VC::Timing("Source::source", sc_core::sc_time(10, sc_core::SC_NS),
          sc_core::sc_time(10, sc_core::SC_NS))); // dii, latency
      provider->add(VC::Timing("Sink::sink", sc_core::sc_time(10, sc_core::SC_NS))); // delay
      provider->add(VC::Timing("Source::source", sc_core::sc_time(100, sc_core::SC_NS)));
      provider->add(VC::Timing("Sink::sink", sc_core::sc_time(100, sc_core::SC_NS)));


      // configure routing
      VC::ignoreMissingRoutes(true);

      VC::Component::Ptr bus = VC::createComponent("Bus");
      VC::Component::Ptr mem = VC::createComponent("Memory");

      VC::Timing transfer(sc_core::sc_time(20,sc_core::SC_NS), sc_core::sc_time(20,sc_core::SC_NS));
      bus->setTransferTiming(transfer);
      mem->setTransferTiming(transfer);
      cpu->setTransferTiming(transfer);

//    VC::Route::Ptr writeRoute = VC::createRoute(getLeafPort(&source.out));
//    VC::Route::Ptr readRoute = VC::createRoute(getLeafPort(&sink.in));
      VC::Route::Ptr writeRoute = VC::createRoute(&source.out);
      VC::Route::Ptr readRoute = VC::createRoute(&sink.in);

      sc_core::sc_time d(10, sc_core::SC_NS);
      sc_core::sc_time l(20, sc_core::SC_NS);

      writeRoute->addHop(cpu);
      writeRoute->addHop(bus).setPriority(0).setTransferTiming(VC::Timing(d,l));
      writeRoute->addHop(mem);

      readRoute->addHop(mem);
      readRoute->addHop(bus);
      readRoute->addHop(cpu);
    } catch (std::exception & e) {
      std::cerr << "Caught exception wile configuring VPC:\n"
          << e.what() << std::endl;
      exit(-1);
    }

    std::cerr << " end of vpc configuration" << std::endl;
#endif // SYSTEMOC_ENABLE_VPC

  }
private:
  Source source; // actors
  Sink sink;
};

int
sc_main(int argc, char **argv)
{
  NetworkGraph top("top"); // create network graph
  smoc_scheduler_top sched(top);

  sc_core::sc_start(); // start simulation (SystemC)
  return 0;
}
