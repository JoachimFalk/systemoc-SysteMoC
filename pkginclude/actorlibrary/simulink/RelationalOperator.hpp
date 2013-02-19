/*
 * Block : Relational Operator
 * 
 * Perform specified relational operation on inputs
 * Library : Logic and Bit Operations
 * Description : 
 * 
 * Two-Input Mode
 * By default, the Relational Operator block compares two inputs using the Relational operator parameter that you specify. 
 * The first input corresponds to the top input port and the second input to the bottom input port.
 * 
 * One-Input Mode
 * TODO
 *
 * Treatment to Rotated block(various block orientations)
 * TODO
 *
 *
 *  TODO: Zero-Crossing
 *  One: to detect when the specified relation is true.
 */

#ifndef __INCLUDED__RELATIONALOPERATOR__HPP__
#define __INCLUDED__RELATIONALOPERATOR__HPP__





template<typename T , int PORTS=1>
 class RelationalOperator: public smoc_actor {
public:

  smoc_port_in<T>   in[PORTS];
  smoc_port_out<T>  out;	
  
  RelationalOperator( sc_module_name name, int logicOperator )
    : smoc_actor(name, start), logicOperator(logicOperator) {

    Expr::Ex<bool >::type eIn(in[0](1) );

    for(int i = 1; i < PORTS; i++){
      eIn = eIn && in[i](1);
    }

    start = eIn  >> out(1)                 >> 
      CALL(RelationalOperator::process) >> start
      ;
  }
protected:
 
  int logicOperator;
  bool inputsLogic[PORTS];

  void process() {   
    
    T output;

    	    
    for( int i=0; i < PORTS; i++ ){
       inputsLogic[i] = in[i][0];
    }

/* 
 * Integer logicOperator = 0 means operator = "=="
 * Integer logicOperator = 1 means operator = "~=" 
 * Integer logicOperator = 2 means operator = "<"
 * Integer logicOperator = 3 means operator = "<="
 * Integer logicOperator = 4 means operator = ">="
 * Integer logicOperator = 5 means operator = ">"
 */

    switch(logicOperator)
    {
	output = 0;
        case 0: 
           if( inputsLogic[0] == inputsLogic[1] )
		output = 1;
	   break;
        case 1: 
           if( inputsLogic[0] != inputsLogic[1] )
		output = 1;
	   break;        
        case 2: 
           if( inputsLogic[0] < inputsLogic[1] )
		output = 1;
	   break;        
        case 3: 
           if( inputsLogic[0] <= inputsLogic[1] )
		output = 1;
	   break;        
        case 4: 
           if( inputsLogic[0] >= inputsLogic[1] )
		output = 1;
	   break;        
        case 5: 
           if( inputsLogic[0] > inputsLogic[1] )
		output = 1;
	   break;        
        default:
	   break;
    }

    out[0] = output;
  }

  smoc_firing_state start;

};

#endif// __INCLUDED__RELATIONALOPERATOR__HPP__

