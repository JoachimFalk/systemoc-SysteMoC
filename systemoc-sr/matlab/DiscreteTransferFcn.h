/*This will be the file for the DiscreteTransferFcn Block*/
using namespace std;
class DiscreteTransferFcn: public smoc_actor {
 public:
  smoc_port_out<real_T> o0;
  smoc_port_in<real_T> i0;
 private:
  static const real_T DiscreteTransferFcn_A = -0.5;
  static const real_T DiscreteTransferFcn_C = 1.0;
  real_T DiscreteTransferFcn_DSTATE;

  //FIXME This will be a non-strict actor
  void go()
  {
    //initialize temporary variables
    real_T o0_temp;

    //Outputs
    o0_temp = DiscreteTransferFcn_C*DiscreteTransferFcn_DSTATE;

    //write temporary variables to output
    o0[0] = o0_temp;
  }

  void tick()
  {
    //initialize temporary variables
    real_T i0_temp = i0[0];

    //Derivative

    //Update
    /* DiscreteTransferFcn Block: '<Root>/Discrete Transfer Fcn' */
    {
      DiscreteTransferFcn_DSTATE = i0_temp + DiscreteTransferFcn_A*
        DiscreteTransferFcn_DSTATE;
    }
  }

  smoc_firing_state start;
 public:
  DiscreteTransferFcn(sc_module_name name)
    : smoc_actor(name, start)
  {
    //Initialize Conditions:
    start =
      o0(1) >>
      i0(1) >>
      (SR_GO(DiscreteTransferFcn::go) &&
       SR_TICK(DiscreteTransferFcn::tick)) >> start
      ;
  }
};
