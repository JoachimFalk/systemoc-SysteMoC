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

#ifndef __INCLUDED__RELATIONALOPERATOR_P__HPP__
#define __INCLUDED__RELATIONALOPERATOR_P__HPP__


#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <actorlibrary/tt/TT.hpp>


template<typename T , int PORTS=1>
 class RelationalOperator_p: public PeriodicActor {
public:

  smoc_port_in<T>   in[PORTS];
  smoc_port_out<T>  out;	
  
  RelationalOperator_p( sc_module_name name, sc_time per, sc_time off, EventQueue* _eq, int logicOperator )
    : PeriodicActor(name, start, per, off, _eq), logicOperator(logicOperator) {

    Expr::Ex<bool >::type eIn(in[0](1) );

    for(int i = 1; i < PORTS; i++){
      eIn = eIn && in[i](1);
    }

    start =  Expr::till( this->getEvent() )  >>
      out(1)  >> eIn               >>
      CALL(RelationalOperator_p::process) >> start
      ;
  }
protected:
 
  int logicOperator;
  //bool inputsLogic[PORTS];
  //T inputsLogic[PORTS];

  void process() {   
    this->resetEvent();
    T output;

    //std::cout << "RelationOperator> get: " << in[0][0] << " @ " << sc_time_stamp() << std::endl;
    //std::cout << "RelationOperator> get: " << in[1][0] << " @ " << sc_time_stamp() << std::endl;
    //std::cout << "RelationOperator> Operator: " << logicOperator << " @ " << sc_time_stamp() << std::endl;	    


/* 
 * Integer logicOperator = 0 means operator = "=="
 * Integer logicOperator = 1 means operator = "~=" 
 * Integer logicOperator = 2 means operator = "<"
 * Integer logicOperator = 3 means operator = "<="
 * Integer logicOperator = 4 means operator = ">="
 * Integer logicOperator = 5 means operator = ">"
 */
    /*
    for( int i=0; i < PORTS; i++ ){
       inputsLogic[i] = in[i][0];
    }

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
    */

    output = 0.0;
    switch(logicOperator)
    {
	
        case 0: 
           if( in[0][0] == in[1][0] )
		output = 1;
	   break;
        case 1: 
           if( in[0][0] != in[1][0] )
		output = 1;
	   break;        
        case 2: 
           if( in[0][0] < in[1][0] )
		output = 1;
	   break;        
        case 3: 
           if( in[0][0] <= in[1][0] )
		output = 1;
           //std::cout << "RelationOperator> Operator: " << logicOperator << " @ " << sc_time_stamp() << std::endl;	    
	   break;        
        case 4: 
           if( in[0][0] >= in[1][0] )
		output = 1;
	   break;        
        case 5: 
           if( in[0][0] > in[1][0] )
		output = 1;
	   break;        
        default:
	   break;
    }
    out[0] = (T)output;
    //std::cout << "RelationOperator> Output: " << (T)output << " @ " << sc_time_stamp() << std::endl;	    
  }

  smoc_firing_state start;

};

#endif// __INCLUDED__RELATIONALOPERATOR__HPP__

