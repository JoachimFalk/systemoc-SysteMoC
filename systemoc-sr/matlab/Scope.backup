/*This will be the file for the Scope Block*/
using namespace std;
class Scope: public smoc_actor {
 public:
  smoc_port_in<real_T> i0;
 private:
  pointer_T Scope_PWORK;

  //FIXME This will be a non-strict actor
  void go()
  {
    //initialize temporary variables

    //Outputs

    //write temporary variables to output
  }

  void tick()
  {
    //initialize temporary variables
    real_T i0_temp = i0[0];

    //Derivative

    //Update

    cout << "Scope: " << i0_temp << endl;
  }

  smoc_firing_state start;
 public:
  Scope(sc_module_name name)
    : smoc_actor(name, start)
  {
    //Initialize Conditions:
    start =
      i0(1) >>
      (SR_GO(Scope::go) &&
       SR_TICK(Scope::tick)) >> start
      ;
  }
};
