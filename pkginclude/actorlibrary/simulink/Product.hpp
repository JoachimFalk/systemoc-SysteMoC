/*  Library : Math Operations
    Block : Product, Divide
    Despcription : Multiply and divide scalars and nonscalars or multiply and 
		   invert matrices.
*/
#ifndef __INCLUDED__PRODUCT__HPP__
#define __INCLUDED__PRODUCT__HPP__





template<typename DATA_TYPE, int PORTS=1>
 class Product: public smoc_actor {
public:

  smoc_port_in<DATA_TYPE>   in[PORTS];
  smoc_port_out<DATA_TYPE>  out;	
  
  Product( sc_module_name name, std::string operators )
    : smoc_actor(name, start), operators(operators) {

    Expr::Ex<bool >::type eIn(in[0](1) );

    for(int i = 1; i < PORTS; i++){
      eIn = eIn && in[i](1);
    }

    start = eIn                    >> 
      CALL(Product::multiply) >> start
      ;
  }
protected:
 // const OPERATOR * operators;
  std::string operators;

  void multiply() {   
    DATA_TYPE output;

    output = 1;
    for( int i = 0; i<PORTS; i++ ){
      if( operators[i] == '*' ){
        output = output * in[i][0];
      }else if( operators[i] == '/' ){
        output = output / in[i][0];
      }else{
        assert(0);
      }
    }
    out[0] = output;
  }

  smoc_firing_state start;

};

#endif// __INCLUDED__PRODUCT__HPP__

