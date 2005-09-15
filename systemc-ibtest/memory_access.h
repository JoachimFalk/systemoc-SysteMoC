#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>

#include <tt_ib.h>
#include <packet_header.h>
#include <generic_communication_node.hpp>

using Expr::field;
using std::endl;
using std::cout;

template <int chunksize>
class chunker: public smoc_actor {

typedef data_chunk<chunksize> d_chunk;


 public:
  smoc_port_in<memory_request>   in;
  smoc_port_out<d_chunk>  out;

  void setMemBuffer(char *buf){
    cerr << "setMemBuffer" << __FILE__ << ":" << __LINE__<<endl;
    buffer=buf;
  }

 private:
  string    buffer;
  memory_request _request;
  smoc_firing_state read;
  smoc_firing_state write;
  bool chunksLeft() const{
    return (_request.length!=0);
  }
  void actionWrite(){
    out[0]=d_chunk(buffer.c_str(), _request.sourceId, _request.address, _request.length);
    t_uint32 read=out[0].getLength();
    cout << "chunker> write " << read << " chars start_addr="<<_request.address<<" length="<<_request.length <<endl;
    _request.address += read;
    _request.length     -= read;
  }
  void actionRead(){
    _request=in[0];
    /*    in[0].dump(cout);
	  length    = in[0].buffer.length; // set guard to true
	  start_addr= in[0].buffer.v_start_addr;
	  buf_id    = in[0].buffer_id;*/
  }
 public:
  chunker(sc_module_name name, const char *buffer):smoc_actor(name,read),buffer(buffer){
    // read memory_request
    read  = in(1) >> 
      CALL(chunker::actionRead)  
		  >> write;
    //write data_chunk
    write = (out(1) && guard(&chunker::chunksLeft))
      >> CALL(chunker::actionWrite)
      >> write
      // no chunks left -> go to state read to read new tt_data
      | (!guard(&chunker::chunksLeft))
      >> read;
  }
};





template <int chunksize>
class unchunker: public smoc_actor {

typedef data_chunk<chunksize> d_chunk;

 public:
  smoc_port_in<d_chunk>  in;
 private:
  string            buffer;
  //  char              tempBuffer[chunksize];
  smoc_firing_state write;
  
  void actionWrite(){
    //    in[0].writeTo(tempBuffer);
    d_chunk ch=in[0];
    buffer.replace(in[0].address, in[0].length, ch.getChunk(), in[0].length);
    cout << "chunker> writen to mem:\n"<<buffer <<endl;
  }
 public:
  void setMemBuffer(char *buf){
    cerr << "setMemBuffer" << __FILE__ << ":" << __LINE__<<endl;
    buffer=buf;
  }
  unchunker(sc_module_name name, const char *buffer):smoc_actor(name,write),buffer(buffer){
    // write every incoming chunk
    write  = (in(1)) >> 
      CALL(unchunker::actionWrite)  
		   >> write;
  }
};


/**
 * Hierachical actor representing a read memory access
 * 
 */
template <int chunksize>
class memory_access_read
: public smoc_graph {

 public:
  smoc_port_in<memory_request>  in;
  smoc_port_out<data_chunk<chunksize> > out;

    
  memory_access_read(sc_module_name name,char *buffer) : smoc_graph(name){
    
    generic_communication_node<memory_request> &m_delay_request =
      registerNode( new generic_communication_node<memory_request>("delay_request") );

    generic_communication_node<data_chunk<chunksize> > &m_delay_response =
      registerNode( new generic_communication_node<data_chunk<chunksize> >("delay_response") );

    chunker<chunksize> &m_chunker = registerNode( new chunker<chunksize>( "read_from_memory", buffer ) );

    connectInterfacePorts( in,  m_delay_request.in );
    
    connectNodePorts( m_delay_request.out, m_chunker.in );
    connectNodePorts( m_chunker.out, m_delay_response.in );

    connectInterfacePorts( out, m_delay_response.out ); 
  }
};



template <int chunksize>
class memory_access_write
: public smoc_graph {
 public:
  smoc_port_in<data_chunk<chunksize> > in;

  memory_access_write( sc_module_name name,char *buffer) : smoc_graph(name){

    generic_communication_node<data_chunk<chunksize> > &m_delay_request =
      registerNode( new generic_communication_node<data_chunk<chunksize> >("delay_request") );

    unchunker<chunksize>   &m_unchunker = registerNode(new unchunker<chunksize>("write_to_memory",buffer));

    connectInterfacePorts( in,  m_delay_request.in );  
    
    connectNodePorts( m_delay_request.out, m_unchunker.in );
  }
};
