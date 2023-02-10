/*  Library : CFS adapter output
    Despcription : 
    Write simulation result of SysteMoC to shared memory.
*/


#ifndef __INCLUDED__CFSOUTPUTADAPTER__HPP__
#define __INCLUDED__CFSOUTPUTADAPTER__HPP__

#include <CoSupport/compatibility-glue/nullptr.h>

#include <cstdlib>
#include <iostream>
#include <systemoc/smoc_moc.hpp>
//#include <actorlibrary/tt/TT.hpp>



//For shared memory
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>



// Header file for socket API
#include <actorlibrary/tt/Socket.h>


template<typename T>
 class CFSOutputAdapter: public smoc_actor {
public:
  smoc_port_in<T>  in;
  //smoc_port_out<T>  out;
struct sembuf operation[1];
T Integration;


  CFSOutputAdapter( sc_module_name name, int _port )
    : smoc_actor(name, start),index(_port), sim_step() {
	

    start = 
      in(1)     >> //in (1)     >>
      CALL(CFSOutputAdapter::process) >> start
      ;
      
      // Create pointer for the shared memory
      key_t key_c;
      key_t semkey;
      key_t key_v;
      int shmid_c;
      int shmid_v;

	semkey = 001122334402;
	//semid = semget( semkey, NUMSEMS, 0666);
	//semid = semget(semkey, NUMSEMS, 0666 | IPC_CREAT);
semid = 163845;
Integration = 0.0;

      key_c = 001122334475; // simpler for me to harcode key instead of ftok()
      key_v = 001122334470; // simpler for me to harcode key instead of ftok()
      // create shared memory
      if ((shmid_c = shmget(key_c,SHM_size,0666 | IPC_CREAT )) == -1){
         perror ("shmget error");
    	 exit (1);
      }
      //printf("id is %d\n", shmid_c);
      // create shared memory
      if ((shmid_v = shmget(key_v,SHM_size,0666 | IPC_CREAT )) == -1){
         perror ("shmget error");
         exit (1);
      } 
      //printf("id is %d\n", shmid_v);
      // Obtain shared memory for control flag
      // attach pointer
      shm_c_ptr = (int*) shmat (shmid_c, nullptr, 0);
      if ((int)shm_c_ptr == -1){
         perror("shmat error");
         exit(1);
      }
      // Obtain shared memory for simulation result
      // attach pointer
      shm_v_ptr = (double*) shmat (shmid_v, nullptr, 0);
      if ((int)shm_v_ptr == -1){
         perror("shmat error");
         exit(1);
      }
  }

protected:

  int sim_step;
  int semid;

  void process() {
 	//this->resetEvent();


	#ifdef SHAREDMEMORY1
	shm_v_ptr[index] = in[0];
	// shm_c_ptr[0]++; // 
	//cout << "- " << index << " " << semid << " " << in[0]  << " " << shm_v_ptr[index] << endl;
	//if( index == 1 )
	operation[0].sem_num = index;//else operations[0].sem_num = 10;
	operation[0].sem_op  = 1;
	operation[0].sem_flg = IPC_NOWAIT;
        rc = semop( semid, operation, 1);
        if(rc==-1)
	{
		perror("Output Adapter: semop() failed");
		exit(1);	
	}
	//cout << "- " << index << endl;
	#endif


	#ifdef SHAREDMEMORYMULTIPORT
	shm_v_ptr[index] = in[0];
	shm_c_ptr[0]++;
	#endif
	
	#ifdef SHAREDMEMORY
        std::cerr << "Out> fired @ " << sc_time_stamp() << std::endl;
        shm_c_ptr[1] = sc_time_stamp().to_default_time_units() - shm_c_ptr[1];  // Computing delay
        cout << "Delay > " << shm_c_ptr[1] * 0.000001 << " ms\n" << endl;
        
	shm_v_ptr[1] = in[0];
	//std::cerr << "Out >" << shm_v_ptr[1] << endl;
	shm_c_ptr[0]++;
	//std::cerr << "Out E>" << std::endl << std::endl;
	
	
	#endif



	/*************** SharedMemory *********************/
        #ifdef SHAREDMEMORY4
        sc_time foo1(0, SC_NS);
	//while(shm_c_ptr[0]!=0) // Read
	//   sleep(0.0001);	
	//printf ("Simulation value: %5.5f ... Time Step: %d\n", shm_v_ptr[1], sim_step);
	//shm_v_ptr[1] = fabs(shm_v_ptr[1]); // TODO
	//Integration += in[0]*0.0001;
	//cout << Integration << endl;
	std::cerr << "Out::fired @ " << sc_time_stamp() << std::endl;
	shm_v_ptr[1] = in[0];
	//cout << shm_v_ptr[1] << endl;
	shm_c_ptr[0]++;
	
	sc_time t = sc_time_stamp();
  	sc_time foo2(0, SC_NS);
  	std::cerr << " ..... " << foo1-foo2 << " in ns: " << t.to_default_time_units() << std::endl;
	std::cerr << in[0] << std::endl;
	std::cout << "Out - Action finished\n" << std::endl;
	#endif
	
	
	/*************** Socket *********************/
	// Establish connection with the server
	#ifdef SOCKET
    	string servAddress = "131.188.52.38";
    	unsigned short ServPort = 5005;
    	TCPSocket sock(servAddress, ServPort);
    	int RCVBUFSIZE = 32;    // Size of receive buffer
    	char messageBuffer[RCVBUFSIZE];    // Buffer for socket the message

    	try {
           ostringstream sstream; // Need to covert double to string
	   sstream << in[0]; 
           string varAsString = sstream.str();
           strncpy(messageBuffer, varAsString.c_str(), RCVBUFSIZE/2); // Covert string to char array

           //for count
           //messageBuffer[16] = '4';
	   //messageBuffer[17] = '2';

           messageBuffer[RCVBUFSIZE-1] = '\0';  // ???
           sock.send(messageBuffer, RCVBUFSIZE); // Do we need a ACK echo???
           //cout << " send..." << in[0] << endl;
    	} catch(SocketException &e) {
		//cout << " send..." << in[0] << endl;
    		cerr << e.what() << endl;
    		exit(1);
  	}
	/********************************************/
	#endif
	


	//sim_step++;	
  }

  smoc_firing_state start;
  int index;
};

#endif // __INCLUDED__CFSOUTPUTADAPTER__HPP__

