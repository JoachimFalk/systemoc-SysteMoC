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


#define INAMEblk "test_in.dat"
#define ONAMEblk "test_out.dat"

class m_source_idct: public smoc_actor {
  public:
    smoc_port_out<int> out;
    smoc_port_out<int> min;
  private:
    int counter;
    std::ifstream i1; 
    
    void process() {
      int myMin;
      int myOut;
      
      
#ifndef NDEBUG
      if (i1.good()) {
#endif
        for ( int j = 0; j <= 63; j++ ) {
          counter++;
#ifdef NDEBUG
          myOut = counter;
#else
          i1 >> myOut;
          cout << name() << "  write " << myOut << std::endl;
#endif
          out[j] = myOut;
        }
        myMin = -256;
#ifndef NDEBUG
        cout << name() << "  write min " << myMin << std::endl;
#endif
        min[0] = myMin;
#ifndef NDEBUG
      } else {
        cout << "File empty! Please create a file with name test_in.dat!" << std::endl;
        exit (1) ;
      }
#endif

    }
    
    smoc_firing_state start;
  public:
    m_source_idct( sc_module_name name
#ifndef KASCPAR_PARSING
, size_t periods
#endif
)
      :smoc_actor( name, start ), counter(0) {
//#ifndef KASCPAR_PARSING
      i1.open(INAMEblk);
      start = ((out.getAvailableSpace() >= 64) &&
               (min.getAvailableSpace() >= 1) &&
               (VAR(counter) < periods*64))
              >> CALL(m_source_idct::process)
              >> start;
//#endif
  }
  ~m_source_idct( ){
//#ifndef KASCPAR_PARSING
        i1.close();
//#endif
  }
};

class m_sink: public smoc_actor {
  public:
    smoc_port_in<int> in;
  
  private:
//#ifndef KASCPAR_PARSING
    std::ofstream fo; 
//#endif
    int           foo;
    
    void process() {
#ifndef NDEBUG
      cout << name() << " receiving " << in[0] << std::endl;
      fo << in[0] << std::endl;
#else
      foo = in[0];
#endif
    }
    
    smoc_firing_state start;
  public:
    m_sink( sc_module_name name )
      : smoc_actor( name, start ),
        fo(ONAMEblk) {
      start = in(1) >> CALL(m_sink::process)  >> start;
    }
    
    ~m_sink() {
      fo.close();
    
    }
};

/*
class m_source: public smoc_actor {
  public:
    smoc_port_out<int> out;
  private:
    int i;
    const int step;

    //std::ifstream i1;

    void process() {
      //int data;
      //std::cout << name() << " generating " << i << std::endl;
     // if(i1.good()){
       // i1 >> data;
        out[0] = i;//data;
        cout << name() << "  write " << out[0] << std::endl;
        i+=step;
      //}else{
       // cout << "  file empty" << std::endl;
      //}
    }
    
    smoc_firing_state start;
  public:
    m_source( sc_module_name name,int init_value=0, int step=1)
      :smoc_actor( name, start ), i(init_value) , step(step){
      //i1.open("test.txt");
      start = ((out.getAvailableSpace() >= 1) && (VAR(i) <= 655)) >>
              CALL(m_source::process)               >> start;
    }
};
*/

class IDCT2d_TEST
: public smoc_graph {
private:
  m_source_idct src_idct;
  m_block_idct  blidct;
  m_sink        snk;
public:
  IDCT2d_TEST(sc_module_name name
#ifndef KASCPAR_PARSING
     , size_t periods
#endif
)
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
  bool generateProblemgraph =
    (argc > 1) && !strcmp(argv[1], "--generate-problemgraph");
  size_t periods            =
    (argc > 1 && !generateProblemgraph)
    ? atoi(argv[1])
    : 100;
  
  smoc_top_moc<IDCT2d_TEST> top("top", periods);
  
  if (generateProblemgraph) {
    smoc_modes::dump(std::cout, top);
  } else {  
    sc_start(-1);
  }
  return 0;
}
#endif


