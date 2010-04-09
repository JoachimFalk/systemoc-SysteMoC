#ifndef __INCLUDE_ADEVA_LIB_HPP
#define __INCLUDE_ADEVA_LIB_HPP

#include <systemoc/smoc_moc.hpp>

/**
 *
 */
template<typename Type, size_t Size>
class Array{
public:
  //
  Type & operator[](size_t index){
    //std::cout << "Array::operator[] (" << index << ")" << std::endl;
    return mData[index];
  }


  //
  const Type & operator[](size_t index) const {
    //std::cout << "Array::operator[] (" << index << ") const" << std::endl;
    return mData[index];
  }

private:
  Type mData [Size];
};

/**
 *
 */
template<typename Type, size_t Size>
std::ostream &operator << (std::ostream &out, const Array<Type, Size> &x)
{
  return out;
}

/**
 *
 */
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

  //
  void forward(){ // GO
    if(!undefined){
      cout << name() << ".forward() " << m_signal << " - " << m_history << endl;
      out[0]     = m_signal;
      history[0] = m_history;

      //save history
      m_history  = m_signal;
    }
  }

  //
  void store(){ // TICK
    if(in.tokenIsValid(0)){
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

    main
      = in(1)                                             >>
        (out(1) && history(1))                            >>
        (SR_GO(Delay::forward) && SR_TICK(Delay::store) ) >> main;
  }
};

/**
 *
 */
template<typename T>
class OutPin : public smoc_actor{
public:

  smoc_port_in<T>     in;
  smoc_port_out<T>    out;

  OutPin(sc_module_name name)
    : smoc_actor(name, main)
  {

    main
      = in(1)                  >>
        out(1)                 >>
        CALL(OutPin::forward)  >>
        main;
  }
private:
  //
  void forward(){
    out[0] = in[0];
  }
  
  smoc_firing_state main;
};

#endif //__INCLUDE_ADEVA_LIB_HPP
