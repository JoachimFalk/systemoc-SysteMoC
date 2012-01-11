/*  Library : CFS adapter
    Despcription : Read inputs of Simulink simulation from shared memory; And write the outputs of SysteMoC simulation to shared memory
*/


#ifndef __INCLUDED__CFSADAPTER_P__HPP__
#define __INCLUDED__CFSADAPTER_P__HPP__

#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
#include <actorlibrary/tt/TT.hpp>



//For shared memory
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>

// macro definitions
#define SHM_size 10240 // size of shared memory segment
int *shm_scan_ptr;

template<typename T>
 class CFSAdapter_p: public PeriodicActor {
public:
  //smoc_port_in<T>  in;
  smoc_port_out<T>  out;

  CFSAdapter_p( sc_module_name name, sc_time per, sc_time off, EventQueue* eventQueue, T _init )
    : PeriodicActor(name, start, per, off, eventQueue),init(_init), sim_step() {
	

    start = Expr::till( this->getEvent() )  >>
      out(1)     >> //in (1)     >>
      CALL(CFSAdapter_p::process) >> start
      ;
  }

protected:

  int sim_step;

  void process() {
 	this->resetEvent();

	key_t key;
	int shmid;
	key = 001122334455; // simpler for me to harcode key instead of ftok()
	// connect
	if ((shmid = shmget(key,SHM_size,0666 | IPC_CREAT )) == -1){
		perror ("shmget error");
		exit (1);
	}
	//printf("id is %d\n", shmid);
	// attach pointer
	shm_scan_ptr = (int*) shmat (shmid, NULL, 0);
	if ((int)shm_scan_ptr == -1){
		perror("shmat error");
		exit(1);
	}
	//printf("Writing data to shared memory...\n");
	//while(1){
		//std::cin >> shm_scan_ptr[1];
		shm_scan_ptr[2] = sim_step;
		shm_scan_ptr[3] = sim_step%19;
		std::cout<<"Simulation step:"<< shm_scan_ptr[2] << " " << shm_scan_ptr[3] <<std::endl;
	//}

	sim_step++;	
  }

  smoc_firing_state start;
  T init;
};

#endif // __INCLUDED__CFSADAPTER_P__HPP__

