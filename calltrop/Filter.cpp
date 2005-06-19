template <typename T>
class Filter: public smoc_actor {
public:
  smoc_port_in<T>    Input;
  smoc_port_out<T>   Output;
private:
  bool (*filter)(const T &);
  
  bool guard() const { return filter(Input[0]); }
  
  void action0() { Output[0] = Input[0] ; }
  void action1() {}
  smoc_firing_state start;
public:
  Filter(sc_module_name name, bool (*filter)(const T &))
    : smoc_actor(name, start),
      filter(filter) {
    start = (Input.getAvailableTokens() >= 1 &
             guard(&Filter::guard)             ) >>
            (Output.getAvailableSpace() >= 1   ) >>
            call(&Filter::action0)               >> start
          | (Input.getAvailableTokens() >= 1 &
             ! guard(&Filter::guard)           ) >>
            call(&Filter::action1)               >> start;
};
