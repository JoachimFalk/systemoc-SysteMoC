#include "systemc.h"


SC_MODULE(Testmodule)
{
  sc_in<bool> clk;
  
  //typedef char[BYTES_PER_WORD] Teststring;
  
  void work()
  {
    Pancham pc;
    while (true)
    {
                  
      
      exit(EXIT_SUCCESS);
      wait();
    }
  }
  
  SC_CTOR( Testmodule )
  {
    SC_CTHREAD(work, clk.pos());    
  }
};

int sc_main(int argc, char * argv[])
{  
  sc_clock CLK("iCLK", sc_time(10, SC_NS));
  Testmodule tm("test_module");
  tm.clk(CLK);
  sc_start(sc_time(3, SC_MS));

  return EXIT_SUCCESS;
}
