/*  Library : Math Operations

    Block : Product, Divide
    Despcription : Multiply and divide scalars and nonscalars or multiply and 
		   invert matrices.
*/
#ifndef __INCLUDED__PRODUCT__HPP__
#define __INCLUDED__PRODUCT__HPP__


#include <systemoc/smoc_expr.hpp>

template<typename DATA_TYPE, int PORTS=1>
class Product: public smoc_actor {
public:
  smoc_port_in<DATA_TYPE>   in;
  smoc_port_out<DATA_TYPE>  out;	
  

protected:
 // const OPERATOR * operators;
  std::string operators;


  void multiply() {   
    
#ifdef _DEBUG    
    cout << name();
#endif
    
    DATA_TYPE output= 1;
    
    for( int i = 0; i<PORTS; i++ ){
      
      DATA_TYPE tmp = in[i][0];
#ifdef _DEBUG      
      cout << name() << " " << tmp << " [" << i << "]" << endl;
#endif
      

      
      if( operators[i] == '*' ){
        output = output * tmp;
      }else if( operators[i] == '/' ){
	//assert(tmp!=0); 
        output = output / tmp;
      }
    }
    out[0] = output;
#ifdef _DEBUG    
    cout << " " << output << endl;
#endif
  }

  smoc_firing_state start;

};

#endif// __INCLUDED__PRODUCT__HPP__

