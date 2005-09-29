template <typename T>
class Sum: public smoc_actor {
public:
  smoc_port_in<T>  A;
  smoc_port_out<T> Result;
private:
  double sum;
  
  void action0() {
    sum = sum + A[0];
    Result[0] = sum;
  }
  smoc_firing_state start;
public:
  Sum(sc_module_name name)
    : smoc_actor(name, start) {
    sum = (A.getAvailableTokens()      >= 1) >>
          (Balance.getAvailableSpace() >= 1) >>
          call(&Sum::action0) >> start;
  }
};
