// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_md_port.hpp>
#include <systemoc/smoc_md_fifo.hpp>
#include <systemoc/smoc_wsdf_edge.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
# include <systemoc/smoc_pggen.hpp>
#endif

#include <wsdf_img_src.hpp>
#include <wsdf_img_snk.hpp>
#include <wsdf_dummy_src.hpp>
#include <wsdf_dummy_snk.hpp>

#define DUMMY_DATA
#define DUMMY_DATA_SIZE_X 4096
#define DUMMY_DATA_SIZE_Y 1714

using namespace std;
using namespace ns_smoc_vector_init;

template <typename T>
class m_top
	: public smoc_graph {
public:
	m_top( sc_module_name name,
				 char* input_filename,
				 char* output_filename)
		: smoc_graph(name) {
#ifdef DUMMY_DATA
		m_wsdf_dummy_src<T> &src = registerNode(new m_wsdf_dummy_src<T>("src",DUMMY_DATA_SIZE_X,DUMMY_DATA_SIZE_Y));
		m_wsdf_dummy_snk<T> &sink = registerNode(new m_wsdf_dummy_snk<T>("sink",src.size_x,src.size_y));
#else
		m_wsdf_img_src<T> &src = registerNode(new m_wsdf_img_src<T>("src",input_filename));
		m_wsdf_img_snk<T> &sink = registerNode(new m_wsdf_img_snk<T>("sink",output_filename,src.size_x,src.size_y));
#endif

#ifndef KASCPAR_PARSING
		//indConnectNodePorts( src.out, sink.in, smoc_wsdf_edge<T>(src.size_y));
		indConnectNodePorts( src.out, sink.in, smoc_wsdf_edge<T>(1));
#endif
	}
};

int sc_main (int argc, char **argv) {
#ifndef DUMMY_DATA
	if(argc < 3) {
		cout << "design-wsdf-recons-dila [input-file][output-file]" << endl;
		return -1;
	}
#endif

  smoc_top_moc<m_top<unsigned char> > top("top", argv[1],argv[2]);
  
#ifndef KASCPAR_PARSING  
#define GENERATE "--generate-problemgraph"
  if (argc > 1 && 0 == strncmp(argv[1], GENERATE, sizeof(GENERATE))) {
    smoc_modes::dump(std::cout, top);
  } else {
    sc_start();
  }
#undef GENERATE
#endif
  return 0;
}
