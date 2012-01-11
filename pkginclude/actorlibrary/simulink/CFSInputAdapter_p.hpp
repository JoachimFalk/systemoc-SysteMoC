/*  Library : CFS input adapter
    Despcription : Read simulation results from Simulink via socket 
    
*/


#ifndef __INCLUDED__CFSINPUTADAPTER_P__HPP__
#define __INCLUDED__CFSINPUTADAPTER_P__HPP__

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

#include <math.h>


// Header file for socket API
#include <actorlibrary/tt/Socket.h>



template<typename T>
 class CFSInputAdapter_p: public PeriodicActor {
public:
  //smoc_port_in<T>  in;
  smoc_port_out<T>  out;
struct sembuf operation[1];

  CFSInputAdapter_p( sc_module_name name, sc_time per, sc_time off, EventQueue* eventQueue, int _port )
    : PeriodicActor(name, start, per, off, eventQueue),index(_port), sim_step() {
	

    start = Expr::till( this->getEvent() )  >>
      out(1)     >> //in (1)     >>
      CALL(CFSInputAdapter_p::process) >> start
      ;
      
      
      /*********** Communicate with Simulink S-function via SHAREDMEMORY **********/
      //Create pointer for the shared memory
      key_t key_c;
      key_t semkey;
      key_t key_v;

      int shmid_c;
      int shmid_v;





    semkey = 001122334401; // 5 sems
	//semkey = 001122334403;  11 sems
    //semid = semget(semkey, NUMSEMS, 0666); 666 (rw-rw-rw-) permissions
    //semid = semget(semkey, NUMSEMS, 0666 | IPC_CREAT);
semid = 131076;

      

      key_c = 001122334475; // simpler for me to harcode key instead of ftok()
      key_v = 001122334470; // simpler for me to harcode key instead of ftok()

      // This variable controls wann ends the systeMoC Simulation
      key_t key_c_end;
      int shmid_c_end;
      key_c_end = 001122334465; // simpler for me to harcode key instead of ftok()
      if ((shmid_c_end = shmget(key_c_end,SHM_size,0666 | IPC_CREAT )) == -1){
         perror ("shmget error");
    	 exit (1);
      }
      shm_c_ptr_end = (double*) shmat (shmid_c_end, NULL, 0);
      if ((int)shm_c_ptr_end == -1){
         perror("shmat error");
         exit(1);
      }


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
      shm_c_ptr = (int*) shmat (shmid_c, NULL, 0);
      if ((int)shm_c_ptr == -1){
         perror("shmat error");
         exit(1);
      }
      // Obtain shared memory for simulation result
      // attach pointer
      shm_v_ptr = (double*) shmat (shmid_v, NULL, 0);
      if ((int)shm_v_ptr == -1){
         perror("shmat error");
         exit(1);
      }
      /*******************************************************************************/
      
      
      /*********** Communicate with Simulink S-function via SOCKET **********/
      
      /**********************************************************************/
      
  }

protected:

  int sim_step;
  int semid;

  void process() {
 	this->resetEvent();



	#ifdef SHAREDMEMORY1
	//sleep(0.01);	
	//cout << semid << " " << index << " " << shm_v_ptr[index] ;
        
	operation[0].sem_num = index;
	operation[0].sem_op  = -1;
	operation[0].sem_flg = 0;
        rc = semop( semid, operation, 1);
        if(rc==-1) 
	{
		perror("Input Adpater: semop() failed during locking");
		exit(1);	
	}
	out[0] = shm_v_ptr[index];
	//cout << " " << shm_v_ptr[index] << endl;
	//cout << " " << shm_v_ptr[index] << endl;  
	//sleep(0.5);
	#endif


	#ifdef SHAREDMEMORYMULTIPORT
	//cout << shm_v_ptr[0] << endl;
	while(shm_c_ptr[0]!=0) 
		sleep(0.0000000000001);
		//sleep(0.001);		
	out[0] = shm_v_ptr[index];
	#endif
	
	#ifdef SHAREDMEMORY
	std::cerr << "In::fired @ " << sc_time_stamp() << std::endl;
	//cout << shm_v_ptr[0] << endl;
	//while(shm_c_ptr[0]!=0) 
	//	sleep(0.0000000000001);
		//sleep(0.001);		
	out[0] = shm_v_ptr[0];
	#endif

	#ifdef SHAREDMEMORY4	
	std::cerr << "In ::fired @ " << sc_time_stamp() << std::endl;
	//out[0] = shm_v_ptr[index];
	out[0] = 0.3;
	std::cout << "In - Action finished" << std::endl;
	#endif


	/*********** SHARED MEMORY ****************/
	#ifdef SHAREDMEMORY
        
	while(shm_c_ptr[0]!=0){ // Read
		//Uif(shm_c_ptr_end[0]>50.0) 
	     	{	
			//cout << "Simulation Surface: " << shm_c_ptr_end[1] << endl;
			//cout << "Simulation Flag: " << shm_c_ptr_end[0] << endl;
			//exit(0); // Simulink has finished its simulation	
		}
		sleep(0.0000000000001);	
	}
	//cout << "in[" << index << "] " << shm_v_ptr[index] << endl;
	//shm_v_ptr[1] = fabs(shm_v_ptr[1]); // TODO
	out[0] = shm_v_ptr[index];
	
	//cout << shm_v_ptr[1] << endl;
	//out[0] = shm_v_ptr[2];
	//shm_c_ptr[1] = 111; // 
	#endif
	
        //sleep(0.01);	
	
	/*************** Socket *********************/
	#ifdef SOCKET
	// Establish connection with the server
    	string servAddress = "131.188.52.38";
    	unsigned short ServPort = 5005;
    	TCPSocket sock(servAddress, ServPort);
    	int RCVBUFSIZE = 32;    // Size of receive buffer
    	char messageBuffer[RCVBUFSIZE];    // Buffer for socket the message
    	double inValue  = 0.0;
    	int bytesReceived = 0;              // Bytes read on each recv()
    	try {
    		//while (1) { // TODO: Why the whole program must stop for this single input??
    		   if ((bytesReceived = (sock.recv(messageBuffer, RCVBUFSIZE))) <= 0) {	
    		      cerr << "Unable to read";
        	      exit(1);	   
    		   }
    		   else
    		   {
    		      //inValue = atof(echoString); // Convert char array to double
    		      
	              //std::stringstream sstring( messageBuffer ); 
                      //cout << sstring << std::endl;
    		      //sstring >> inValue;
	              inValue = atof( messageBuffer );
		      //printf ("Incoming: %5.6f \n", inValue);
                      //if( inValue > 0 ) 
	              //   inValue = floor(inValue * 100.0) / 100.0; // rounded down
                      //else
                      //   inValue = (-1) * (floor(abs(inValue) * 100.0) / 100.0); // rounded down
		      //float nearest = floorf(val * 100 +  0.5) / 100;
		      //float rounded_up = ceilf(val * 100) / 100;

	              //printf("%.5f", inValue );
                      //sscanf(messageBuffer, "%f", &inValue);
    		      //break;
    		   }
    		
    		//}	
    	        out[0] = inValue;
    	        //cout << "Timestep " << sim_step <<  " recieved..." << inValue;
    	} catch(SocketException &e) {
	        cerr << "Wrong" << endl;
    		cerr << e.what() << endl;
    		exit(1);
  	}
	/********************************************/
        #endif

        //printf ("Simulation value: %5.6f \n", inValue);
	//sim_step++;	
  }

  smoc_firing_state start;
  int index;
};

#endif // __INCLUDED__CFSINPUTADAPTER_P__HPP__

