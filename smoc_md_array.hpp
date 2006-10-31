#ifndef SMOC_MD_ARRAY_HPP
#define SMOC_MD_ARRAY_HPP

/*
  Defintion of a multi-dimensional array with variable number
  of dimensions
 */

template <typename DATA_TYPE>
class smoc_md_array{
public:
  /// constructor with immediate allocation of data structures
	template <typename T2>
  smoc_md_array(
								unsigned int nbr_dimensions,           //number of dimensions
								const T2& buffer_extensions        //extension in each dimension
								);
  ~smoc_md_array();


private:
  unsigned int nbr_dimensions;       //number of buffer dimensions;
  unsigned long *buffer_extensions;   //extension in each dimension

  //size of memory zone
  unsigned long buffer_size;

  DATA_TYPE *data_buffer;

private:
  //converts a given coordinate into the memory position
  //where we can find the data element in the buffer
	template <typename T2>
  unsigned long get_mem_position(const T2& idx) const;

	template <typename T2>
	bool allocate_memory(unsigned int nbr_dimensions,           //number of dimensions
											 const T2& buffer_extensions           //extension in each dimension
											 );

public:
	unsigned int dimensions(void) const;
	unsigned long size(unsigned int dimension) const;

	template <typename T2>
	DATA_TYPE& operator[](const T2& idx);
	template <typename T2>
	const DATA_TYPE& operator[](const T2& idx) const;
};

template <typename DATA_TYPE> template <typename T2>
smoc_md_array<DATA_TYPE>::smoc_md_array(
			      unsigned int nbr_dimensions,           //number of dimensions
			      const T2& buffer_extensions        //extension in each dimension
						)
  : data_buffer(NULL)
{
  assert(allocate_memory(nbr_dimensions, buffer_extensions));
}

template <typename DATA_TYPE>
smoc_md_array<DATA_TYPE>::~smoc_md_array(){
  if (data_buffer != NULL)
    delete[] data_buffer;
}

template <typename DATA_TYPE> template <typename T2>
bool smoc_md_array<DATA_TYPE>::allocate_memory(unsigned int nbr_dimensions,           //number of dimensions
																					const T2& buffer_extensions        //extension in each dimension
																					){
  if (data_buffer != NULL){
    //memory already allocated
    return false;
  }

  buffer_size = 1;

  //copy parameters
	this->nbr_dimensions = nbr_dimensions;
  this->buffer_extensions = new unsigned long[nbr_dimensions];

  for(unsigned int i = 0; i < nbr_dimensions; i++){
    buffer_size *= buffer_extensions[i];
    this->buffer_extensions[i] = buffer_extensions[i];
  }
  
  //allocate memory for buffer
  data_buffer = new DATA_TYPE[buffer_size];

  if (data_buffer == NULL){
    return false;
  }
  
	return true;

}

template <typename DATA_TYPE>
unsigned int smoc_md_array<DATA_TYPE>::dimensions(void) const{
	return nbr_dimensions;
}

template <typename DATA_TYPE>
unsigned long smoc_md_array<DATA_TYPE>::size(unsigned int dimension) const {
	assert(dimension < nbr_dimensions);
	return buffer_extensions[dimension];
}

template <typename DATA_TYPE> template <typename T2>
unsigned long smoc_md_array<DATA_TYPE>::get_mem_position(const T2& idx) const{
  unsigned long mem_pos = 0;
  unsigned long mul_factor = 1;  

  for (unsigned int i = 0; i < nbr_dimensions; i++){
    assert(idx[i] < buffer_extensions[i]);
    mem_pos += idx[i] * mul_factor;
    mul_factor *= buffer_extensions[i];
  }

  return mem_pos;
}

template <typename DATA_TYPE> template <typename T2>
DATA_TYPE& smoc_md_array<DATA_TYPE>::operator[](const T2& idx){
	return data_buffer[get_mem_position(idx)];
}

template <typename DATA_TYPE>	template <typename T2>
const DATA_TYPE& smoc_md_array<DATA_TYPE>::operator[](const T2& idx) const {
	return data_buffer[get_mem_position(idx)];
}




#endif //SMOC_MD_ARRAY_HPP
