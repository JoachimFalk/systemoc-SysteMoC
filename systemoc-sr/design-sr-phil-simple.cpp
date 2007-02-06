// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_sr_signal.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif


//ADeVA Lib ??
template<typename T>
class Delay : public smoc_actor{
public:
  typedef T                  signal_t;

  smoc_port_in<signal_t>     in;
  smoc_port_out<signal_t>    out;

private:
  signal_t                   m_signal;
  bool                       undefined;

  void forward(){ // GO
    if(!undefined){
      //cout << name() << ".forward()" << endl;
      out[0] = m_signal;
    }
  }

  void store(){ // TICK
    if(in.isValid(0)){
      //cout << name() << ".store()" << endl;
      m_signal = in[0];
      undefined = false;
    } else {
      //undefined = true;
    }
  }

  smoc_firing_state main;
public:
  Delay(sc_module_name name)
    : smoc_actor(name, main),
      undefined(true){

    main = in(1)                                        >>
      out(1)                                            >>
      (SR_GO(Delay::forward) && SR_TICK(Delay::store) ) >> main;
  }
};



//pkg:
//enum    phil_type   {eating, right_fork_taken, thinking};
typedef unsigned short timer_type;
typedef unsigned short phil_type;
#define eating 0
#define right_fork_taken 1
#define thinking 2


class Clock: public smoc_actor {
public:
  smoc_port_out<bool> out1;
  smoc_port_out<bool> out2;
  smoc_port_out<bool> out3;
  smoc_port_out<bool> out4;
  smoc_port_out<bool> out5;
  smoc_port_out<bool> out6;
  smoc_port_out<bool> out7;
  smoc_port_out<bool> out8;
  smoc_port_out<bool> out9;
  smoc_port_out<bool> out10;
private:
  int limitCount;

  void pos() {
    cout << name() << ".pos()" << endl;
    limitCount++;
    out1[0] = 1;
    out2[0] = 1;
    out3[0] = 1;
    out4[0] = 1;
    out5[0] = 1;
    out6[0] = 1;
    out7[0] = 1;
    out8[0] = 1;
    out9[0] = 1;
    out10[0] = 1;
  }

  void neg(){
    cout << name() << ".neg()" << endl;
    out1[0] = 0;
    out2[0] = 0;
    out3[0] = 0;
    out4[0] = 0;
    out5[0] = 0;
    out6[0] = 0;
    out7[0] = 0;
    out8[0] = 0;
    out9[0] = 0;
    out10[0] = 0;
  }
  
  smoc_firing_state s_pos, s_neg;
public:
  Clock(sc_module_name name, SMOC_ACTOR_CPARAM( int, limit ))
    : smoc_actor(name, s_pos),  limitCount( 0 ) {

    s_pos = (VAR(limitCount)<limit) >>
      ( out1(1) && out2(1) && out3(1) && out4(1) && out5(1)
      && out6(1) && out7(1) && out8(1) && out9(1) && out10(1) ) >>
      CALL(Clock::pos) >>
      s_neg;

    s_neg = 
      ( out1(1) && out2(1) && out3(1) && out4(1) && out5(1)
      && out6(1) && out7(1) && out8(1) && out9(1) && out10(1) ) >>
      CALL(Clock::neg) >>
      s_pos;
  }
};


class Philosopher: public smoc_actor {
public:
  smoc_port_in<bool>            clk;
  smoc_port_in<timer_type>      timer;
  smoc_port_in<phil_type>       left_phil;
  smoc_port_in<phil_type>       right_phil;
  smoc_port_out<phil_type>      phil1;
  smoc_port_out<phil_type>      phil2;
  smoc_port_out<phil_type>      phil3;
private:
  bool c1() const{
    //TODO: edge detection
    return clk[0];
  }

  bool c2() const{
    return right_phil[0] == thinking;
  }

  bool c3() const{
    return left_phil[0] == thinking;
  }

  bool c4() const{
    return timer[0]>=4;
  }

  bool c5() const{
    return timer[0]==8;
  }

  void think(){
    cout << name() << ".think()" << endl;
    phil1[0] = thinking;
    phil2[0] = thinking;
    phil3[0] = thinking;
  }

  void take_right_fork(){
    cout << name() << ".right_fork_taken()" << endl;
    phil1[0] = right_fork_taken;
    phil2[0] = right_fork_taken;
    phil3[0] = right_fork_taken;
  }

  void eat(){
    cout << name() << ".eat()" << endl;
    phil1[0] = eating;
    phil2[0] = eating;
    phil3[0] = eating;
  }
  smoc_firing_state s_thinking, s_right_fork_taken, s_eating;
  smoc_firing_state init;
public:
  Philosopher(sc_module_name name)
    : smoc_actor(name, init){

    init = (phil1(1) && phil2(1) && phil3(1))                                       >>
      CALL(Philosopher::think)                                                      >>
      s_thinking;

    s_thinking = (clk(1) && timer(1) && right_phil(1) && GUARD(Philosopher::c1)     &&
		  GUARD(Philosopher::c2) && GUARD(Philosopher::c5) )                >>
      (phil1(1) && phil2(1) && phil3(1))                                            >>
      CALL(Philosopher::take_right_fork)                                            >>
      s_right_fork_taken;
    
    s_right_fork_taken = (clk(1) && left_phil(1) && GUARD(Philosopher::c1)          &&
		  GUARD(Philosopher::c3) )                                          >>
      (phil1(1) && phil2(1) && phil3(1))                                            >>
      CALL(Philosopher::eat)                                                        >>
      s_eating;

    s_eating = (clk(1) && timer(1) && GUARD(Philosopher::c1)                        &&
		  GUARD(Philosopher::c4) )                                          >>
      (phil1(1) && phil2(1) && phil3(1))                                            >>
      CALL(Philosopher::think)                                                      >>
      s_thinking;

  }
};

class Timer
  : public smoc_actor{
public:
  smoc_port_in<bool>            clk;
  smoc_port_in<phil_type>       phil;
  smoc_port_out<timer_type>     timer;
  
private:
  timer_type m_timer;

  bool c1() const{
    //TODO: edge detection
    return clk[0];
  }

  bool c2() const{
    return phil[0] == right_fork_taken;
  }

  bool c3() const{
    return phil[0] == eating;
  }

  bool c4() const{
    return m_timer<8;
  }

  void add(){
    cout << name() << ".add()" << endl;
    timer[0] = ++m_timer;
  }

  void zero(){
    cout << name() << ".zero()" << endl;
    timer[0] = m_timer = 0;
  }

  smoc_firing_state main;
public:
  Timer(sc_module_name name)
    :smoc_actor(name, main), m_timer(0){
    main = (clk(1) && phil(1) && GUARD(Timer::c1) && GUARD(Timer::c2) && !GUARD(Timer::c3) && !GUARD(Timer::c4)) >>
      timer(1)                                        >>
      CALL(Timer::zero)                               >>
      main

      | (clk(1) && phil(1) && GUARD(Timer::c1) && !GUARD(Timer::c2) && GUARD(Timer::c3) && GUARD(Timer::c4)) >>
      timer(1)                                        >>
      CALL(Timer::add)                                >>
      main

      | (clk(1) && phil(1) && GUARD(Timer::c1) && !GUARD(Timer::c2) && !GUARD(Timer::c3) && GUARD(Timer::c4)) >>
      timer(1)                                        >>
      CALL(Timer::add)                                >>
      main;

  }
};

class PhilosopherTestBench
  :public smoc_graph{
protected:
  Philosopher phil1;
  Philosopher phil2;
  Philosopher phil3;
  Philosopher phil4;
  Philosopher phil5;
  Timer       timer1;
  Timer       timer2;
  Timer       timer3;
  Timer       timer4;
  Timer       timer5;
  Clock       clk;


private:
  template <typename T_chan_init>
  void connect(
      smoc_port_out<typename T_chan_init::data_type> &b,
      smoc_port_in<typename T_chan_init::data_type>  &a,
      const T_chan_init i 
      //L l, R r, C c
      ){
    Delay<typename T_chan_init::data_type> *delay = new Delay<typename T_chan_init::data_type>("Delay");
    connectNodePorts(b,          delay->in, i);
    connectNodePorts(delay->out, a,         T_chan_init(i));
  }


public:
  PhilosopherTestBench(sc_module_name name, int times)
    :smoc_graph(name),
     phil1("Phil1"),
     phil2("Phil2"),
     phil3("Phil3"),
     phil4("Phil4"),
     phil5("Phil5"),
     timer1("Timer1"),
     timer2("Timer2"),
     timer3("Timer3"),
     timer4("Timer4"),
     timer5("Timer5"),
     clk("CLK", times){
    connect( phil5.phil1,   phil4.right_phil,    smoc_sr_signal<phil_type>() );
    connect( phil4.phil1,   phil3.right_phil,    smoc_sr_signal<phil_type>() );
    connect( phil3.phil1,   phil2.right_phil,    smoc_sr_signal<phil_type>() );
    connect( phil2.phil1,   phil1.right_phil,    smoc_sr_signal<phil_type>() );
    connect( phil1.phil1,   phil5.right_phil,    smoc_sr_signal<phil_type>() );

    connect( phil5.phil2,   phil1.left_phil,    smoc_sr_signal<phil_type>() );
    connect( phil4.phil2,   phil5.left_phil,    smoc_sr_signal<phil_type>() );
    connect( phil3.phil2,   phil4.left_phil,    smoc_sr_signal<phil_type>() );
    connect( phil2.phil2,   phil3.left_phil,    smoc_sr_signal<phil_type>() );
    connect( phil1.phil2,   phil2.left_phil,    smoc_sr_signal<phil_type>() );

    connect( timer5.timer,  phil5.timer,        smoc_sr_signal<phil_type>() );
    connect( timer4.timer,  phil4.timer,        smoc_sr_signal<phil_type>() );
    connect( timer3.timer,  phil3.timer,        smoc_sr_signal<phil_type>() );
    connect( timer2.timer,  phil2.timer,        smoc_sr_signal<phil_type>() );
    connect( timer1.timer,  phil1.timer,        smoc_sr_signal<phil_type>() );

    connect( phil5.phil3,   timer5.phil,        smoc_sr_signal<phil_type>() );
    connect( phil4.phil3,   timer4.phil,        smoc_sr_signal<phil_type>() );
    connect( phil3.phil3,   timer3.phil,        smoc_sr_signal<phil_type>() );
    connect( phil2.phil3,   timer2.phil,        smoc_sr_signal<phil_type>() );
    connect( phil1.phil3,   timer1.phil,        smoc_sr_signal<phil_type>() );

    connect( clk.out5,       timer5.clk,        smoc_sr_signal<bool>()      );
    connect( clk.out4,       timer4.clk,        smoc_sr_signal<bool>()      );
    connect( clk.out3,       timer3.clk,        smoc_sr_signal<bool>()      );
    connect( clk.out2,       timer2.clk,        smoc_sr_signal<bool>()      );
    connect( clk.out1,       timer1.clk,        smoc_sr_signal<bool>()      );

    connect( clk.out10,      phil5.clk,         smoc_sr_signal<bool>()      );
    connect( clk.out9,       phil4.clk,         smoc_sr_signal<bool>()      );
    connect( clk.out8,       phil3.clk,         smoc_sr_signal<bool>()      );
    connect( clk.out7,       phil2.clk,         smoc_sr_signal<bool>()      );
    connect( clk.out6,       phil1.clk,         smoc_sr_signal<bool>()      );

  }
};
 

int sc_main (int argc, char **argv) {
  size_t count = (argc>1?atoi(argv[1]):0);
  smoc_top_moc<PhilosopherTestBench> nsa_tb("top", count);
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, nsa_tb);
  } else {
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}
