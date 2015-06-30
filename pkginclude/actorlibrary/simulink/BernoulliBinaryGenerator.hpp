/*
 * BernoulliBinaryGenerator.hpp
 *
 *  Created on: Jun 12, 2015
 *      Author: gasmi
 */

#ifndef PKGINCLUDE_ACTORLIBRARY_SIMULINK_BERNOULLIBINARYGENERATOR_HPP_
#define PKGINCLUDE_ACTORLIBRARY_SIMULINK_BERNOULLIBINARYGENERATOR_HPP_

#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_expr.hpp>

#include "SimulinkDataType.hpp"

template<typename DATA_TYPE, int PORTS = 1>
class BernoulliBinaryGenerator: public smoc_actor {
public:
  // Ports declaration
  smoc_port_out<std::vector<DATA_TYPE> > out[PORTS];
private:
  void RandSrcInitState_U_64(const uint32_T seed[], real_T state[],
      int32_T nChans);
  void RandSrc_U_D(std::vector<DATA_TYPE> outputs[], const real_T minVec[],
      int32_T minLen, const real_T maxVec[], int32_T maxLen, real_T state[],
      int32_T nChans, int32_T nSamps);
  void process();

public:
  BernoulliBinaryGenerator(sc_module_name name, double probOfZero,
      uint32_T iniseed, double sampleTime, bool frameBased,
      unsigned int sampPerFrame)

      : smoc_actor(name, start), probOfZero(probOfZero), iniseed(iniseed),
          sampleTime(sampleTime), frameBased(frameBased),
          sampPerFrame(sampPerFrame)

  {
    Expr::Ex<bool>::type eOut(out[0](1));

    for (int i = 1; i < PORTS; i++) {
      eOut = eOut && out[i](1);
    }

    start = eOut >> CALL(BernoulliBinaryGenerator::process) >> start;

    RandSrcInitState_U_64(&this->iniseed, RandomSource_STATE_DWORK, PORTS);
  }

private:

  double        probOfZero;
  uint32_T      iniseed;
  double        sampleTime;
  bool          frameBased;
  unsigned int  sampPerFrame;

  real_T RandomSource_STATE_DWORK[35 * PORTS];

  smoc_firing_state start;

};

template<typename DATA_TYPE, int PORTS>
void BernoulliBinaryGenerator<DATA_TYPE, PORTS>::process() {
  /* S-Function (sdsprandsrc2): '<S1>/Random Source' */
  std::vector<DATA_TYPE> randouts[PORTS];
  const real_T min[1] = { 0.0 };
  const real_T max[1] = { 1.0 };

  RandSrc_U_D(randouts, min, 1, max, 1, RandomSource_STATE_DWORK, PORTS,
      sampPerFrame);
  for (int p = 0; p < PORTS; ++p) {
    std::vector<double> output;
    output.resize(sampPerFrame);
    for (int i = 0; i < sampPerFrame; ++i) {
      output[i] = randouts[p][i] > probOfZero ? 1 : 0;
    }
    out[p][0] = output;
  }
}

template<typename DATA_TYPE, int PORTS>
void BernoulliBinaryGenerator<DATA_TYPE, PORTS>::RandSrcInitState_U_64(
    const uint32_T seed[], real_T state[], int32_T nChans) {
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
        d = (real_T) ((int32_T) (j >> 19) & 1) + (d + d);
      }

      state[35 * i + k] = ldexp(d, -53);
    }

    state[35 * i + 32] = 0.0;
    state[35 * i + 33] = 0.0;
  }

  /* End of InitializeConditions for S-Function (sdsprandsrc2): '<S1>/Random Source' */
}

template<typename DATA_TYPE, int PORTS>
void BernoulliBinaryGenerator<DATA_TYPE, PORTS>::RandSrc_U_D(
    std::vector<DATA_TYPE> outputs[], const real_T minVec[], int32_T minLen,
    const real_T maxVec[], int32_T maxLen, real_T state[], int32_T nChans,
    int32_T nSamps) {
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
  onePtr = (int8_T (*)[]) &one;
  lsw = ((*onePtr)[0U] == 0);
  for (chan = 0; chan < nChans; chan++) {
    min = minVec[minLen > 1 ? chan : 0];
    max = maxVec[maxLen > 1 ? chan : 0];
    max -= min;
    i = (int32_T) ((uint32_T) state[chan * 35 + 33] & 31U);
    j = (uint32_T) state[chan * 35 + 34];
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
      (*(int32_T (*)[]) &d)[lsw] ^= j;
      j ^= j << 13;
      j ^= j >> 17;
      j ^= j << 5;
      (*(int32_T (*)[]) &d)[lsw ^ 1] ^= j & 1048575U;
      outputs[chan][samps] = max * d + min;
    }

    state[chan * 35 + 33] = i;
    state[chan * 35 + 34] = j;
  }

  /* End of S-Function (sdsprandsrc2): '<S1>/Random Source' */
}

#endif /* PKGINCLUDE_ACTORLIBRARY_SIMULINK_BERNOULLIBINARYGENERATOR_HPP_ */
