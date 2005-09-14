class Merge: public smoc_actor {
public:
  smoc_port_in<double>  A, B;
  smoc_port_out<double> Out;
private:
  void action0() { Result[0] = A[0]; }
  void action1() { Result[0] = B[0]; }
  smoc_firing_state start;
public:
  Merge(sc_module_name name)
    : smoc_actor(name, start) {
    start = (A.getAvailableTokens()  >= 1) >>
            (Out.getAvailableSpace() >= 1) >>
            call(&Merge::action0)          >> start
          | (B.getAvailableTokens()  >= 1) >>
            (Out.getAvailableSpace() >= 1) >>
            call(&Merge::action1)          >> start;
  }
};
