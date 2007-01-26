#include <iostream>
#include <map>
#include <string>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>


#include <packet_header.h>

#include <generic_communication_node.hpp>

using Expr::field;
using std::endl;
using std::cout;

struct storage_completed_msg{
  storage_completed_msg( int id ) : buffer_id( id ) {}
  storage_completed_msg( ) : buffer_id( -1 ) {}
  int buffer_id;
  //...
};

template <int chunksize>
class data_chunk{
 private:
  char chunk[chunksize];
 public:
  int chunk_id;
  int chunk_count;
  t_uint32 length;
  t_uint64 address;
  int buffer_id;

  int getChunkSize(){ return chunksize; }

  t_uint32 getLength(){ return length; }

  data_chunk() : length(0){}

  data_chunk( const char *buf, int buffer_id, t_uint64 start, t_uint32 _length )
    : address(start), buffer_id( buffer_id ){
    for( length = 0; length < chunksize && length < _length; length++ ){
      chunk[ length ] = buf[ start + length ];
    }
  }

  const char *getChunk(){ return chunk; }
};

struct data_request{
  t_uint64 address;
  t_uint32 length;
  int buffer_id;

  data_request( int buffer_id, t_uint64 address, t_uint32 length ) :
    address( address ), length( length ), buffer_id( buffer_id ){
    
  }

  data_request() : address( 0 ), length( 0 ), buffer_id(-1) {}
};

struct data_buffer{
  std::set<int> missing_chunks;
  t_uint64 address;
  t_uint32 length;
  int buffer_id;
  std::string data;

  /**
   * Create an empty buffer, to fill in some chunks step by step with "write_chunk"!
   */
  data_buffer( int buffer_size, int chunk_count ){
    data.resize( buffer_size );
    for(int i = 0; i < chunk_count ; i++){
      missing_chunks.insert(i);
    }
  }

  data_buffer( const char * data, int buffer_id, t_uint64 start)
    : address( start ), buffer_id( buffer_id ), data( data ){
    length = this->data.length();
  }
  
  data_buffer( ){}

  /**
   * Fill data from chunk!
   */
  template <int chunksize>
  void write_chunk(data_chunk<chunksize> ch){
    
    ch.address = ch.chunk_id*chunksize; // set new storage location
    
    data.replace(ch.address, ch.length, ch.getChunk(), ch.length);
    std::set<int>::iterator iter = missing_chunks.find(ch.chunk_id);
    missing_chunks.erase(iter);
  }

  bool complete(){
    return missing_chunks.empty();
  }
};


/**
 * Reads from memory array (initialised in ctor).
 * Creates a lot of data chunks with fixed size "chunksize".
 */
template <int chunksize>
class memory_chunker: public smoc_actor {

typedef data_chunk<chunksize> d_chunk;

 public:
  smoc_port_in<data_request>   in;
  smoc_port_out<d_chunk>  out;

 private:
  std::string    buffer;
  data_request _request;
  int actual_chunk_id;
  int chunk_count;
  smoc_firing_state read;
  smoc_firing_state write;
  bool chunksLeft() const{
    return ( _request.length != 0 );
  }
  void actionWrite(){
    d_chunk data(buffer.c_str(), _request.buffer_id, _request.address, _request.length);

    data.chunk_id = actual_chunk_id;
    data.chunk_count=chunk_count;

    actual_chunk_id++;

    out[0]=data;
    //cerr << "send chunk #" << data.chunk_id << endl;
    t_uint32 read=out[0].getLength();
    _request.address += read;
    _request.length  -= read;
  }
  void actionRead(){
    _request=in[0];
    actual_chunk_id=0;

    //calculate number of chunks
    chunk_count=( _request.length / chunksize );
    if( ( _request.length % chunksize ) > 0) { chunk_count++; } 

  }
 public:
  memory_chunker( sc_module_name name, const char *buffer )
    : smoc_actor( name, read ), buffer( buffer ), actual_chunk_id( 0 ){
    // read data_request
    read  = in(1) >> 
      CALL(memory_chunker::actionRead)  
		  >> write;
    //write data_chunk
    write = (out(1) && guard(&memory_chunker::chunksLeft))
      >> CALL(memory_chunker::actionWrite)
      >> write
      // no chunks left -> go to state read to read new tt_data
      | (!guard(&memory_chunker::chunksLeft))
      >> read;
  }
};

/**
 * Takes all given chunks and write the data into data (data_buffer).
 */
template <int chunksize>
class chunks_to_buffer: public smoc_actor {

typedef data_chunk<chunksize> d_chunk;

 public:
  smoc_port_in<d_chunk>  in;
  smoc_port_out<data_buffer>  out;
 private:
  std::map<int, data_buffer* > buffer_map;

  smoc_firing_state write;

  bool completedChunkSeries() const {
    for( std::map<int,data_buffer* >::const_iterator iter = buffer_map.begin();
	 iter != buffer_map.end();
	 iter++){
      if( iter->second->complete()) return true;
    }
    return false;
  }
  
  data_buffer *getBuffer(int id, int chunk_count){
    if( buffer_map.count(id) == 0 ){
      int size=chunksize*chunk_count;
      buffer_map[id]=new data_buffer(size, chunk_count);
    }
    return  buffer_map[id];
  }
  
  void sendBuffer(){
    for( std::map<int,data_buffer* >::const_iterator iter = buffer_map.begin();
	 iter != buffer_map.end();
	 iter++){
      if( iter->second->complete()){
	out[0]=*iter->second;
	buffer_map.erase(iter->first);
      }
    } 
  }
  void actionWrite(){
    //    in[0].writeTo(tempBuffer);
    d_chunk ch=in[0];

    //cerr << "Got Chunk id="<<ch.chunk_id << " bufferid="<<ch.buffer_id<<endl;
    
    data_buffer  *buffer = getBuffer(ch.buffer_id, ch.chunk_count);

    buffer->write_chunk(ch);
    //    delete ch;
    cout << "chunks_to_buffer> writen to buf:\n" << buffer->data <<endl;
  }
 public:
  chunks_to_buffer(sc_module_name name):smoc_actor(name,write){
    // write every incoming chunk
    write  = (in(1) && !guard(&chunks_to_buffer::completedChunkSeries)) 
      >> CALL(chunks_to_buffer::actionWrite)  
      >> write
    | (guard(&chunks_to_buffer::completedChunkSeries) && out(1)) 
      >> CALL(chunks_to_buffer::sendBuffer)  
      >> write;
  }
};

/**
 * Takes all given chunks and write the data into memory array.
 */
template <int chunksize>
class chunks_to_memory: public smoc_actor {

typedef data_chunk<chunksize> d_chunk;

 public:
  smoc_port_in<d_chunk>  in;
  smoc_port_out<storage_completed_msg>  out;
 private:
  //  std::set<int> missing_chunks;
  std::map<int, std::set<int>* > chunk_set;


  std::string            buffer;
  //  char              tempBuffer[chunksize];
  smoc_firing_state write;

  bool completedChunkSeries() const {
    for( std::map<int,std::set<int>* >::const_iterator iter = chunk_set.begin();
	 iter != chunk_set.end();
	 iter++){
      if( iter->second->empty()) return true;
    }
    return false;
  }
  
  std::set<int> *getChunkSet(int id, int chunk_count){
    if( chunk_set.count(id) == 0 ){
      //int size=chunksize*chunk_count;
      
      std::set<int> *missing_chunks=new std::set<int>();
      for( int i = 0; i < chunk_count; i++ ){
	missing_chunks->insert(i);
      }
      chunk_set[id]=missing_chunks;
    }
    return  chunk_set[id];
  }

  void sendCompletionMsg(){
    for( std::map<int,std::set<int>* >::const_iterator iter = chunk_set.begin();
	 iter != chunk_set.end();
	 iter++){
      if( iter->second->empty()){
	out[0]=storage_completed_msg(iter->first);
	chunk_set.erase(iter->first);
      }
    }
  }

  void actionWrite(){
    //    in[0].writeTo(tempBuffer);
    d_chunk ch=in[0];
    buffer.replace( ch.address, ch.length, ch.getChunk(), ch.length );
    getChunkSet( ch.buffer_id, ch.chunk_count )->erase( ch.chunk_id );
    
    cout << "chunks_to_memory> will write:\n"<< ch.getChunk() <<endl;
    cout << "chunks_to_memory> writen to mem:\n"<<buffer <<endl;
  }
 public:
  chunks_to_memory( sc_module_name name, const char *buffer ):smoc_actor( name, write ),buffer( buffer ){
    // write every incoming chunk
    write  = ( in( 1 ) && !guard( &chunks_to_memory::completedChunkSeries ) ) 
      >> CALL( chunks_to_memory::actionWrite )  
      >> write
    | ( out( 1 ) &&  guard( &chunks_to_memory::completedChunkSeries ) ) 
      >> CALL( chunks_to_memory::sendCompletionMsg )  
      >> write;
  }
};

/**
 * Reads from data_buffer.
 * Creates a lot of data chunks with fixed size "chunksize".
 */
template <int chunksize>
class buffer_chunker: public smoc_actor {

typedef data_chunk<chunksize> d_chunk;

 public:
  smoc_port_in<data_buffer>   in;
  smoc_port_out<d_chunk>  out;

 private:
  data_buffer  buffer;
  int actual_chunk_id;
  int chunk_count;
  t_uint32 actual_length_to_chunk;
  smoc_firing_state read;
  smoc_firing_state write;
  bool chunksLeft() const{
    return ( chunk_count > 0 && chunk_count > actual_chunk_id );
  }
  void actionWrite(){
    t_uint64 chunk_offset=( actual_chunk_id * chunksize );
    d_chunk data(buffer.data.c_str(), buffer.buffer_id, chunk_offset, actual_length_to_chunk);
    data.address = buffer.address + chunk_offset;

    data.chunk_id = actual_chunk_id;
    data.chunk_count=chunk_count;

    actual_chunk_id++;
    actual_length_to_chunk-=chunksize;

    out[0]=data;
    //cerr << "send chunk #" << data.chunk_id << endl;
  }
  void actionRead(){
    buffer=in[0];
    actual_chunk_id=0;
    actual_length_to_chunk=buffer.length;

    //calculate number of chunks
    chunk_count=( buffer.length / chunksize );
    if( ( buffer.length % chunksize ) > 0) { chunk_count++; } 

  }
 public:
  buffer_chunker( sc_module_name name)
    : smoc_actor( name, read ), actual_chunk_id( 0 ){
    // read data_request
    read  = in(1) >> 
      CALL(buffer_chunker::actionRead)  
		  >> write;
    //write data_chunk
    write = (out(1) && guard(&buffer_chunker::chunksLeft))
      >> CALL(buffer_chunker::actionWrite)
      >> write
      // no chunks left -> go to state read to read new tt_data
      | (!guard(&buffer_chunker::chunksLeft))
      >> read;
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
  smoc_port_in<data_request>  in;
  smoc_port_out<data_buffer> out;

    
  memory_access_read(sc_module_name name,char *buffer) : smoc_graph(name){
    
    generic_communication_node<data_request> &m_delay_request =
      registerNode( new generic_communication_node<data_request>("delay_request") );

    memory_chunker<chunksize> &m_memory_chunker = registerNode( new memory_chunker<chunksize>( "read_from_memory", buffer ) );

    generic_communication_node<data_chunk<chunksize> > &m_delay_response =
      registerNode( new generic_communication_node<data_chunk<chunksize> >("delay_response") );

    
    chunks_to_buffer<chunksize> &m_chunk2buf = registerNode( new chunks_to_buffer<chunksize>( "chunk2buf")); 
    
    connectInterfacePorts( in,  m_delay_request.in );
    
    connectNodePorts( m_delay_request.out, m_memory_chunker.in );
    connectNodePorts( m_memory_chunker.out, m_delay_response.in);
    connectNodePorts( m_delay_response.out, m_chunk2buf.in);
    
    connectInterfacePorts( out, m_chunk2buf.out ); 
  }
};




/**
 *Hierachical actor representing a write memory access
 */
template <int chunksize>
class memory_access_write
: public smoc_graph {
 public:
  smoc_port_in<data_buffer> in;
  smoc_port_out<storage_completed_msg>   out;
  
  memory_access_write( sc_module_name name,char *buffer) : smoc_graph(name){
    
    buffer_chunker<chunksize> &m_buffer_chunker = registerNode( new buffer_chunker<chunksize>( "read_from_buffer") );
    
    
    generic_communication_node<data_chunk<chunksize>,4 > &m_delay_request =
      registerNode( new generic_communication_node<data_chunk<chunksize>,4 >("delay_request") );

    chunks_to_memory<chunksize>   &m_chunks_to_memory = registerNode(new chunks_to_memory<chunksize>("write_to_memory",buffer));
    
    connectInterfacePorts( in,  m_buffer_chunker.in );  
    
    connectNodePorts( m_buffer_chunker.out, m_delay_request.in, smoc_fifo<data_chunk<chunksize> >(4) );
    connectNodePorts( m_delay_request.out, m_chunks_to_memory.in, smoc_fifo<data_chunk<chunksize> >(4) );

    connectInterfacePorts( out, m_chunks_to_memory.out);
  }
};
