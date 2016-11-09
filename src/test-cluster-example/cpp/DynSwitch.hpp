// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#ifndef _INCLUDED_DYN_SWITCH
#define _INCLUDED_DYN_SWITCH

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_actor.hpp>

#include <CoSupport/commondefs.h>

#ifndef XILINX_EDK_RUNTIME
#include <iostream>
#else
#endif

class DynSwitch
  : public smoc_actor {

public:
  smoc_port_in<int> in1;
  smoc_port_in<int> in2;

  smoc_port_out<int> out1;
  smoc_port_out<int> out2;

private:

  //This variable is used in order to decide
  //with path to go
  unsigned long path_selector;

  //The number of the current invocation
  unsigned long invocation_id;


  unsigned long int_pow(unsigned long x, unsigned int power){
    unsigned long return_value = 1;

    for(unsigned int i = 1; i <= power; i++){
      return_value *= x;
    }

    return return_value;
  }

  void calc_next_selector(){
    invocation_id++;

    path_selector = invocation_id % 7;
    path_selector += invocation_id % 3;

    path_selector = path_selector % 2;

#ifndef NDEBUG
#ifndef KASCPAR_PARSING
#ifndef XILINX_EDK_RUNTIME
    std::cout << "Next path_selector = " << path_selector << std::endl;
#else
    xil_printf("Next path_selector = %u\r\n",path_selector);
#endif
#endif
#endif
  }

  //Actions
  //The names correspond to the transitions
  //in Figure 4 of the DAC2008 paper
  void action_t1(){
#ifndef NDEBUG
#ifndef KASCPAR_PARSING
#ifndef XILINX_EDK_RUNTIME
    std::cout << "I am action_t1" << std::endl;
    std::cout << "Starting invocation " << invocation_id << std::endl;    
#else
    xil_printf("I am action_t1\n\r");
    xil_printf("Starting invocation %u\n\r", invocation_id);
#endif
#endif
#endif

    int dummy COSUPPORT_ATTRIBUTE_UNUSED = in2[0];
    out1[0] = 0;
    out1[1] = 1;

  }

  void action_t1_init(){
#ifndef NDEBUG
#ifndef KASCPAR_PARSING
#ifndef XILINX_EDK_RUNTIME
    std::cout << "I am action_t1_init" << std::endl;
    std::cout << "Starting invocation " << invocation_id << std::endl;
#else
    xil_printf("I am action_t1_init\n\r");
    xil_printf("Starting invocation %u\n\r",invocation_id);
#endif
#endif
#endif

    out1[0] = 0;
    out1[1] = 1;
  }

  void action_t2(){
#ifndef NDEBUG
#ifndef KASCPAR_PARSING
#ifndef XILINX_EDK_RUNTIME
    std::cout << "I am action_t2" << std::endl;
    std::cout << "Starting invocation " << invocation_id << std::endl;
#else
    xil_printf("I am action_t2\n\r");
    xil_printf("Starting invocation %u\r\n",invocation_id);
#endif
#endif
#endif

    int dummy COSUPPORT_ATTRIBUTE_UNUSED = in2[0];
    out2[0] = 0;
  }

  void action_t3(){
#ifndef NDEBUG
#ifndef KASCPAR_PARSING
#ifndef XILINX_EDK_RUNTIME
    std::cout << "I am action_t3" << std::endl;
#else
    xil_printf("I am action_t3\n\r");
#endif
#endif
#endif

    //update path selector
    calc_next_selector();

    int dummy COSUPPORT_ATTRIBUTE_UNUSED = in1[0]+in1[1];
    out2[0] = 0;
  }

  void action_t4(){
#ifndef NDEBUG
#ifndef KASCPAR_PARSING
#ifndef XILINX_EDK_RUNTIME
    std::cout << "I am action_t4" << std::endl;
#else
    xil_printf("I am action_t4\n\r");
#endif
#endif
#endif

    //update path selector
    calc_next_selector();

    int dummy COSUPPORT_ATTRIBUTE_UNUSED = in2[0];
    out1[0] = 0;
    out1[1] = 1;
  }
  
  void action_t5_path1(){
#ifndef NDEBUG
#ifndef KASCPAR_PARSING
#ifndef XILINX_EDK_RUNTIME
    std::cout << "I am action_t5" << std::endl;
#else
    xil_printf("I am action_t5\n\r");
#endif
#endif
#endif

    int dummy COSUPPORT_ATTRIBUTE_UNUSED = in2[0];
    out2[0] = 0;
  }

  void action_t5_path2(){
#ifndef NDEBUG
#ifndef KASCPAR_PARSING
#ifndef XILINX_EDK_RUNTIME
    std::cout << "I am action_t5" << std::endl;
#else
    xil_printf("I am action_t5\n\r");
#endif
#endif
#endif

    int dummy COSUPPORT_ATTRIBUTE_UNUSED = in1[0] + in1[1];
    out2[0] = 0;
  }


  //We divide the firing state machine into
  //two paths leading through the static actor  
  //The names correspond to the states shown in Fig. 4
  //of the DAC2008 paper

  smoc_firing_state fsm_start;   //Start state in order to avoid
                                 //initial token
  smoc_firing_state fsm_path1_q0;
  smoc_firing_state fsm_path1_q1;
  smoc_firing_state fsm_path1_q3;
  smoc_firing_state fsm_path2_q0;
  smoc_firing_state fsm_path2_q2;
  smoc_firing_state fsm_path2_q3;

  smoc_firing_state stuck;

public:
  DynSwitch(sc_core::sc_module_name name,
            unsigned long num_iterations)
    : smoc_actor(name,fsm_start),
      path_selector(0),
      invocation_id(0)
  {

    SMOC_REGISTER_CPARAM(num_iterations);

    /* Initial state in order to avoid initial tokens */
    fsm_start = 
      out1(2) >>
      CALL(DynSwitch::action_t1_init) >>
      fsm_path1_q1;

    /* Path1 through static cluster */
    fsm_path1_q1 =
      (in1(2) && out2(1)) >>
      CALL(DynSwitch::action_t3) >>
      fsm_path1_q3;

    fsm_path1_q3 =
      (in2(1) && out2(1)) >>
      ((VAR(path_selector) <= (unsigned long)0) && (VAR(invocation_id) < num_iterations)) >>
      CALL(DynSwitch::action_t5_path1) >>
      fsm_path1_q0

      |(in2(1) && out2(1)) >>
      ((VAR(path_selector) > (unsigned long)0) && (VAR(invocation_id) < num_iterations)) >>
      CALL(DynSwitch::action_t5_path1) >>
      fsm_path2_q0

      |(in2(1) && out2(1)) >>
      (VAR(invocation_id) >= num_iterations) >>
      CALL(DynSwitch::action_t5_path1) >>
      stuck;

    fsm_path1_q0 =
      (in2(1) && out1(2)) >>
      CALL(DynSwitch::action_t1) >>
      fsm_path1_q1;

    /* Path2 through static cluster */
    fsm_path2_q0 =
      (in2(1) && out2(1)) >>
      CALL(DynSwitch::action_t2) >>
      fsm_path2_q2;

    fsm_path2_q2 =
      (in2(1) && out1(2)) >>
      CALL(DynSwitch::action_t4) >>
      fsm_path2_q3;

    fsm_path2_q3 =
      (in1(2) && out2(1)) >>
      ((VAR(path_selector) <= (unsigned long)0) && (VAR(invocation_id) < num_iterations)) >>
      CALL(DynSwitch::action_t5_path2) >>
      fsm_path1_q0

      |(in1(2) && out2(1)) >>
      ((VAR(path_selector) > (unsigned long)0) && (VAR(invocation_id) < num_iterations))>>
      CALL(DynSwitch::action_t5_path2) >>
      fsm_path2_q0

      |(in1(2) && out2(1)) >>
      (VAR(invocation_id) < num_iterations)>>
      CALL(DynSwitch::action_t5_path2) >>
      stuck;

  }


};

#endif
