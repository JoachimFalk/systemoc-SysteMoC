#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>

#include <adeva_lib.hpp>

//pkg:
enum    phil_type   {eating, right_fork_taken, thinking};
typedef unsigned short timer_type;

typedef smoc_multicast_sr_signal<phil_type>  PhilSignal;
typedef smoc_multicast_sr_signal<timer_type> TimerSignal;
typedef smoc_multicast_sr_signal<bool>       BoolSignal;


class Clock: public smoc_actor {
public:
  smoc_port_out<bool> out;
private:
  int limitCount;

  void pos() {
    cout << name() << ".pos()" << endl;
    limitCount++;
    out[0] = 1;
  }

  void neg(){
    cout << name() << ".neg()" << endl;
    out[0] = 0;
  }
  
  smoc_firing_state s_pos, s_neg;
public:
  Clock(sc_module_name name, int limit )
    : smoc_actor(name, s_pos),  limitCount( 0 ) {
    SMOC_REGISTER_CPARAM(limit);

    s_pos = (VAR(limitCount)<limit) >>
      ( out(1) )                    >>
      CALL(Clock::pos)              >>
      s_neg;

    s_neg = 
      ( out(1) )       >>
      CALL(Clock::neg) >>
      s_pos;
  }
};


class Philosopher: public smoc_actor {
public:
  smoc_port_in<bool>            clk;
  smoc_port_in<bool>            clk_hist;
  smoc_port_in<timer_type>      timer;
  smoc_port_in<timer_type>      timer_hist;
  smoc_port_in<phil_type>       left_phil;
  smoc_port_in<phil_type>       left_phil_hist;
  smoc_port_in<phil_type>       right_phil;
  smoc_port_in<phil_type>       right_phil_hist;
  smoc_port_out<phil_type>      phil;
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
    phil[0] = thinking;
  }

  void take_right_fork(){
    cout << name() << ".right_fork_taken()" << endl;
    phil[0] = right_fork_taken;
  }

  void eat(){
    cout << name() << ".eat()" << endl;
    phil[0] = eating;
  }
  smoc_firing_state s_thinking, s_right_fork_taken, s_eating;
  smoc_firing_state init;
public:
  Philosopher(sc_module_name name)
    : smoc_actor(name, init){

    init = ( phil(1) )                                       >>
      CALL(Philosopher::think)                                >>
      s_thinking;

    s_thinking =
      (clk(1) && timer(1) && right_phil(1) && GUARD(Philosopher::c1) &&
       GUARD(Philosopher::c2) && GUARD(Philosopher::c5) )            >>
      ( phil(1) )                                                    >>
      CALL(Philosopher::take_right_fork)                             >>
      s_right_fork_taken;
    
    s_right_fork_taken = 
      (clk(1) && left_phil(1) && GUARD(Philosopher::c1) &&
       GUARD(Philosopher::c3) )                         >>
      ( phil(1) )                                       >>
      CALL(Philosopher::eat)                            >>
      s_eating;

    s_eating =
      (clk(1) && timer(1) && GUARD(Philosopher::c1) &&
       GUARD(Philosopher::c4) )                     >>
      ( phil(1) )                                   >>
      CALL(Philosopher::think)                      >>
      s_thinking;

  }
};

class Timer
  : public smoc_actor{
public:
  smoc_port_in<bool>            clk;
  smoc_port_in<bool>            clk_hist;
  smoc_port_in<phil_type>       phil;
  smoc_port_in<phil_type>       phil_hist;
  smoc_port_out<timer_type>     timer;
  
private:
  timer_type m_timer;

  bool c1() const{
    //TODO: edge detection
    return clk[0] && (clk[0] != clk_hist[0]);
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
    main = (clk(1) && clk_hist(1) && phil(1) && GUARD(Timer::c1)
      && GUARD(Timer::c2) && !GUARD(Timer::c3) && !GUARD(Timer::c4)) >>
      timer(1)                                        >>
      CALL(Timer::zero)                               >>
      main

      | (clk(1) && clk_hist(1) && phil(1) && GUARD(Timer::c1)
      && !GUARD(Timer::c2) && GUARD(Timer::c3) && GUARD(Timer::c4)) >>
      timer(1)                                        >>
      CALL(Timer::add)                                >>
      main

      | (clk(1) && clk_hist(1) && phil(1) && GUARD(Timer::c1)
      && !GUARD(Timer::c2) && !GUARD(Timer::c3) && GUARD(Timer::c4)) >>
      timer(1)                                        >>
      CALL(Timer::add)                                >>
      main;

  }
};

class PhilosopherTestBench
  :public smoc_graph_sr{
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

  Delay<phil_type>  d1;
  Delay<phil_type>  d2;
  Delay<phil_type>  d3;
  Delay<phil_type>  d4;
  Delay<phil_type>  d5;
  Delay<timer_type> d6;
  Delay<timer_type> d7;
  Delay<timer_type> d8;
  Delay<timer_type> d9;
  Delay<timer_type> d10;
  Delay<bool>       d11;


public:
  PhilosopherTestBench(sc_module_name name, int times)
    :smoc_graph_sr(name),
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
     clk("CLK", times),
     d1("d1"),
     d2("d2"),
     d3("d3"),
     d4("d4"),
     d5("d5"),
     d6("d6"),
     d7("d7"),
     d8("d8"),
     d9("d9"),
     d10("d10"),
     d11("d11")
  {
    PhilSignal psig;

    connector(psig)
      << phil5.phil << d5.in;
    connector(psig)
      << d5.out << phil4.right_phil
      << phil1.left_phil << timer5.phil;
    connector(psig)
      << d5.history << phil4.right_phil_hist
      << phil1.left_phil_hist << timer5.phil_hist;
    
    connector(psig)
      << phil4.phil << d4.in;
    connector(psig)
      << d4.out << phil3.right_phil
      << phil5.left_phil << timer4.phil;
    connector(psig)
      << d4.history << phil3.right_phil_hist
      << phil5.left_phil_hist << timer4.phil_hist;

    connector(psig)
      << phil3.phil << d3.in;
    connector(psig)
      << d3.out << phil2.right_phil
      << phil4.left_phil << timer3.phil;
    connector(psig)
      << d3.history << phil2.right_phil_hist
      << phil4.left_phil_hist << timer3.phil_hist;

    connector(psig)
      << phil2.phil << d2.in;
    connector(psig)
      << d2.out << phil1.right_phil
      << phil3.left_phil << timer2.phil;
    connector(psig)
      << d2.history << phil1.right_phil_hist
      << phil3.left_phil_hist << timer2.phil_hist;
   
    connector(psig)
      << phil1.phil << d1.in;
    connector(psig)
      << d1.out << phil5.right_phil
      << phil2.left_phil << timer1.phil;
    connector(psig)
      << d1.history << phil5.right_phil_hist
      << phil2.left_phil_hist << timer1.phil_hist;

    TimerSignal tsig;

    connector(tsig) << timer5.timer << d6.in;
    connector(tsig) << d6.out << phil5.timer; 
    connector(tsig) << d6.history << phil5.timer_hist;
    
    connector(tsig) << timer4.timer << d7.in;
    connector(tsig) << d7.out << phil4.timer; 
    connector(tsig) << d7.history << phil4.timer_hist;
    
    connector(tsig) << timer3.timer << d8.in;
    connector(tsig) << d8.out << phil3.timer; 
    connector(tsig) << d8.history << phil3.timer_hist;

    connector(tsig) << timer2.timer << d9.in;
    connector(tsig) << d9.out << phil2.timer; 
    connector(tsig) << d9.history << phil2.timer_hist;

    connector(tsig) << timer1.timer << d10.in;
    connector(tsig) << d10.out << phil1.timer; 
    connector(tsig) << d10.history << phil1.timer_hist;

    BoolSignal  bsig;

    connector(bsig)
      << clk.out << d11.in;
    connector(bsig)
      << d11.out << timer5.clk << timer4.clk << timer3.clk << timer2.clk
      << timer1.clk << phil5.clk << phil4.clk << phil3.clk << phil2.clk
      << phil1.clk; 
    connector(bsig)
      << d11.history << timer5.clk_hist << timer4.clk_hist << timer3.clk_hist
      << timer2.clk_hist << timer1.clk_hist << phil5.clk_hist << phil4.clk_hist
      << phil3.clk_hist << phil2.clk_hist << phil1.clk_hist;
  }
};
 

int sc_main (int argc, char **argv) {
  size_t count = (argc>1?atoi(argv[1]):0);
  smoc_top_moc<PhilosopherTestBench> nsa_tb("top", count);
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, nsa_tb);
  } else {
    sc_start();
  }
#undef GENERATE
  return 0;
}
