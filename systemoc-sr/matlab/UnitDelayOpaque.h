/*This will be the file for the UnitDelayOpaque Block*/
using namespace std;
class UnitDelayOpaque: public smoc_actor {
 public:
  smoc_port_out<real_T> o0;
  smoc_port_in<real_T> i0;
 private:
  static const real_T UnitDelay_X0 = 0.0;
  real_T UnitDelay_DSTATE;

  //FIXME This will be a non-strict actor
  void go()
  {
    //initialize temporary variables
    real_T o0_temp;

    //Outputs

    /* UnitDelay: '<Root>/Unit Delay' */
    o0_temp = UnitDelay_DSTATE;

    //write temporary variables to output
    o0[0] = o0_temp;
  }

  void tick()
  {
    //initialize temporary variables
    real_T i0_temp = i0[0];

    //Derivative

    //Update

    /* Update for UnitDelay: '<Root>/Unit Delay' */
    UnitDelay_DSTATE = i0_temp;
  }

  smoc_firing_state start;
 public:
  UnitDelayOpaque(sc_module_name name)
    : smoc_actor(name, start)
  {
    //Initialize Conditions:

    /* InitializeConditions for UnitDelay: '<Root>/Unit Delay' */
    UnitDelay_DSTATE = UnitDelay_X0;
    start =
      o0(1) >>
      i0(1) >>
      (SR_GO(UnitDelayOpaque::go) &&
       SR_TICK(UnitDelayOpaque::tick)) >> start
      ;
  }
};
