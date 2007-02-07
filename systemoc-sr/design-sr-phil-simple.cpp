#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>

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
  Clock(sc_module_name name, SMOC_ACTOR_CPARAM( int, limit ))
    : smoc_actor(name, s_pos),  limitCount( 0 ) {

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
    PhilSignal().connect(phil5.phil).connect(d5.in);
    PhilSignal().connect(d5.out)
      .connect(phil4.right_phil)
      .connect(phil1.left_phil)
      .connect(timer5.phil);
    PhilSignal().connect(d5.history)
      .connect(phil4.right_phil_hist)
      .connect(phil1.left_phil_hist)
      .connect(timer5.phil_hist);


    PhilSignal().connect(phil4.phil).connect(d4.in);
    PhilSignal().connect(d4.out)
      .connect(phil3.right_phil)
      .connect(phil5.left_phil)
      .connect(timer4.phil);
    PhilSignal().connect(d4.history)
      .connect(phil3.right_phil_hist)
      .connect(phil5.left_phil_hist)
      .connect(timer4.phil_hist);

    PhilSignal().connect(phil3.phil).connect(d3.in);
    PhilSignal().connect(d3.out)
      .connect(phil2.right_phil)
      .connect(phil4.left_phil)
      .connect(timer3.phil);
    PhilSignal().connect(d3.history)
      .connect(phil2.right_phil_hist)
      .connect(phil4.left_phil_hist)
      .connect(timer3.phil_hist);

    PhilSignal().connect(phil2.phil).connect(d2.in);
    PhilSignal().connect(d2.out)
      .connect(phil1.right_phil)
      .connect(phil3.left_phil)
      .connect(timer2.phil);
    PhilSignal().connect(d2.history)
      .connect(phil1.right_phil_hist)
      .connect(phil3.left_phil_hist)
      .connect(timer2.phil_hist);

    PhilSignal().connect(phil1.phil).connect(d1.in);
    PhilSignal().connect(d1.out)
      .connect(phil5.right_phil)
      .connect(phil2.left_phil)
      .connect(timer1.phil);
    PhilSignal().connect(d1.history)
      .connect(phil5.right_phil_hist)
      .connect(phil2.left_phil_hist)
      .connect(timer1.phil_hist);

    TimerSignal().connect(timer5.timer).connect(d6.in);
    TimerSignal().connect(d6.out).connect(phil5.timer);
    TimerSignal().connect(d6.history).connect(phil5.timer_hist);

    TimerSignal().connect(timer4.timer).connect(d7.in);
    TimerSignal().connect(d7.out).connect(phil4.timer);
    TimerSignal().connect(d7.history).connect(phil4.timer_hist);

    TimerSignal().connect(timer3.timer).connect(d8.in);
    TimerSignal().connect(d8.out).connect(phil3.timer);
    TimerSignal().connect(d8.history).connect(phil3.timer_hist);

    TimerSignal().connect(timer2.timer).connect(d9.in);
    TimerSignal().connect(d9.out).connect(phil2.timer);
    TimerSignal().connect(d9.history).connect(phil2.timer_hist);

    TimerSignal().connect(timer1.timer).connect(d10.in);
    TimerSignal().connect(d10.out).connect(phil1.timer);
    TimerSignal().connect(d10.history).connect(phil1.timer_hist);

    BoolSignal().connect(clk.out).connect(d11.in);
    BoolSignal().connect(d11.out)
      .connect(timer5.clk)
      .connect(timer4.clk)
      .connect(timer3.clk)
      .connect(timer2.clk)
      .connect(timer1.clk)
      .connect(phil5.clk)
      .connect(phil4.clk)
      .connect(phil3.clk)
      .connect(phil2.clk)
      .connect(phil1.clk);
    BoolSignal().connect(d11.history)
      .connect(timer5.clk_hist)
      .connect(timer4.clk_hist)
      .connect(timer3.clk_hist)
      .connect(timer2.clk_hist)
      .connect(timer1.clk_hist)
      .connect(phil5.clk_hist)
      .connect(phil4.clk_hist)
      .connect(phil3.clk_hist)
      .connect(phil2.clk_hist)
      .connect(phil1.clk_hist);
  }
};
 

int sc_main (int argc, char **argv) {
  size_t count = (argc>1?atoi(argv[1]):0);
  smoc_top_sr_moc<PhilosopherTestBench> nsa_tb("top", count);
  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, nsa_tb);
  } else {
    sc_start(-1);
  }
#undef GENERATE
  return 0;
}
