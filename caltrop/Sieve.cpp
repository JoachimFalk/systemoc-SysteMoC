template <typename T>
class Select: public smoc_actor {
public:
  smoc_port_in<int>  Control;
  smoc_port_in<T>    Data0, Data1;
  smoc_port_out<T>   Output;
private:
  void action0() { Output[0] = Data0[0] ; }
  void action1() { Output[0] = Data0[1] ; }
  smoc_firing_state start;
public:
  Select(sc_module_name name, int initialChannel = 0)
    : smoc_actor(name, start) {
    smoc_firing_state atChannel0, atChannel1;
    
    atChannel0
      = (Control.getAvailableTokens() >= 1 &
         Data0.getAvailableTokens()   >= 1 &
         Control.getValueAt(0)        == 0   ) >>
        (Output.getAvailableSpace()   >= 1   ) >>
        call(&Select::action0)                 >> atChannel0
      | (Control.getAvailableTokens() >= 1 &
         Data1.getAvailableTokens()   >= 1 &
         Control.getValueAt(0)        == 1   ) >>
        (Output.getAvailableSpace()   >= 1   ) >> 
        call(&Select::action1)                 >> atChannel1
      | (Data0.getAvailableTokens()   >= 1   ) >>
        (Output.getAvailableSpace()   >= 1   ) >>
         call(&Select::action0)                >> atChannel0;

    atChannel1
      = (Control.getAvailableTokens() >= 1 &
         Data1.getAvailableTokens()   >= 1 &
         Control.getValueAt(0)        == 1   ) >>
        (Output.getAvailableSpace()   >= 1   ) >>
        call(&Select::action1)                 >> atChannel1
      | (Control.getAvailableTokens() >= 1 &
         Data0.getAvailableTokens()   >= 1 &
         Control.getValueAt(0)        == 0   ) >>
        (Output.getAvailableSpace()   >= 1   ) >> 
        call(&Select::action0)                 >> atChannel0
      | (Data1.getAvailableTokens()   >= 1   ) >>
        (Output.getAvailableSpace()   >= 1   ) >>
         call(&Select::action1)                >> atChannel1;
    
    start = initialChannel == 0
      ? atChannel0
      : atChannel1;
  }
};
