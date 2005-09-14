template <typename T>
class Add: public smoc_actor {
public:
  smoc_port_in<T>  A, B;
  smoc_port_out<T> Result;
private:
  void action0() { Result[0] = A[0] + B[0]; }
  smoc_firing_state start;
public:
  Add(sc_module_name name)
    : smoc_actor(name, start) {
    start = (A.getAvailableTokens()  >= 1 &&
             B.getAvailableTokens()  >= 1   ) >>
            (out.getAvailableSpace() >= 1   ) >>
              call(&Add::action0)             >> start;
  }
};


