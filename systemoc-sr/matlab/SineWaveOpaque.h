/*This will be the file for the SineWaveOpaque Block*/
using namespace std;
class SineWaveOpaque: public smoc_actor {
 public:
  smoc_port_out<real_T> o0;
  smoc_port_out<real_T> o1;
  smoc_port_in<real_T> i0;
 private:
  static const real_T SineWave_Amp = 1.0;
  static const real_T SineWave_Bias = 0.0;
  static const real_T SineWave_Freq = 1.0;
  static const real_T SineWave_Phase = 0.0;
  static const real_T Gain_Gain = 1.0;
  real_T t0;
  real_T t1;
  void update()
  {
    //initialize temporary variables
    real_T i0_temp = i0[0];
    real_T o0_temp;
    real_T o1_temp;

    //Outputs 0

    /* Sin: '<Root>/Sine Wave' */
    o0_temp = sin(t0 * SineWave_Freq + SineWave_Phase) * SineWave_Amp +
      SineWave_Bias;

    /* Gain: '<Root>/Gain' */
    o1_temp = i0_temp * Gain_Gain;

    //Derivative 0

    //Update 0

    //Update Time
    t0 += 1.0;

    //Outputs 1

    //Derivative 1

    //Update 1

    //Update Time
    t1 += 1.0;

    //write temporary variables to output
    o0[0] = o0_temp;
    o1[0] = o1_temp;
  }

  smoc_firing_state start;
 public:
  SineWaveOpaque(sc_module_name name)
    : smoc_actor(name, start)
  {
    //Initialize Conditions:
    t0 = 0;                            //init time
    t1 = 0;                            //init time
    start =
      o0(1) >>
      o1(1) >>
      i0(1) >>
      CALL(SineWaveOpaque::update) >> start
      ;
  }
};
