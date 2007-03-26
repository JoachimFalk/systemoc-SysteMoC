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

#define REAL_BLOCK_DATA
#define IMAGE_WIDTH  648
#define IMAGE_HEIGHT 408

//#define VERBOSE_ACTOR

#include "block_idct.hpp"


#ifndef REAL_BLOCK_DATA
# include "IDCTsource.hpp"
# include "IDCTsink.hpp"
#else
# include "IDCT_block_source.hpp"
# include "IDCT_block_sink.hpp"
#endif


#ifndef DEFAULT_BLOCK_COUNT
# ifdef REAL_BLOCK_DATA
#  define DEFAULT_BLOCK_COUNT ((IMAGE_WIDTH)/8*(IMAGE_HEIGHT)/8)
# else
#  define DEFAULT_BLOCK_COUNT 25
# endif
#endif

class IDCT2d_TEST
: public smoc_graph {
private:
#ifndef REAL_BLOCK_DATA
  m_source_idct src_idct;  
  m_sink        snk;
#else
  m_block_source_idct src_idct;
  m_block_sink        snk;
#endif
  m_block_idct  blidct;
public:
  IDCT2d_TEST(sc_module_name name, size_t periods)
    : smoc_graph(name),
      src_idct("src_idct", periods),
#ifdef REAL_BLOCK_DATA
      snk("snk",IMAGE_WIDTH, IMAGE_HEIGHT) ,
#else
      snk("snk"),
#endif
      blidct("blidct")
{
#ifndef KASCPAR_PARSING
    connectNodePorts( src_idct.out, blidct.I,   smoc_fifo<int>(128));
    connectNodePorts( src_idct.min, blidct.MIN, smoc_fifo<int>(4));
# ifndef REAL_BLOCK_DATA
    connectNodePorts( blidct.O, snk.in, smoc_fifo<int>(128));
# else
    connectNodePorts( blidct.O, snk.in, smoc_fifo<int>(IMAGE_WIDTH/8*64));
# endif
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
