// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_md_port.hpp>
#include <smoc_md_fifo.hpp>
#include <smoc_wsdf_edge.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

#include <wsdf_img_src.hpp>
#include <wsdf_img_snk.hpp>

using namespace std;
using namespace ns_smoc_vector_init;

class m_top
	: public smoc_graph {
public:
	m_top( sc_module_name name,
				 char* input_filename,
				 char* output_filename)
		: smoc_graph(name) {
		m_wsdf_img_src &src = registerNode(new m_wsdf_img_src("src",input_filename));
		m_wsdf_img_snk &sink = registerNode(new m_wsdf_img_snk("sink",output_filename,src.size_x,src.size_y));

#ifndef KASCPAR_PARSING
		indConnectNodePorts( src.out, sink.in, smoc_wsdf_edge<int>(1));
#endif
	}
};

int sc_main (int argc, char **argv) {
	if(argc < 2) {
		cout << "design-wsdf-recons-dila [input-file][output-file]" << endl;
		return -1;
	}

  smoc_top_moc<m_top> top("top", argv[1],argv[2]);
  
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
