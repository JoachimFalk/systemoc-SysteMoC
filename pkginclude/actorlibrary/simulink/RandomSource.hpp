/*Library :Sources

 * Random_Source.cpp
 Description:
 Output a random signal with uniform or gaussian distribuation
 *
 *  Created on: 03.06.2015
 *      Author: gasmi
 */

#ifndef PKGINCLUDE_ACTORLIBRARY_SIMULINK_RANDOMSOURCE_HPP_
#define PKGINCLUDE_ACTORLIBRARY_SIMULINK_RANDOMSOURCE_HPP_

#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_expr.hpp>

#include "SimulinkDataType.hpp"

template<typename DATA_TYPE, int PORTS=1>
class RandomSource: public smoc_actor {

public:
  smoc_port_out<std::vector<DATA_TYPE> > out[PORTS];
private:
  // state vector
  real_T RandomSource_STATE_DWORK[35*PORTS];

  uint32_T RandomSource_InitSeed[PORTS];/* Computed Parameter: RandomSource_InitSeed
                                         * Referenced by: '<S1>/Random Source'
                                         */
  real_T RandomSource_MinRTP[PORTS];    /* Expression: MinVal
                                         * Referenced by: '<S1>/Random Source'
                                         */
  real_T RandomSource_MaxRTP[PORTS];    /* Expression: MaxVal
                                         * Referenced by: '<S1>/Random Source'
                                         */
  int nrSamples;

  void RandSrcInitState_U_64(const uint32_T seed[], real_T state[], int32_T nChans);
  void RandSrc_U_D(std::vector<DATA_TYPE> outputs[], const real_T minVec[], int32_T
      minLen, const real_T maxVec[], int32_T maxLen, real_T state[], int32_T nChans,
      int32_T nSamps);

  smoc_firing_state start;
public:

  RandomSource(sc_core::sc_module_name name, const uint32_T RandomSource_InitSeed[PORTS], const real_T RandomSource_MinRTP[PORTS], const real_T RandomSource_MaxRTP[PORTS], int nrSamples);

  void process();
};

template<typename DATA_TYPE, int PORTS>
RandomSource<DATA_TYPE, PORTS>::RandomSource(
    sc_core::sc_module_name name,
    const uint32_T RandomSource_InitSeed[PORTS],
    const real_T   RandomSource_MinRTP[PORTS],
    const real_T   RandomSource_MaxRTP[PORTS],
    int nrSamples)
  : smoc_actor(name, start),
    nrSamples(nrSamples)
{
  memcpy(this->RandomSource_InitSeed, RandomSource_InitSeed, sizeof(RandomSource_InitSeed));
  memcpy(this->RandomSource_MinRTP, RandomSource_MinRTP, sizeof(RandomSource_MinRTP));
  memcpy(this->RandomSource_MaxRTP, RandomSource_MaxRTP, sizeof(RandomSource_MaxRTP));

  SMOC_REGISTER_CPARAM(RandomSource_InitSeed);
  SMOC_REGISTER_CPARAM(RandomSource_MinRTP);
  SMOC_REGISTER_CPARAM(RandomSource_MaxRTP);
  SMOC_REGISTER_CPARAM(nrSamples);

  // Build actor FSM
  Expr::Ex<bool >::type eOut(out[0](1));
  for(int i = 1; i < PORTS; i++){
    eOut = eOut && out[i](1);
  }
  start = eOut >> CALL(RandomSource::process) >> start;

  RandSrcInitState_U_64(RandomSource_InitSeed, RandomSource_STATE_DWORK, PORTS);
}

template<typename DATA_TYPE, int PORTS>
void RandomSource<DATA_TYPE, PORTS>::process() {
  std::vector<DATA_TYPE> outputs[PORTS];

  /* S-Function (sdsprandsrc2): '<S1>/Random Source' */
  RandSrc_U_D(outputs, RandomSource_MinRTP, 1,
              RandomSource_MaxRTP, 1,
              RandomSource_STATE_DWORK, PORTS, nrSamples);

  for (int i = 0; i < PORTS; ++i) {
    out[i][0] = outputs[i];
  }
}


template<typename DATA_TYPE, int PORTS>
void RandomSource<DATA_TYPE, PORTS>::RandSrcInitState_U_64(const uint32_T seed[], real_T state[], int32_T nChans)
{
  int32_T i;
  uint32_T j;
  int32_T k;
  int32_T n;
  real_T d;

  /* InitializeConditions for S-Function (sdsprandsrc2): '<S1>/Random Source' */
  /* RandSrcInitState_U_64 */
  for (i = 0; i < nChans; i++) {
    j = seed[i] != 0U ? seed[i] : 2147483648U;
    state[35 * i + 34] = j;
    for (k = 0; k < 32; k++) {
      d = 0.0;
      for (n = 0; n < 53; n++) {
        j ^= j << 13;
        j ^= j >> 17;
        j ^= j << 5;
        d = (real_T)((int32_T)(j >> 19) & 1) + (d + d);
      }

      state[35 * i + k] = ldexp(d, -53);
    }

    state[35 * i + 32] = 0.0;
    state[35 * i + 33] = 0.0;
  }

  /* End of InitializeConditions for S-Function (sdsprandsrc2): '<S1>/Random Source' */
}

template<typename DATA_TYPE, int PORTS>
void RandomSource<DATA_TYPE, PORTS>::RandSrc_U_D(
    std::vector<DATA_TYPE> outputs[],
    const real_T minVec[], int32_T minLen,
    const real_T maxVec[], int32_T maxLen,
    real_T state[],
    int32_T nChans, int32_T nSamps)
{
  int32_T one;
  int32_T lsw;
  int8_T (*onePtr)[];
  int32_T chan;
  real_T min;
  real_T max;
  int32_T samps;
  real_T d;
  int32_T i;
  uint32_T j;

  /* S-Function (sdsprandsrc2): '<S1>/Random Source' */
  /* RandSrc_U_D */
  one = 1;
  onePtr = (int8_T (*)[])&one;
  lsw = ((*onePtr)[0U] == 0);
  for (chan = 0; chan < nChans; chan++) {
    min = minVec[minLen > 1 ? chan : 0];
    max = maxVec[maxLen > 1 ? chan : 0];
    max -= min;
    i = (int32_T)((uint32_T)state[chan * 35 + 33] & 31U);
    j = (uint32_T)state[chan * 35 + 34];
    outputs[chan].resize(nSamps);
    for (samps = 0; samps < nSamps; samps++) {
      /* "Subtract with borrow" generator */
      d = state[((i + 20) & 31) + chan * 35];
      d -= state[((i + 5) & 31) + chan * 35];
      d -= state[chan * 35 + 32];
      if (d >= 0.0) {
        state[chan * 35 + 32] = 0.0;
      } else {
        d++;

        /* set 1 in LSB */
        state[chan * 35 + 32] = 1.1102230246251565E-16;
      }

      state[chan * 35 + i] = d;
      i = (i + 1) & 31;

      /* XOR with shift register sequence */
      (*(int32_T (*)[])&d)[lsw] ^= j;
      j ^= j << 13;
      j ^= j >> 17;
      j ^= j << 5;
      (*(int32_T (*)[])&d)[lsw ^ 1] ^= j & 1048575U;
      outputs[chan][samps] = max * d + min;
    }

    state[chan * 35 + 33] = i;
    state[chan * 35 + 34] = j;
  }

  /* End of S-Function (sdsprandsrc2): '<S1>/Random Source' */
}

#endif /* PKGINCLUDE_ACTORLIBRARY_SIMULINK_RANDOMSOURCE_HPP_ */
