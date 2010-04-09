/*This will be the file for the DiscreteTimeIntegratorOpaque Block*/
using namespace std;
class DiscreteTimeIntegratorOpaque: public smoc_actor {
 public:
  smoc_port_out<real_T> o0;
  smoc_port_in<real_T> i0;
 private:
  static const real_T DiscreteTimeIntegrator_gai = 1.0;
  static const real_T DiscreteTimeIntegrator_IC = 0.0;
  real_T DiscreteTimeIntegrator_DSTATE;

  //FIXME This will be a non-strict actor
  void go()
  {
    cout << ">DiscreteTimeIntegrator::go()"<< endl;
    //initialize temporary variables
    real_T o0_temp;

    //Outputs

    /* DiscreteIntegrator: '<Root>/Discrete-Time Integrator' */
    o0_temp = DiscreteTimeIntegrator_DSTATE;

    //write temporary variables to output
    o0[0] = o0_temp;

    cout << "<DiscreteTimeIntegrator::go()"<< endl;
  }

  void tick()
  {
    cout << ">DiscreteTimeIntegrator::tick()"<< endl;
    //initialize temporary variables
    real_T i0_temp = i0[0];

    //Derivative

    //Update

    /* Update for DiscreteIntegrator: '<Root>/Discrete-Time Integrator' */
    DiscreteTimeIntegrator_DSTATE = DiscreteTimeIntegrator_gai * i0_temp +
      DiscreteTimeIntegrator_DSTATE;
    cout << "<DiscreteTimeIntegrator::tick()"<< endl;
  }

  smoc_firing_state start;
 public:
  DiscreteTimeIntegratorOpaque(sc_module_name name)
    : smoc_actor(name, start)
  {
    //Initialize Conditions:

    /* InitializeConditions for DiscreteIntegrator: '<Root>/Discrete-Time Integrator' */
    DiscreteTimeIntegrator_DSTATE = DiscreteTimeIntegrator_IC;
    start =
      o0(1) >>
      i0(1) >>
      (SR_GO(DiscreteTimeIntegratorOpaque::go) &&
       SR_TICK(DiscreteTimeIntegratorOpaque::tick)) >> start
      ;
  }
};
