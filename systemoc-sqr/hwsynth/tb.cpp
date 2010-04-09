#include "tb.h"

void tb::source()
{
  hscd_float a(1.0);
  
  out.reset();
  
  rst.write(1);
  wait(5);
  rst.write(0);
  
  while(true) {
    while(out.tokens() < 1)
      wait(1);
    
    out.write(a);
    out.commit();
    
    a += 1.0;
  }
}

void tb::sink()
{
  std::stringstream str;
  std::ofstream log_file("sim_out.log");
  
  // reset
  in.reset();
  wait(1);

  for(int i = 0; i < 10; i++) {
    while(in.tokens() < 1)
      wait(1);
    
    str << in.read().to_double() << std::endl;
    std::cout << str.str();
    log_file  << str.str();
    str.str("");
    str.clear();
    
    in.commit();
  }
  
  // Sim is now done
  esc_stop();
}
