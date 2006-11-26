// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

#include <img_src_1d.hpp>
#include <img_snk_1d.hpp>

using namespace std;

template <typename T>
class m_top
	: public smoc_graph {
public:
	m_top( sc_module_name name,
				 char* input_filename,
				 char* output_filename)
		: smoc_graph(name) {
		m_img_src_1d<T> &src = registerNode(new m_img_src_1d<T>("src",input_filename));
		m_img_snk_1d<T> &sink = registerNode(new m_img_snk_1d<T>("sink",output_filename,src.size_x,src.size_y));

#ifndef KASCPAR_PARSING
		connectNodePorts( src.out, sink.in, smoc_fifo<T>(src.size_x*src.size_y));
#endif
	}
};

int sc_main (int argc, char **argv) {
	if(argc < 3) {
		cout << "design-wsdf-recons-dila [input-file][output-file]" << endl;
		return -1;
	}

  smoc_top_moc<m_top<unsigned char> > top("top", argv[1],argv[2]);
  
#ifndef KASCPAR_PARSING  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {
    sc_start(-1);
  }
#undef GENERATE
#endif
  return 0;
}
