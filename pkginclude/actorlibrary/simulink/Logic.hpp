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

    start = eIn         >> out(1)           >> 
      CALL(Logic::process) >> start
      ;
  }
protected:
 
  int logicOperator;
  bool inputsLogic[PORTS];

  void process() {   
    
    T output=0.0;

    output = in[0][0];
    switch(logicOperator)
    {
	
        case 0: /* AND */
           for( int i=1; i<PORTS; i++ )
		output = output && in[i][0];
	   break;
        case 1: /* OR */
           for( int i=1; i<PORTS; i++ )
		output = output || in[i][0];
	   break;
        case 2: /* NAND */
           for( int i=1; i<PORTS; i++ )
		output = output && in[i][0];
	   output = !output;
	   break;
        case 3: /* NOR */
           for( int i=1; i<PORTS; i++ )
		output = output || in[i][0];
	   output = !output;
	   break;
        case 4: /* XOR : true + true ---> false, true + false ---> true */
           for( int i=1; i<PORTS; i++ )
               if( output == in[i][0] )
		  output = 0;
               else
                  output = 1;
	   break;
        case 5: /* NXOR */
           for( int i=1; i<PORTS; i++ )
               if( output == in[i][0] )
		  output = 0;
               else
                  output = 1;
           output = !output;
	   break;
        case 6: /* NOT : Should have only one input */
           output = !in[0][0];
           //std::cout << "Logic: IN  :" << in[0][0] << " @ " << sc_time_stamp() << std::endl;
           //std::cout << "Logic: OUT :" << output  << " @ " << sc_time_stamp() << std::endl;
	   break;
        default:
	   break;
    }

    out[0] = (T)output;
    //cout << name() << " " << output << " - " << logicOperator << endl;
    
#ifdef _DEBUG    
    cout << " " << output << " ->" << endl;
#endif
  }

  smoc_firing_state start;

};

#endif// __INCLUDED__LOGIC__HPP__

