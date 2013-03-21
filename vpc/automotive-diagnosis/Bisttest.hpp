#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>

//BISTTEST
class Bisttest: public smoc_actor {
public:
  smoc_port_in<int> in;
  smoc_port_out<int> out;
private:

int received;
int testcost;
int result;
int payload1;
int payload2;
int payload3;
int payload4;
int payload5;



  void testingp1() {
//	 std::cout << name() << " BISTTEST received "<<this->payload1<<" Byte "<< std::endl;
	 received += this->payload1;
//	 std::cout << name() << " overall received data = "<<received<<" Byte "<<" time = "<< sc_time_stamp()<< std::endl;

  }
  void testingp2() {
//	 std::cout << name() << " BISTTEST received "<<this->payload2<<" Byte "<< std::endl;
	 received += this->payload2;
//	 std::cout << name() << " overall received data = "<<received<<" Byte "<<" time = "<< sc_time_stamp()<< std::endl;

  }
  void testingp3() {
//	 std::cout << name() << " BISTTEST received "<<this->payload3<<" Byte "<< std::endl;
	 received += this->payload3;
//	 std::cout << name() << " overall received data = "<<received<<" Byte "<<" time = "<< sc_time_stamp()<< std::endl;

  }
  void testingp4() {
//	 std::cout << name() << " BISTTEST received "<<this->payload4<<" Byte "<< std::endl;
	 received += this->payload4;
//	 std::cout << name() << " overall received data = "<<received<<" Byte "<<" time = "<< sc_time_stamp()<< std::endl;

  }
  void testingp5() {
//	 std::cout << name() << " BISTTEST received "<<this->payload5<<" Byte "<< std::endl;
	 received += this->payload5;
//	 std::cout << name() << " overall received data = "<<received<<" Byte "<<" time = "<< sc_time_stamp()<< std::endl;

  }
  void testComplete() {
//	 std::cout << name() << " BISTTEST complete! @ "<<" time = "<<sc_time_stamp()<< std::endl;
//	 std::cout << name() << " Write "<<this->result<<" Byte of results @ "<< sc_time_stamp()<< std::endl;
	 received = 0;
  }


//GUARD
    bool guardTestComplete() const {
// 	  std::cout<<"test complete ? received = "<<received<<" testcost - all payloads = "<<testcost-(payload1+payload2+payload3+payload4+payload5)<<std::endl;
  	  if(received > testcost-(payload1+payload2+payload3+payload4+payload5)){
  		  return true;
  	  }else{
  		  return false;
  	  }
    }



  smoc_firing_state test;
  smoc_firing_state end;

public:
  Bisttest(sc_module_name name, int testcost, int result,int payload1, int payload2, int payload3, int payload4, int payload5)
  : smoc_actor(name, test),received(0),testcost(testcost),result(result),payload1(payload1),payload2(payload2),payload3(payload3),payload4(payload4),payload5(payload5){


	 test = in(payload1)
    		>> CALL(Bisttest::testingp1)
    		>> test
    	| 	in(payload2)
			>> CALL(Bisttest::testingp2)
    		>> test
    	|   in(payload3)
	  	    >>CALL(Bisttest::testingp3)
	  	    >>test
	   	|   in(payload4)
		    >>CALL(Bisttest::testingp4)
		    >>test
		|   in(payload5)
		    >>CALL(Bisttest::testingp5)
		    >>test
		| GUARD(Bisttest::guardTestComplete)
			>>out(result)
			>>CALL(Bisttest::testComplete)
			>>end;

  }
};
