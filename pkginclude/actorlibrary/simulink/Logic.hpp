/*
 * Block : Logical Operator
 * 
 * Perform specified logical operation on input
 * Library : Logic and Bit Operations
 * Description : The Logical Operator block performs the specified logical operation on its inputs. 
 * An input value is TRUE (1) if it is nonzero and FALSE (0) if it is zero.
 * 
 * 
 */

#ifndef __INCLUDED__LOGIC__HPP__
#define __INCLUDED__LOGIC__HPP__


			/* 
			 * Integer logicOperator = 0 means operator = AND
			 * Integer logicOperator = 1 means operator = OR 
			 * Integer logicOperator = 2 means operator = NAND
			 * Integer logicOperator = 3 means operator = NOR
			 * Integer logicOperator = 4 means operator = XOR
			 * Integer logicOperator = 5 means operator = NXOR
			 * Integer logicOperator = 6 means operator = NOT
			 */


template<typename T , int PORTS=1>
 class Logic: public smoc_actor {
public:

  smoc_port_in<T>   in[PORTS];
  smoc_port_out<T>  out;	
  
  Logic( sc_module_name name, int logicOperator )
    : smoc_actor(name, start), logicOperator(logicOperator) {

    Expr::Ex<bool >::type eIn(in[0](1) );

    for(int i = 1; i < PORTS; i++){
      eIn = eIn && in[i](1);
    }

    start = eIn                    >> 
      CALL(Logic::process) >> start
      ;
  }
protected:
 
  int logicOperator;
  bool inputsLogic[PORTS];

  void process() {   
    
    T output;

    	    
    for( int i=0; i < PORTS; i++ ){
       if( in[i][0] > 0 )
	  inputsLogic[i] = true;
	else
	  inputsLogic[i] = false;
    }

    switch(logicOperator)
    {
	output = inputsLogic[0];
        case '0': /* AND */
           for( int i=1; i<PORTS; i++ )
		output = output && inputsLogic[i];
	   break;
        case '1': /* OR */
           for( int i=1; i<PORTS; i++ )
		output = output || inputsLogic[i];
	   break;
        case '2': /* NAND */
           for( int i=1; i<PORTS; i++ )
		output = output && inputsLogic[i];
	   output = !output;
	   break;
        case '3': /* NOR */
           for( int i=1; i<PORTS; i++ )
		output = output || inputsLogic[i];
	   output = !output;
	   break;
        case '4': /* XOR : true + true ---> false, true + false ---> true */
           for( int i=1; i<PORTS; i++ )
               if( output == inputsLogic[i] )
		  output = 0;
               else
                  output = 1;
	   break;
        case '5': /* NXOR */
           for( int i=1; i<PORTS; i++ )
               if( output == inputsLogic[i] )
		  output = 0;
               else
                  output = 1;
           output = !output;
	   break;
        case '6': /* NOT : Should have only one input */
           output = !inputsLogic[0];
	   break;
        default:
	   break;
    }

    out[0] = output;
  }

  smoc_firing_state start;

};

#endif// __INCLUDED__LOGIC__HPP__

