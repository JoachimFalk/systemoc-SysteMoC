/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: InfiniBand HCA
 * Comment:
 * -----------------------------------------------------------------------------
 * bit_buffer.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/

#include <assert.h>
#include <string>
#include <limits.h>

template <typename T> class bit_field;


/**
 *  \brief Bit Buffer
 *
 * the bit_buffer class manages a (std::string based) buffer
 */
class bit_buffer 
{
  // template <typename T>
  //friend class bit_field<T>;
  
  public:
    typedef bit_buffer this_type;
    
  protected:
    std::string  mem;
    // memory size in bytes
    size_t  size;

  /**
   *  \brief Bit Field
   *
   *  a bit field represents a bit range within a
   *  bit_buffer. this range can be read/written.
   *
   *  a bit_field is initialized with a reference
   *  to the buffer, an bit offset and a bit range
   *  of the field
   */
  template <typename T> class bit_field
  {
    public:
      typedef bit_field<T> this_type;

    protected:
      bit_buffer &bb;
      size_t bo;
      size_t bl;

    public:
      /// constructor
      bit_field( bit_buffer &bb, size_t bo, size_t bl)
        : bb(bb), bo(bo), bl(bl) {
        assert( (bo + bl) <= (bb.size * CHAR_BIT) );
      }
    
      /// write bit_field with value x
      this_type &operator= (T x);
      /// value from bit_field
      operator T() const;
  };

  public:
    /// read string from buffer
    std::string get_string() { return this->mem; }

    size_t      get_size() { return this->size; }
    
    /// constructor for empty bitbuffer
    bit_buffer( size_t s ) : 
      mem(s, 0x00),
      size(s)
    {}

    /// constructur with initial buffer value
    bit_buffer(std::string data ) :
      mem( data ),
      size( data.size() )
    {}
};
