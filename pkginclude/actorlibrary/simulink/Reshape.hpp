/*
 * Reshape.hpp
 *
 *  Created on: 05.06.2015
 *      Author: gasmi
 */

#ifndef PKGINCLUDE_ACTORLIBRARY_SIMULINK_RESHAPE_HPP_
#define PKGINCLUDE_ACTORLIBRARY_SIMULINK_RESHAPE_HPP_
#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_expr.hpp>
#include <random>
#include <boost/random/linear_congruential.hpp>
#include <boost/random/uniform_int.hpp>
#include <boost/random/uniform_real.hpp>
#include <boost/random/bernoulli_distribution.hpp>
#include <boost/random/variate_generator.hpp>
#include <ctime>
using namespace std ;


typedef boost::minstd_rand  base_generator_type ;// This is a typedef for random number generator
// This is a reproducible simulation experiment.  See main().



template<typename T,typename S>
class Reshape: public smoc_actor{

public :
  smoc_port_in<T> in;

  smoc_port_out<S>  out; // double

  Reshape(sc_module_name name):
    smoc_actor(name,start) {

    start=out(1)  >>
        CALL ( Reshape ::reshape) >> start;


  }
 // protected:



private :
  smoc_firing_state start;


  void reshape()
  {

 const int M,N;
double Matrix_int[N][M];
double Vector_out [N*M];
int k = 0;
Matrix_int= int(0);
for( int i= 0; i<N ; i++)
  for( int j=0; i<M ; j++,k++)
  {
Vector_out[k]= Matrix_int[i][j];
k++;
  }

out=Vector_out;
}


  };


#endif /* PKGINCLUDE_ACTORLIBRARY_SIMULINK_RESHAPE_HPP_ */
