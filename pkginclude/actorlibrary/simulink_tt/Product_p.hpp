/*  Library : Math Operations
    Block : Product, Divide
    Despcription : Multiply and divide scalars and nonscalars or multiply and 
		   invert matrices.
*/
#ifndef __INCLUDED__PRODUCT_P__HPP__
#define __INCLUDED__PRODUCT_P__HPP__

#include <cstdlib>
#include <iostream>


#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
#include <systemoc/smoc_expr.hpp>
//#include <actorlibrary/tt/TT.hpp>



template<typename DATA_TYPE, int PORTS=1>
 class Product_p: public smoc_periodic_actor {
public:

  smoc_port_in<DATA_TYPE>   in[PORTS];
  smoc_port_out<DATA_TYPE>  out;	
  
  Product_p( sc_module_name name, sc_time period, sc_time offset, std::string operators )
    : smoc_periodic_actor(name, start, period, offset), operators(operators) {

    Expr::Ex<bool >::type eIn(in[0](1) );

    for(int i = 1; i < PORTS; i++){
      eIn = eIn && in[i](1);
    }

    start = //Expr::till( this->getEvent() ) >>  
	out(1) >> eIn                    >> 
      CALL(Product_p::multiply) >> start
	;
  }
protected:
 // const OPERATOR * operators;
  std::string operators;

  void multiply() {   
	//this->resetEvent();
    DATA_TYPE output;

    output = (DATA_TYPE)1;
    for( int i = 0; i<PORTS; i++ ){
      if( operators[i] == '*' ){
        output = output * in[i][0];
      }else if( operators[i] == '/' ){
	if( in[i][0] == 0 ){
		std::cout << sc_time_stamp() << " " <<  name() << "WARNING: Division by ZERO -" << std::endl;
	 	break;
        }
		
        output = output / in[i][0];
      }else{
        assert(0);
      }
    }
    //std::cout << this->name() << " " << output << std::endl;
    out[0] = output;
    
    	#ifdef EnablePrint
	std::cout << sc_time_stamp() << " " <<  name() << " " << " out : " << output << "\n";
	#endif
  }

  smoc_firing_state start;

};

#endif// __INCLUDED__PRODUCT__HPP__

