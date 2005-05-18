#include "systemc.h"
#ifndef P_STRUCT_H
#define P_STRUCT_H


typedef struct {
  char* name;
  int pid;
  sc_event* interupt;
  double delay;
  double remaining_delay;
  double priority;
  double period;
  double deadline;

}p_struct;


enum action_command { assign,resign,retire,add};

typedef struct{
  int target_pid;
  action_command command;
}action_struct;


#endif
