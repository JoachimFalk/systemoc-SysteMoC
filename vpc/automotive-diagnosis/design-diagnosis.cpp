// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_tt.hpp>
#include <systemoc/smoc_register.hpp>
/*
#include "src.hpp"
#include "snk.hpp"
*/
#include "src_neu.hpp"
#include "snk_neu.hpp"
#include "simple_sink.hpp"
#include "simple_sink_2.hpp"
#include "simple_src_tt.hpp"
#include "simple_src_tt_2.hpp"
#include "simple_sink_tt.hpp"
#include "simple_Task4_2.hpp"
#include "simple_Task2_1.hpp"
#include "simple_Task3_1.hpp"
#include "simple_Task1_2.hpp"
#include "simple_Task2_2.hpp"
#include "simple_Task.hpp"
#include "simple_Task_tt.hpp"
#include "simple_sink2.hpp"
#include "ActorDisabler.hpp"
#include <CoSupport/Tracing/TracingFactory.hpp>

#define PERFORMANCE_EVALUATION

#ifdef PERFORMANCE_EVALUATION
//#  include <CoSupport/SystemC/PerformanceEvaluation.hpp>
//#  include <CoSupport/Tracing/TracingFactory.hpp>
#endif // PERFORMANCE_EVALUATION

class TTGraph
  :public smoc_graph_tt{
public:
  TTGraph(sc_module_name name)
    :smoc_graph_tt(name),
     csc1("CameraSensorComponent1_a89", sc_time(5, SC_MS), sc_time(1, SC_MS), 0.0, "a90"),
     csc2("CameraSensorComponent2_a73", sc_time(5, SC_MS), sc_time(1, SC_MS), 0.0,"a74", true, "sensor_trace"),
     csc3("CameraSensorComponent3_a57", sc_time(5, SC_MS), sc_time(1, SC_MS), 0.0,"a58"),
     csc4("CameraSensorComponent4_a24", sc_time(5, SC_MS), sc_time(1, SC_MS), 0.0,"a25"),
     hd1("HDCam1_a2634", sc_time(40, SC_MS), sc_time(1, SC_MS), 0.0,"a2635"),
     hd2("HDCam2_a2662", sc_time(40, SC_MS), sc_time(1, SC_MS), 0.0,"a2663"),
     tr1("TriggerRadar1_a415", sc_time(5, SC_MS), sc_time(1, SC_MS), 0.0,"a416", true, "trace2"),
     tr2("TriggerRadar2_a431", sc_time(5, SC_MS), sc_time(1, SC_MS), 0.0,"a432"),
     sensor("Sensor_a447", sc_time(5, SC_MS), sc_time(1, SC_MS), 0.0,"a448"),
     diagnose("Diagnose_a941", sc_time(5, SC_MS), sc_time(1, SC_MS), 0.0,"a942"),
     turn01("Turn01_a973", sc_time(5, SC_MS), sc_time(1, SC_MS), 0.0,"a974", true, "turn01_trace"),
     turn02("Turn02_a957", sc_time(5, SC_MS), sc_time(1, SC_MS), 0.0,"a958"),
     iprep1("ImagePreprocessing1_a169","a170","a171"),
     iprep2("ImagePreprocessing2_a148","a149","a150"),
     iprep3("ImagePreprocessing3_a127","a128","a129"),
     iprep4("ImagePreprocessing4_a106","a107","a108"),
     radar1("Radar1_a464","a465","a466"),
     radar2("Radar2_a485","a486","a487"),
     ws("WheelSensor_a506","a507","a508","a1324"),
     turnInd("TurnIndLt_a990","a991","a992"),
     ef1("ef1_a1011","a1012","a1013"),
     ef2("ef2_a1032","a1033","a1034"),
     ipostp1("ImagePostProcessing1_a318","a319","a320"),
     ipostp2("ImagePostProcessing2_a297","a298","a299"),
     od1("ObjectDetection1_a545","a546","a547"),
     albs("AntilockBreak_a599","a600","a601"),
     pe("PathEstimation_a620","a1941","a622"),
     od2("ObjectDetection2_a566","a567","a568"),
     os("ObjectSelection_a716","a717","a718"),
     bc("BreakControl_a840","a841","a842"),
     tc("ThrottleControl_a861","a862","a863"),
     slfog("SLFogLmp_a1134","a1135","a1136"),
     fogft("FogLmpFrAct_a1161","a1162","a1163"),
     idc("ImageDataCollector_a226","a227","a228","a229","a230","a231","a232"),
     df("DataFusion_a669","a670","a687","a671"),
     hdproc("HDprocessing_a2383","a2384","a2385","a2386"),
     acc("AdaptiveCruiseControl_a745","a746","a811","a747"),
     tbta("ThrottleBreakTorqueArbitration_a776","a777","a778","a779","a780"),
     fogl("FogLmp_a1079","a1080","a1081","a1082","a1083"),
     vac2("VideoActuatorComponent2_a367","a368"),
     vac1("VideoActuatorComponent1_a383","a384", "sensor_trace"),
     foglOn("FogLmpOn_a1191","a1192", "turn01_trace"),
     hdvidact("HDVidAct_a2498","a2499"),
     act("Actuators_a905","a906","a907", "trace2"),
     st1("simpleTask1_a3210", sc_time(3, SC_MS), SC_ZERO_TIME),
     st2("simpleTask2_a3669", sc_time(4, SC_MS), SC_ZERO_TIME),
     restbus1send("Restbus1send_a3703", sc_time(4, SC_MS), sc_time(2, SC_MS), "a3704", 0.1),
     restbus1receive("Restbus1receive_a3719", "a3720")
  {

smoc_fifo<sc_time> ib1("a187",2), ib2("a195",1), ib3("a203",1), ib4("a211",1);
ib1.connect(csc4.out).connect(iprep4.in);
ib2.connect(csc3.out).connect(iprep3.in);
ib3.connect(csc2.out).connect(iprep2.in);
ib4.connect(csc1.out).connect(iprep1.in);

smoc_fifo<sc_time> hd_chan1("a2678", 5000), hd_chan2("a2685", 5000), hd_chan3("a2512", 5000);
hd_chan1.connect(hd1.out).connect(hdproc.in1);
hd_chan2.connect(hd2.out).connect(hdproc.in2);
hd_chan3.connect(hdproc.out).connect(hdvidact.in);

smoc_fifo<sc_time> im1("a285"), im2("a272"), im32("a266"), im42("a260");
im1.connect(iprep1.out).connect(idc.in4);
im2.connect(iprep2.out).connect(idc.in3);
im32.connect(iprep3.out).connect(idc.in2);
im42.connect(iprep4.out).connect(idc.in1);

smoc_fifo<sc_time> im3("a348"), im4("a354"), ib3_8("a397"), ib4_8("a405");
im3.connect(idc.out1).connect(ipostp1.in);
im4.connect(idc.out2).connect(ipostp2.in);
ib4_8.connect(ipostp2.out).connect(vac2.in);
ib3_8.connect(ipostp1.out).connect(vac1.in);

smoc_fifo<sc_time> alpha1("a524"), alpha2("a536"), alpha3("a530");
alpha1.connect(tr1.out).connect(radar1.in);
alpha2.connect(tr2.out).connect(radar2.in);
alpha3.connect(sensor.out).connect(ws.in);

smoc_fifo<sc_time> m11("a584"), m12("a590");
m11.connect(radar1.out).connect(od1.in);
m12.connect(radar2.out).connect(od2.in);

//smoc_register<sc_time> m3, m7;
smoc_fifo<sc_time> m3("a638"), m7("a894"), m3_2("a1956"), m8("a887");
m3.connect(ws.out1).connect(albs.in);
m7.connect(tbta.out2).connect(bc.in);
m3_2.connect(ws.out2).connect(pe.in); //FIXME m3_2 is not part of the DSE-model
m8.connect(tbta.out1).connect(tc.in);

smoc_fifo<sc_time> m1("a698"), m2("a704"), m10("a826"), m4("a816");
m1.connect(od1.out).connect(df.in2);
m2.connect(od2.out).connect(df.in1);
m10.connect(albs.out).connect(tbta.in2);
m4.connect(pe.out).connect(acc.in2);

smoc_fifo<sc_time> m5("a734"), m9("a763", 20), m6("a802"), alphaa("a923"), alphab("a931"); // m8("a") ?  TODO
m5.connect(df.out).connect(os.in);
m9.connect(os.out).connect(acc.in1);
m6.connect(acc.out).connect(tbta.in1);
alphaa.connect(bc.out).connect(act.in2);
alphab.connect(tc.out).connect(act.in1);

smoc_fifo<sc_time> dma("a1050"), t02("a1056"), t01("a1062");
dma.connect(diagnose.out).connect(turnInd.in);
t02.connect(turn02.out).connect(ef1.in);
t01.connect(turn01.out).connect(ef2.in);

smoc_fifo<sc_time> dti("a1105"), ef("a1114"), efrq("a1123");
dti.connect(turnInd.out).connect(fogl.in3);
ef.connect(ef1.out).connect(fogl.in2);
efrq.connect(ef2.out).connect(fogl.in1);

smoc_fifo<sc_time> foglmp("a1152"), slfog_c("a1179"), foglmpact("a1205");
foglmp.connect(fogl.out).connect(slfog.in);
slfog_c.connect(slfog.out).connect(fogft.in);
foglmpact.connect(fogft.out).connect(foglOn.in);

smoc_fifo<sc_time> restbus1channel("a3733", 10);
restbus1channel.connect(restbus1send.out).connect(restbus1receive.in);

/*#ifdef PERFORMANCE_EVALUATION
      CoSupport::SystemC::PerformanceEvaluation::getInstance().startUnit();
#endif // PERFORMANCE_EVALUATION
*/
  }

protected:
  SimpleSource_tt  csc1, csc2, csc3, csc4;
  SimpleSource_tt_hd hd1, hd2;
  SimpleSource_tt  tr1, tr2, sensor;
  SimpleSource_tt  diagnose, turn01, turn02;
  SimpleTask  iprep1, iprep2, iprep3, iprep4;
  SimpleTask  radar1, radar2;
  SimpleTask1_2 ws;
  SimpleTask  turnInd, ef1, ef2;

  SimpleTask  ipostp1, ipostp2;
  SimpleTask  od1, albs, pe, od2;
  SimpleTask   bc, tc;
  SimpleTask os;
  SimpleTask  slfog, fogft;

  SimpleTask4_2 idc;
  SimpleTask2_1 acc;
  SimpleTask2_1 df;
  SimpleTask2_1_hd hdproc;
  SimpleTask2_2 tbta;
  SimpleTask3_1 fogl;

  SimpleSink vac2, vac1;
  SimpleSink foglOn;
  SimpleSink_hd hdvidact;
  SimpleSink2 act;

  SimpleTask_tt st1, st2;
  SimpleSource_tt_2  restbus1send;
  SimpleSink_2 restbus1receive;
};

int sc_main (int argc, char **argv) {
  size_t runtime = (argc>1?atoi(argv[1]):0);

  CoSupport::Tracing::TracingFactory::getInstance().setTraceFile("tracing.log");
  TTGraph tt_Test("TTGraph");
  //ActorDisabler disabler1(&tt_Test);
  smoc_scheduler_top top(tt_Test);

  sc_start(sc_time(runtime,SC_MS));
  return 0;
}
