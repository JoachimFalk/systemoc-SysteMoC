#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>

//MULTIPLEXER
class Mux: public smoc_periodic_actor {
public:
  smoc_port_in<int> in;
  smoc_port_out<int> out1;
  smoc_port_out<int> out2;
  smoc_port_out<int> out3;
  smoc_port_out<int> out4;
  smoc_port_out<int> out5;
private:
bool inputFromSource;
int iter;
int send;
int valid1;
int valid2;
int valid3;
int valid4;
int valid5;
int testcost;
int result;
int period1;
int payload1;
int period2;
int payload2;
int period3;
int payload3;
int period4;
int payload4;
int period5;
int payload5;

  void reexecute(void){
	  this->forceReexecution();
  }

  void iterate(void){
	  iter++;
//	  std::cout<<"iterate!!!!!!!"<<std::endl;
  }

  void sendOut1(void){
//	  std::cout<<name()<<" read "<<this->payload1<<" Byte"<<std::endl;
	  valid1 += this->period1;
	  send += this->payload1;
	  reexecute();
  }

  void sendOut2(void){
//	  std::cout<<name()<<" read "<<this->payload2<<" Byte"<<std::endl;
	  valid2 += this->period2;
	  send += this->payload2;
	  reexecute();
  }

  void sendOut3(void){
//	  std::cout<<name()<<" read "<<this->payload3<<" Byte"<<std::endl;
	  valid3 += this->period3;
	  send += this->payload3;
	  reexecute();
  }
  void sendOut4(void){
//	  std::cout<<name()<<" read "<<this->payload4<<" Byte"<<std::endl;
	  valid4 += this->period4;
	  send += this->payload4;
	  reexecute();
  }
  void sendOut5(void){
//	  std::cout<<name()<<" read "<<this->payload5<<" Byte"<<std::endl;
	  valid5 += this->period5;
	  send += this->payload5;
	  reexecute();
  }
  void report(void){
//		std::cout<<"available Tokens at input port = "<<in.numAvailable()<<std::endl;
//		std::cout<<"free space in out1 =  "<<out1.numFree()<<std::endl;
//		std::cout<<"free space in out2 =  "<<out2.numFree()<<std::endl;
//		std::cout<<"free space in out3 =  "<<out3.numFree()<<std::endl;
//		std::cout<<"free space in out4 =  "<<out4.numFree()<<std::endl;
//		std::cout<<"free space in out5 =  "<<out5.numFree()<<std::endl;
//		std::cout<<"valid1 = "<<valid1<<std::endl;
//		std::cout<<"valid2 = "<<valid2<<std::endl;
//		std::cout<<"valid3 = "<<valid3<<std::endl;
//		std::cout<<"valid4 = "<<valid4<<std::endl;
//		std::cout<<"valid5 = "<<valid5<<std::endl;
//		std::cout<<"iter = "<<iter<<std::endl;
//	  std::cout<<"send = "<<send<<std::endl;
  }

  void stop(void){
//	  std::cout<<"STOP!!!"<<std::endl;
	  stopPeriodicActorExecution();
  }


  	  // GUARDS
     bool enableOut1 () const {
//   	 std::cout<<"enableOut1 " <<" num available at in = "<<in.numAvailable()<< std::endl;

     	if((iter == valid1) && (this->period1 != -1)){
//      		std::cout<<"enableOut1 = true "<<sc_time_stamp()<<" valid1 = "<< valid1<<std::endl;
//      		std::cout<<"available Tokens at input port = "<<in.numAvailable()<<std::endl;
//      		std::cout<<"free space in out1 =  "<<out1.numFree()<<std::endl;
      		return true;
      	}else{
//      		std::cout<<"enableOut1 = false "<<sc_time_stamp()<<" valid1 = "<< valid1<<std::endl;
      		return false;
      	}
      }

      bool enableOut2 () const {
//    	  std::cout<<"enableOut2 " << " num available at in = "<<in.numAvailable()<<std::endl;
      	if((iter == valid2) && (this->period2 != -1)){
// 		std::cout<<"enableOut2 = true "<<sc_time_stamp()<< " valid2 = "<< valid2<<std::endl;
      		return true;
      	}else{
//      		std::cout<<"enableOut2 = false "<<sc_time_stamp()<<" valid2 = "<< valid2<<std::endl;
      		return false;
      	}
      }

      bool enableOut3 () const {
//  	  std::cout<<"enableOut3 " <<" num available at in = "<<in.numAvailable()<< std::endl;
      	if((iter == valid3) && (this->period3 != -1)){
//      		std::cout<<"enableOut3 = true "<<sc_time_stamp()<< " valid3 = "<< valid3<<std::endl;
      		return true;
      	}else{
//      		std::cout<<"enableOut3 = false "<<sc_time_stamp()<<" valid3 = "<< valid3<<std::endl;
      		return false;
      	}
      }

      bool enableOut4 () const {
//   	  std::cout<<"enableOut4 " << " num available at in = "<<in.numAvailable()<<std::endl;
      	if((iter == valid4) && (this->period4 != -1)){
//     		std::cout<<"enableOut3 = true "<<sc_time_stamp()<< " valid4 = "<< valid4<<std::endl;
      		return true;
      	}else{
//      		std::cout<<"enableOut3 = false "<<sc_time_stamp()<<" valid4 = "<< valid4<<std::endl;
      		return false;
      	}
      }
      bool enableOut5 () const {
//   	  std::cout<<"enableOut5 " << " num available at in = "<<in.numAvailable()<<std::endl;
      	if((iter == valid5)  && (this->period5 != -1)){
//     		std::cout<<"enableOut3 = true "<<sc_time_stamp()<< " valid5 = "<< valid5<<std::endl;
      		return true;
      	}else{
//			std::cout<<"enableOut3 = false "<<sc_time_stamp()<<" valid5 = "<< valid5<<std::endl;
      		return false;
      	}
      }

      bool guardMuxComplete() const {
//    	  std::cout<<"mux complete ? send = "<<send<<" testcost - 10 = "<<testcost-10<<std::endl;
    	  if(inputFromSource){
    	  if(send >= testcost-10){
//  	  std::cout<<"guardMuxComplete = false"<<std::endl;
    		  return false;
    	  }else{
//    		  std::cout<<"guardMuxComplete = true"<<std::endl;
    		  return true;
    	  }
    	  }
    	  else if(!inputFromSource){
    		  if(send >= result-10){
//    		  std::cout<<"guardMuxComplete = false"<<std::endl;
    		      return false;
    		  }else{
//    			  std::cout<<"guardMuxComplete = true"<<std::endl;
    		      return true;
    		  }
    	  }
    	  return true;
      }



  smoc_firing_state multiplex;
  smoc_firing_state end;

public:
  Mux(sc_module_name name,bool inputFromSource, int testcost, int result, int period1, int payload1, int period2, int payload2, int period3, int payload3, int period4, int payload4, int period5, int payload5)
    : smoc_periodic_actor(name, multiplex, sc_time(1,SC_MS), SC_ZERO_TIME),inputFromSource(inputFromSource),send(0),testcost(testcost),result(result),valid1(period1),valid2(period2),valid3(period3),valid4(period4),valid5(period5),iter(1),period1(period1),payload1(payload1),period2(period2),payload2(payload2),period3(period3),payload3(payload3),period4(period4),payload4(payload4),period5(period5),payload5(payload5){

	  	  	  multiplex = 	(GUARD(Mux::enableOut1)
	  	  			  	  	  	&& GUARD(Mux::guardMuxComplete))
	  	  			  	  	  	>> (in(payload1) && out1(1))
	  	  						>> CALL(Mux::sendOut1)
	  	  						>> multiplex
	  	  				| 	(GUARD(Mux::enableOut2)
	  	  						&& GUARD(Mux::guardMuxComplete))
	  	  						>> (in(payload2) && out2(1))
	  	  						>> CALL(Mux::sendOut2)
	  	  						>> multiplex
						| 	(GUARD(Mux::enableOut3)
								&& GUARD(Mux::guardMuxComplete))
								>> (in(payload3) && out3(1))
								>> CALL(Mux::sendOut3)
								>> multiplex
						| 	(GUARD(Mux::enableOut4)
								&& GUARD(Mux::guardMuxComplete))
								>> (in(payload4) && out4(1))
								>> CALL(Mux::sendOut4)
								>> multiplex
						| 	(GUARD(Mux::enableOut5)
								&& GUARD(Mux::guardMuxComplete))
								>> (in(payload5) && out5(1))
								>> CALL(Mux::sendOut5)
								>> multiplex
						| 	(!GUARD(Mux::enableOut2)
							&& !GUARD(Mux::enableOut1)
							&& !GUARD(Mux::enableOut3)
							&& !GUARD(Mux::enableOut4)
							&& !GUARD(Mux::enableOut5)
							&& GUARD(Mux::guardMuxComplete))
								>> CALL(Mux::iterate)
								>> CALL(Mux::report)
								>> multiplex
	  	  	  	  	  	|	(!GUARD(Mux::guardMuxComplete))
	  	  	  	  	  		>> CALL(Mux::stop)
	  	  			  	  	  	>> end;

  }
};
