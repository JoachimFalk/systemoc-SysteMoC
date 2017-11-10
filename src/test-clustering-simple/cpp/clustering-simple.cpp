#include <systemc.h>

#include <systemoc/smoc_moc.hpp>

#include "PgTop.hpp"

int sc_main(int argc, char *argv[]) {
#ifndef KASCPAR_PARSING
  PgTop pgTop("pgTop");
  smoc_scheduler_top _top(&pgTop);
  if (argc >= 2)
    sc_start(sc_time(atoi(argv[1]), SC_MS));
  else
    sc_start();
#endif // KASCPAR_PARSING
  return 0;
}

