// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

static const char message [] = "Hello SysteMoC!";

class Source: public smoc_actor {
public:
  enum Type {FUNCTION_GUARD, INLINE_GUARD};

  // ports:
  smoc_port_out<char> out;
private:
  // functionality state:
  unsigned int count;
  unsigned int size;

  // guards:
  bool hasToken() const{
    return count<size;
  }

  // actions:
  void src() {
    //std::cout << this->name() << " send: \""
    //          << message[count] << "\"" << std::endl;
    out[0] = message[count];
    ++count;
  }

  // FSM states:
  smoc_firing_state start;
public:
  // actor constructor
  Source(sc_core::sc_module_name name, Type type = INLINE_GUARD)
    : smoc_actor(name, start),
      count(0),
      size(sizeof(message))
  {

    // FSM definition:
    start = 
      GUARD(Source::hasToken)  >>
      out(1)                   >>
      CALL(Source::src)        >> start
      ;
  }
};


class TypedSource: public smoc_actor {
public:
  enum Type {FUNCTION_GUARD, INLINE_GUARD};

  // ports:
  smoc_port_out<char> out;
private:
  // functionality state:
  unsigned int count;
  unsigned int size;

  // guards:
  bool hasToken() const{
    return count<size;
  }

  // actions:
  void src() {
    std::cout << this->name() << " send: \""
              << message[count] << "\"" << std::endl;
    out[0] = message[count];
    ++count;
  }

  // FSM states:
  smoc_firing_state start;
public:
  // actor constructor
  TypedSource(sc_core::sc_module_name name, Type type)
    : smoc_actor(name, start),
      count(0),
      size(sizeof(message))
  {

    switch(type){
    default:
    case FUNCTION_GUARD:
      // FSM definition:
      start = 
        (out(1)                          &&
         GUARD(TypedSource::hasToken))   >>
        CALL(TypedSource::src)           >> start
        ;
      break;
    case INLINE_GUARD:
      // equivalent FSM using other syntax:
      start = 
        //        VAR(count)<VAR(size)     >>
        (out(1) && VAR(count)<VAR(size))   >>
        CALL(TypedSource::src) >> start
        ;
      break;

    }
  }
};


class Sink: public smoc_actor {
public:
  // ports:
  smoc_port_in<char> in;
private:
  // actions:
  void sink() {
    std::cout << this->name() << " recv: \""
              << in[0] << "\"" << std::endl;
  }

  // FSM states:
  smoc_firing_state start;
public:
  // actor constructor
  Sink(sc_core::sc_module_name name)
    : smoc_actor(name, start)
  {

    // FSM definition:
    start =
      in(1)                 >>
      CALL(Sink::sink) >> start
      ;
  }
};



class NetworkGraph: public smoc_graph {
protected:
  // actors
  Sink           sink;
public:
  // networkgraph constructor
  NetworkGraph(sc_core::sc_module_name name, const unsigned int actor)
    : smoc_graph(name),
      // create actors
      sink("Sink")
  {
    if(actor == 0){
      TypedSource *t = new TypedSource("TypedSource_IG",
                                      TypedSource::INLINE_GUARD);
      connectNodePorts(t->out, sink.in);
    }else if(actor == 1){
      TypedSource *t = new TypedSource("TypedSource_FG",
                                      TypedSource::FUNCTION_GUARD);
      connectNodePorts(t->out, sink.in);
    }else{
      Source *s = new Source("Source");
      connectNodePorts(s->out, sink.in);
    }

  }
};

int sc_main (int argc, char **argv) {
  int type = 0;
  if (argc >= 2) {
    type = atoi(argv[1]);
  }  
  // create networkgraph
  smoc_top_moc<NetworkGraph> top("top", type);

  // start simulation (SystemC)
  sc_core::sc_start();
  return 0;
}
