// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _INCLUDED_SMOC_PGGEN_HPP
#define _INCLUDED_SMOC_PGGEN_HPP

#include <iostream>
#include <exception>

#include <cassert>

#include <map>

class smoc_root_node;

namespace smoc_modes {

  class eNoChannel : public std::exception {};
  class eNoInterface : public std::exception {};
  class eNoPort : public std::exception {};
  
  class PGWriter {
    friend class Node;
  protected:
    typedef  std::map<const void *,int> idmap_ty;
    
    std::ostream    &out;
    static int       idmap_last;
    static idmap_ty  idmap;
    
    static const char   indent_buf[];
    static const size_t indent_buf_len;
    
    int indent_lev;
    
    const char *
    indentation () const;
    
    static std::string toId(int id);
  public:
    PGWriter(std::ostream &out)
      : out(out), indent_lev(0) {}
    
    void indentUp() { ++indent_lev; }
    void indentDown() { --indent_lev; }
    
    template <typename T>
    std::ostream &operator << (T t) { return out << indentation() << t; }
    
    static std::string getId( const void *p );
    static std::string getId();
    
    ~PGWriter( void ) {
      out.flush();
    }
  };

  void dump(std::ostream &out, smoc_root_node &top);
};

#endif // _INCLUDED_SMOC_PGGEN_HPP
