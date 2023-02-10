
#include <cstdlib>
#include <iostream>

class PeriodicTask
  : public smoc_actor {
  SC_HAS_PROCESS(PeriodicTask);
public:

  PeriodicTask(sc_module_name name, sc_time period, size_t iterations)
    : smoc_actor( name, main ),
      period( period ),
      iterations( iterations )
  {

    SC_THREAD(time_out_process);
  
    main =
        Expr::till( event )            >>
        CALL(PeriodicTask::process)  >> main
      ;
  }

  smoc_event event;

private:

  void time_out_process() {
    for ( int i = iterations; i > 0; --i ) {
      wait( period );
      smoc_notify( event );
      //std::cout << "timeout send" << std::endl;
    }
  }

  void process()  {

    smoc_reset( event );

    std::cout << this->name() << "> released @ " << sc_time_stamp()
              << std::endl;
    
  }

  sc_time period;

  size_t iterations;

  smoc_firing_state main;
};

