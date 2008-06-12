//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 et:

#ifndef SMOC_MD_ARRAY_HPP
#define SMOC_MD_ARRAY_HPP

/*
  Defintion of a multi-dimensional array with variable number
  of dimensions
 */

#include <stdlib.h>
#include <cassert>
#include <iostream>

template <typename DATA_TYPE>
class smoc_md_array{
public:
  template <typename T> friend
  std::istream& operator>>(std::istream& stream, smoc_md_array<T>& array);

  template <typename T> friend
  std::ostream& operator<<(std::ostream& stream, const smoc_md_array<T>& array);
public:
  /// constructor with immediate allocation of data structures
  template <typename T2>
  smoc_md_array(
		unsigned int nbr_dimensions,           //number of dimensions
		const T2& buffer_extensions        //extension in each dimension
		);
  /// Copy constructor
  smoc_md_array(const smoc_md_array<DATA_TYPE>& a);
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
  //idx_offset indicates the first element of
  //the vector which shall be taken into account
  template <typename T2>
  unsigned long get_mem_position(const T2& idx,
                                 unsigned int idx_offset = 0
                                 ) const;

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

  //Data element access with index offset
  template <typename T2>
  DATA_TYPE& get_item(const T2& idx,
                      unsigned int idx_offset);
  template <typename T2>
  const DATA_TYPE& get_item(const T2& idx,
                            unsigned int idx_offset) const;
};

template <typename DATA_TYPE> template <typename T2>
smoc_md_array<DATA_TYPE>::smoc_md_array(
					unsigned int nbr_dimensions,           //number of dimensions
					const T2& buffer_extensions        //extension in each dimension
					)
  : data_buffer(NULL)
{
  bool temp = allocate_memory(nbr_dimensions, buffer_extensions);
  assert(temp);
}


template <typename DATA_TYPE>
smoc_md_array<DATA_TYPE>::smoc_md_array(const smoc_md_array<DATA_TYPE>& a) 
  : data_buffer(NULL)
{
  bool temp = allocate_memory(a.nbr_dimensions, a.buffer_extensions);
  assert(temp);

  //Copy data values
  for (unsigned long i = 0;
       i < buffer_size;
       i++){
    data_buffer[i] = a.data_buffer[i];
  }
  
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
    //std::cout << "buffer-extensions: " << buffer_extensions[i] << std::endl;
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
unsigned long smoc_md_array<DATA_TYPE>::get_mem_position(const T2& idx,
                                                         unsigned int idx_offset) const{
  unsigned long mem_pos = 0;
  unsigned long mul_factor = 1;  

  for (unsigned int i = 0; i < nbr_dimensions; i++){
    assert(idx[i+idx_offset] >= 0);
    //std::cout << "i = " << i << ": ";
    //std::cout << "idx[i+idx_offset] = " << idx[i+idx_offset] << " ";
    //std::cout << "buffer_extensions[i] = " << buffer_extensions[i] << std::endl;
    assert((unsigned long)idx[i+idx_offset] < buffer_extensions[i]);
    mem_pos += idx[i+idx_offset] * mul_factor;
    mul_factor *= buffer_extensions[i];
  }

  return mem_pos;
}

template <typename DATA_TYPE> template <typename T2>
DATA_TYPE& smoc_md_array<DATA_TYPE>::operator[](const T2& idx){
  return data_buffer[get_mem_position(idx)];
}

template <typename DATA_TYPE>   template <typename T2>
const DATA_TYPE& smoc_md_array<DATA_TYPE>::operator[](const T2& idx) const {
  return data_buffer[get_mem_position(idx)];
}

template <typename DATA_TYPE> template <typename T2>
DATA_TYPE& 
smoc_md_array<DATA_TYPE>::get_item(const T2& idx,
                                   unsigned int idx_offset) {
  return data_buffer[get_mem_position(idx,idx_offset)];
}

template <typename DATA_TYPE> template <typename T2>
const DATA_TYPE& 
smoc_md_array<DATA_TYPE>::get_item(const T2& idx,
                                   unsigned int idx_offset) const{
  return data_buffer[get_mem_position(idx,idx_offset)];
}

template <typename DATA_TYPE>
std::istream& operator>>(std::istream& stream, smoc_md_array<DATA_TYPE>& array) {
  unsigned long current_coord[array.nbr_dimensions];

  for(unsigned int i = 0; i < array.nbr_dimensions; i++){
    current_coord[i] = 0;
  }
        
  bool finished = false;
  while (!finished){              
                
    stream >> array[(unsigned long*)current_coord];

    finished = true;
    for (unsigned int i = 0; i < array.nbr_dimensions; i++){
      current_coord[i]++;                     

      if (current_coord[i] >= array.buffer_extensions[i]){
	current_coord[i] = 0;
      }else{
	finished = false;
	break;
      }
    }

    if (!stream.good()){
      finished = true;
    }
  }

  return stream;
}

template <typename DATA_TYPE>
std::ostream& operator<<(std::ostream& stream, const smoc_md_array<DATA_TYPE>& array){

  unsigned long current_coord[array.nbr_dimensions];

  for(unsigned int i = 0; i < array.nbr_dimensions; i++){
    current_coord[i] = 0;
  }

  //write array extensions
  stream << "[" << array.nbr_dimensions << "](";
  for(unsigned int i = 0; i < array.nbr_dimensions-1; i++){
    stream << array.buffer_extensions[i] << ",";
  }
  stream << array.buffer_extensions[array.nbr_dimensions-1] << ")" << std::endl;
        
  bool finished = false;
  while (!finished){
    //for(unsigned int i = 0; i < array.nbr_dimensions; i++){
    //      std::cout << current_coord[i];
    //}
    //std::cout << std::endl;

    stream << array[(unsigned long*)current_coord] << " ";

    finished = true;
    for (unsigned int i = 0; i < array.nbr_dimensions; i++){
      current_coord[i]++;

      if (current_coord[i] >= array.buffer_extensions[i]){
	current_coord[i] = 0;
	stream << std::endl;
      }else{
	finished = false;
	break;
      }
    }
  }

  return stream;
}





#endif //SMOC_MD_ARRAY_HPP