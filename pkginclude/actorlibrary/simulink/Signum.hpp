/*  Library : Math Operations
    Block : Signum
    Despcription : Indicate sign of input

    Description : The Sign block indicates the sign of the input
	          The output is 1 when the input is greater than zero
	          The output is 0 when the input is equal to zero
	          Then output is -1 when the input is less than zero 

TODO:Zero-Crossing
One:to detect when the input crosses through zero.
*/


#ifndef __INCLUDED__SIGNUM__HPP__
#define __INCLUDED__SIGNUM__HPP__



template<typename T>
 class Signum : public smoc_actor {
public:
  smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  Signum( sc_module_name name )
    : smoc_actor(name, start){


    start = in(1)              >>
      out(1)                   >>
      CALL(Signum ::process) >> start
      ;
  }

protected:

  void process() {

	T data = in[0];
	if(  data == 0 )
	  out[0] = 0; 
	if(  data > 0 )
	  out[0] = 1;
	if(  data < 0 )
	  out[0] = -1; 
  }

  smoc_firing_state start;
};

#endif // __INCLUDED__SIGNUM__HPP__

