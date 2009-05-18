// vim: set sw=2 ts=8:

#include <cstdlib>
#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_node_types.hpp>

#include <img_src_1d.hpp>
#include <img_snk_1d.hpp>

#include <img_block_src_1d.hpp>
#include <img_block_snk_1d.hpp>

#include <dummy_src_1d.hpp>
#include <dummy_snk_1d.hpp>

#include <dummy_block_src_1d.hpp>
#include <dummy_block_snk_1d.hpp>

//#define BLOCK_OPERATION
#define DUMMY_DATA
#define DUMMY_DATA_SIZE_X 4096
#define DUMMY_DATA_SIZE_Y 1714

using namespace std;

template <typename T>
class m_top
	: public smoc_graph {
public:
	m_top( sc_module_name name,
				 char* input_filename,
				 char* output_filename)
		: smoc_graph(name) {
#ifdef BLOCK_OPERATION
# ifdef DUMMY_DATA
		m_dummy_block_src_1d<T> &src = registerNode(new m_dummy_block_src_1d<T>("src",DUMMY_DATA_SIZE_X,DUMMY_DATA_SIZE_Y));
		m_dummy_block_snk_1d<T> &sink = registerNode(new m_dummy_block_snk_1d<T>("sink",src.size_x,src.size_y));
# else
		m_img_block_src_1d<T> &src = registerNode(new m_img_block_src_1d<T>("src",input_filename));
		m_img_block_snk_1d<T> &sink = registerNode(new m_img_block_snk_1d<T>("sink",output_filename,src.size_x,src.size_y));
# endif
#else
# ifdef DUMMY_DATA
		m_dummy_src_1d<T> &src = registerNode(new m_dummy_src_1d<T>("src",DUMMY_DATA_SIZE_X,DUMMY_DATA_SIZE_Y));
		m_dummy_snk_1d<T> &sink = registerNode(new m_dummy_snk_1d<T>("sink",src.size_x,src.size_y));
# else
		m_img_src_1d<T> &src = registerNode(new m_img_src_1d<T>("src",input_filename));
		m_img_snk_1d<T> &sink = registerNode(new m_img_snk_1d<T>("sink",output_filename,src.size_x,src.size_y));
# endif
#endif

#ifndef KASCPAR_PARSING
		connectNodePorts( src.out, sink.in, smoc_fifo<T>(src.size_x*src.size_y));
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
  
  sc_start();

  return 0;
}
