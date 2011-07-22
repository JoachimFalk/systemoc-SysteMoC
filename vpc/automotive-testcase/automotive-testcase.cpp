// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_tt.hpp>
#include <systemoc/smoc_register.hpp>
/*
#include "src.hpp"
#include "snk.hpp"
*/

#include "audioSink.hpp"
#include "audioSend.hpp"
#include "sendTask.hpp"
#include "receiveTask.hpp"
#include "simple_actor_tt.hpp"
#include "simple_src.hpp"
#include "simple_sink.hpp"
//#include "ActorDisabler.hpp"
#include <CoSupport/Tracing/TracingFactory.hpp>

class TTGraph
  :public smoc_graph_tt{
public:
  TTGraph(sc_module_name name)
    :smoc_graph_tt(name),
     csc1("Producer_44,1kHz", sc_time(22675, SC_NS), sc_time(0, SC_MS),"a25"),
     act("Player_44,1kHz",sc_time(22675, SC_NS), sc_time(0, SC_MS)),
     task1("Send_1722", sc_time(125, SC_US), sc_time(125, SC_US)),
     task2("Receive_1722", "id_in5", "id_in6", "id_out"),
     traffic1("Traffic1", sc_time(228.8, SC_MS), sc_time(1, SC_US)),
     traffic2("Traffic2", sc_time(460, SC_US), sc_time(0, SC_US), 0.5),
     traffic3_send("Traffic3_send", sc_time(24, SC_US), sc_time(22, SC_US)),
     traffic3_consume("Traffic3_consume", "traff_consume")
  {

smoc_fifo<int> ch1("channel1", 10);
smoc_fifo<int>  ch2_1("channel2_1", 10), ch2_2("channel2_2", 10);
smoc_fifo<int> ch3("channel3", 10);
smoc_fifo<int> chTraffic3("channelTraffic3",10);
ch1.connect(csc1.out).connect(task1.in);
ch2_1.connect(task1.out5).connect(task2.in5);
ch2_2.connect(task1.out6).connect(task2.in6);
ch3.connect(task2.out).connect(act.in);
chTraffic3.connect(traffic3_send.out).connect(traffic3_consume.in);
  }

protected:
  AudioSend csc1;
  AudioSink act;
  SendTask task1;
  ReceiveTask task2;

  SimpleActor_tt traffic1;
  SimpleActor_tt traffic2;
//  SimpleActor_tt traffic3;
  SimpleSrc traffic3_send;
  SimpleSink traffic3_consume;
};

int sc_main (int argc, char **argv) {
  size_t runtime = (argc>1?atoi(argv[1]):0);

  CoSupport::Tracing::TracingFactory::getInstance().setTraceFile("tracing.log");
  TTGraph tt_Test("TTGraph");
//  ActorDisabler disabler1(&tt_Test);
  smoc_scheduler_top top(tt_Test);

  sc_start(sc_time(runtime,SC_MS));
  return 0;
}
