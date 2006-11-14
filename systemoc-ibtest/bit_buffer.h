/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 * 
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

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
