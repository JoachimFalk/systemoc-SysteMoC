#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>

//DEMULTIPLEXER
class Demux: public smoc_actor {
public:
	smoc_port_in<int> in1;
	smoc_port_in<int> in2;
	smoc_port_in<int> in3;
	smoc_port_in<int> in4;
	smoc_port_in<int> in5;
	smoc_port_out<int> out;

private:
int payload1;
int payload2;
int payload3;
int payload4;
int payload5;

  void in1Data() {
//	  std::cout << name() << " DEMULTIPLEXER received 1 Frame with size of "<<this->payload1<<" Byte "<< std::endl;
//	  std::cout << name() << "Time = "<< sc_time_stamp() << std::endl;
  }
  void in2Data() {
//	  std::cout << name() << " DEMULTIPLEXER received 1 Frame with size of "<<this->payload2 <<" Byte "<< std::endl;
//	  std::cout << name() << "Time = "<< sc_time_stamp() << std::endl;
  }
  void in3Data() {
//	  std::cout << name() << " DEMULTIPLEXER received 1 Frame with size of "<<this->payload3<<" Byte " <<std::endl;
//	  std::cout << name() << "Time = "<< sc_time_stamp() << std::endl;
  }
  void in4Data() {
//	  std::cout << name() << " DEMULTIPLEXER received 1 Frame with size of "<<this->payload4<<" Byte " <<std::endl;
//	  std::cout << name() << "Time = "<< sc_time_stamp() << std::endl;
  }
  void in5Data() {
//	  std::cout << name() << " DEMULTIPLEXER received 1 Frame with size of "<<this->payload5<<" Byte " <<std::endl;
//	  std::cout << name() << "Time = "<< sc_time_stamp() << std::endl;
  }
  smoc_firing_state demultiplex;
public:
  Demux(sc_module_name name, int payload1, int payload2, int payload3, int payload4, int payload5)
    : smoc_actor(name, demultiplex),payload1(payload1),payload2(payload2),payload3(payload3),payload4(payload4),payload5(payload5){


	  demultiplex = 	(in1(1) && out(payload1))
							  >> CALL(Demux::in1Data)
							  >> demultiplex
					  | (in2(1) && out(payload2))
					  	  	  >> CALL(Demux::in2Data)
					  	  	  >> demultiplex
					  | (in3(1) && out(payload3))
					  	  	  >> CALL(Demux::in3Data)
					  	  	  >> demultiplex
					  | (in4(1) && out(payload4))
					  	  	  >> CALL(Demux::in4Data)
					  	  	  >> demultiplex
					  | (in5(1) && out(payload5))
						  	  >> CALL(Demux::in5Data)
						  	  >> demultiplex;
  }
};
