#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>

//TESTANALYSER
class Testanalyser: public smoc_actor {
public:
  smoc_port_in<int> in;
private:
int received;
int result;
int payload1;
int payload2;
int payload3;
int payload4;
int payload5;


  bool guardResultComplete() const {
	  if(received > result-(payload1+payload2+payload3+payload4+payload5)){
		  return true;
	  }else{
		  return false;
	  }
  }
  void result1() {
//	 std::cout << name() << " Testanalyser received "<<this->payload1<<" Byte "<< std::endl;
	 received += this->payload1;
//	 std::cout << name() << " overall received data = "<<received<<" Byte Bytes"<<" time = "<< sc_time_stamp()<< std::endl;

  }
  void result2() {
//	 std::cout << name() << " Testanalyser received "<<this->payload2<<" Byte "<< std::endl;
	 received += this->payload2;
//	 std::cout << name() << " overall received data = "<<received<<" Byte time = "<< sc_time_stamp()<< std::endl;
  }
  void result3() {
//	 std::cout << name() << " Testanalyser received "<<this->payload3<<" Byte "<< std::endl;
	 received += this->payload3;
//	 std::cout << name() << " overall received data = "<<received<<" Byte time = "<< sc_time_stamp()<< std::endl;
  }
  void result4() {
//	 std::cout << name() << " Testanalyser received "<<this->payload4<<" Byte "<< std::endl;
	 received += this->payload4;
//	 std::cout << name() << " overall received data = "<<received<<" Byte time = "<< sc_time_stamp()<< std::endl;
  }
  void result5() {
//	 std::cout << name() << " Testanalyser received "<<this->payload5<<" Byte "<< std::endl;
	 received += this->payload5;
//	 std::cout << name() << " overall received data = "<<received<<" Byte time = "<< sc_time_stamp()<< std::endl;
  }
  void resultComplete() {
	 std::cout << name() << " Result complete! @ "<<" time = "<<sc_time_stamp()<< std::endl;
	 std::cout << name() << " Test successful @ "<< sc_time_stamp()<< std::endl;
	 received = 0;
  }

  smoc_firing_state test;
  smoc_firing_state end;

public:
  Testanalyser(sc_module_name name, int result, int payload1, int payload2, int payload3, int payload4, int payload5)
    : smoc_actor(name, test),received(0),result(result),payload1(payload1),payload2(payload2),payload3(payload3),payload4(payload4),payload5(payload5){

	  test =
    		in(payload1)
    		>> CALL(Testanalyser::result1)
    		>> test
    	|
    		in(payload2)
    		>> CALL(Testanalyser::result2)
    		>> test
    	|
	  	    in(payload3)
	  	    >>CALL(Testanalyser::result3)
	  	    >>test
	   	|
		    in(payload4)
		    >>CALL(Testanalyser::result4)
		    >>test
		|
		    in(payload5)
		    >>CALL(Testanalyser::result5)
		    >>test
		| GUARD(Testanalyser::guardResultComplete)
			>>CALL(Testanalyser::resultComplete)
			>>end;
  }
};
