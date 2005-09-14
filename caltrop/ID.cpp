template <typename T>
class ID: public smoc_actor {
public:
  smoc_port_in<T>  In;
  smoc_port_out<T> Out;
private:
  void action0() { Out[0] = In[0]; }
  smoc_firing_state start;
public:
  ID(sc_module_name name)
    : smoc_actor(name, start) {
    start = (In.getAvailableTokens() >= 1) >>
            (Out.getAvailableSpace() >= 1) >>
            call(&ID::action0)             >> start;
  }
};
