//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

#include "callib.hpp"

#include "block_idct.hpp"
#include "IDCTsource.hpp"
#include "IDCTsink.hpp"


#ifndef DEFAULT_BLOCK_COUNT
#define DEFAULT_BLOCK_COUNT 25
#endif

class IDCT2d_TEST
: public smoc_graph {
private:
  m_source_idct src_idct;
  m_block_idct  blidct;
  m_sink        snk;
public:
  IDCT2d_TEST(sc_module_name name, size_t periods)
    : smoc_graph(name),
      src_idct("src_idct", periods),
      blidct("blidct"),
      snk("snk") {
#ifndef KASCPAR_PARSING
    connectNodePorts( src_idct.out, blidct.I,   smoc_fifo<int>(128));
    connectNodePorts( src_idct.min, blidct.MIN, smoc_fifo<int>(4));
    connectNodePorts( blidct.O, snk.in, smoc_fifo<int>(128));
#endif
  }
};

#ifndef KASCPAR_PARSING
int sc_main (int argc, char **argv) {
  size_t periods            =
    (argc > 1)
    ? atoi(argv[1])
    : DEFAULT_BLOCK_COUNT;
  
  smoc_top_moc<IDCT2d_TEST> top("top", periods);
  
  sc_start(-1);
  
  return 0;
}
#endif
