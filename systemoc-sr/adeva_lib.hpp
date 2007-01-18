#include <smoc_moc.hpp>

//ADeVA Lib ??
template<typename T>
class Delay : public smoc_actor{
public:
  typedef T                  signal_t;

  smoc_port_in<signal_t>     in;
  smoc_port_out<signal_t>    out;
  smoc_port_out<signal_t>    history;

private:
  signal_t                   m_signal;
  signal_t                   m_history;
  bool                       undefined;

  void forward(){ // GO
    if(!undefined){
      //cout << name() << ".forward()" << endl;
      out[0]     = m_signal;
      history[0] = m_history;

      //save history
      m_history  = m_signal;
    }
  }

  void store(){ // TICK
    if(in.isValid(0)){
      //cout << name() << ".store()" << endl;
      m_signal = in[0];
      if(undefined) m_history = in[0]; // initialize with no history
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
      (out(1) && history(1))                            >>
      (SR_GO(Delay::forward) && SR_TICK(Delay::store) ) >> main;
  }
};

