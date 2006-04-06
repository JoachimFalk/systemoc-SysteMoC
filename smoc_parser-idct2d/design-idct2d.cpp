
#include "IDCT2d.hpp"

int sc_main (int argc, char **argv) {
  smoc_top_moc<HelperTop> top("top");

#ifndef KASCPAR_PARSING
  if (argc > 1 && 0 == strcmp(argv[1], "--generate-problemgraph")) {
    smoc_modes::dump(std::cout, top);
  } else {
    sc_start(-1);
  }
#endif
  return 0;
}
