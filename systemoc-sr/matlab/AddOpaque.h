/*This will be the file for the AddOpaque Block*/
using namespace std;
class AddOpaque: public smoc_actor {
 public:
  smoc_port_out<real_T> o0;
  smoc_port_in<real_T> i0;
  smoc_port_in<real_T> i1;
 private:
  void update()
  {
    //initialize temporary variables
    real_T i0_temp = i0[0];
    real_T i1_temp = i1[0];
    real_T o0_temp;

    //Outputs

    /* Sum: '<Root>/Add' */
    o0_temp = i0_temp + i1_temp;

    //Derivative

    //Update

    //write temporary variables to output
    o0[0] = o0_temp;
  }

  smoc_firing_state start;
 public:
  AddOpaque(sc_module_name name)
    : smoc_actor(name, start)
  {
    //Initialize Conditions:
    start =
      o0(1) >>
      i0(1) >>
      i1(1) >>
      CALL(AddOpaque::update) >> start
      ;
  }
};
