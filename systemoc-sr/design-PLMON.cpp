#include <smoc_moc.hpp>
#include <smoc_sr_signal.hpp>

#include <adeva_lib.hpp>

// data types:
enum    plmon_type   {e_monitor, e_latch, e_unequipped};
typedef unsigned short data_type;

class AIS: public smoc_actor {
public:
  smoc_port_out<bool>           out;
  smoc_firing_state             init;
public:
  AIS(sc_module_name name)
    : smoc_actor(name, init){}
};

//the PLMON MTT as SysteMoC actor
class PLMON: public smoc_actor {
public:
  //input ports
  smoc_port_in<bool>            AIS;
  smoc_port_in<bool>            AIS_hist;
  smoc_port_in<bool>            CPUread;
  smoc_port_in<bool>            CPUread_hist;
  smoc_port_in<data_type>       in;
  smoc_port_in<data_type>       in_hist;
  smoc_port_in<data_type>       last;
  smoc_port_in<data_type>       last_hist;

  //output port
  smoc_port_out<plmon_type>     out;
private:
  bool c1() const{
    return AIS[0]==false;
  }

  bool c2() const{
    return in[0] == 0x00;
  }

  bool c2_event() const{
    bool hist   = in_hist[0] == 0x00;
    bool actual = in[0]      == 0x00;
    return hist != actual;
  }

  bool c3() const{
    return in[0] != last[0];
  }
  bool c3_event() const{
    bool hist   = in_hist[0] != last_hist[0];
    bool actual = in[0]      != last[0];
    return hist != actual;
  }

  bool c4() const{
    return CPUread[0];
  }

  bool c4_event() const{
    bool hist   = CPUread_hist[0];
    bool actual = CPUread[0];
    return hist != actual;
  }

  void unequipped(){
    out[0] = e_unequipped;
  }

  void latch(){
    out[0] = e_latch;
  }

  void monitor(){
    out[0] = e_monitor;
  }

  smoc_firing_state s_monitor, s_latch, s_unequipped;
  smoc_firing_state init;
public:
  PLMON(sc_module_name name)
    : smoc_actor(name, init){

    init = out(1) >> CALL(PLMON::monitor) >> s_monitor;

    s_monitor = ( AIS(1) && in(1) && in_hist(1) && !GUARD(PLMON::c1) &&
		  GUARD(PLMON::c2) && GUARD(PLMON::c2_event) )       >>
      out(1)                                                         >>
      CALL(PLMON::unequipped)                                        >>
      s_unequipped
    |           ( AIS(1) && in(1) && in_hist(1) && !GUARD(PLMON::c1) &&
		  !GUARD(PLMON::c2) && GUARD(PLMON::c3)              &&
		  GUARD(PLMON::c3_event) )                           >>
      out(1)                                                         >>
      CALL(PLMON::latch)                                             >>
      s_latch;

    s_latch   = ( AIS(1) && in(1) && in_hist(1) && !GUARD(PLMON::c1) &&
		  GUARD(PLMON::c2) && GUARD(PLMON::c2_event) )       >>
      out(1)                                                         >>
      CALL(PLMON::unequipped)                                        >>
      s_unequipped
    |           ( CPUread(1) && CPUread_hist(1) &&
		  GUARD(PLMON::c4) && GUARD(PLMON::c4_event) )       >>
      out(1)                                                         >>
      CALL(PLMON::monitor)                                           >>
      s_monitor;

    s_unequipped = ( AIS(1) && in(1) && in_hist(1) && 
		     !GUARD(PLMON::c1) && !GUARD(PLMON::c2) &&
		     GUARD(PLMON::c2_event) )       >>
      out(1)                                                         >>
      CALL(PLMON::monitor)                                        >>
      s_monitor;

    

  }
};


class PLMONTestBench
  :public smoc_graph{
protected:
  AIS   m_AIS;
  //...
  PLMON m_PLMON;
private:
  template <typename T_chan_init>
  void connect(
      smoc_port_out<typename T_chan_init::data_type> &b,
      smoc_port_in<typename T_chan_init::data_type>  &a,
      smoc_port_in<typename T_chan_init::data_type>  &a_hist,
      const T_chan_init i 
      //L l, R r, C c
      ){
    Delay<typename T_chan_init::data_type> *delay = new Delay<typename T_chan_init::data_type>("Delay");
    connectNodePorts(b,              delay->in, i);
    connectNodePorts(delay->out,     a,         T_chan_init(i));
    connectNodePorts(delay->history, a_hist,    T_chan_init(i));
  }

public:
  PLMONTestBench(sc_module_name name, int times)
    : smoc_graph(name),
      m_AIS("AIS"),
      //...
      m_PLMON("PLMON"){
    connect( m_AIS.out, m_PLMON.AIS, m_PLMON.AIS_hist ,smoc_sr_signal<bool>() );
    //...
  }
};
 

int sc_main (int argc, char **argv) {
  size_t count = (argc>1?atoi(argv[1]):0);
  smoc_top_sr_moc<PLMONTestBench> nsa_tb("top", count);
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, nsa_tb);
  } else {
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}
