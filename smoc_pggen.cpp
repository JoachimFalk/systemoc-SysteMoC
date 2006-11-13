// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 *
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

#include <smoc_pggen.hpp>
#include <smoc_root_node.hpp>
#include <sstream>

namespace smoc_modes {

  int                 PGWriter::idmap_last = 0;
  PGWriter::idmap_ty  PGWriter::idmap;
  const char          PGWriter::indent_buf[] = 
     "                                                                "
     "                                                                "
     "                                                                "
     "                                                                ";
  const size_t        PGWriter::indent_buf_len = sizeof(PGWriter::indent_buf) - 1;

  const char *
  PGWriter::indentation () const {
    unsigned int indent = 2*indent_lev;
    
    return indent >= indent_buf_len
      ? indent_buf : indent_buf + indent_buf_len - indent;
  }
    
  std::string PGWriter::toId(int id) {
    std::ostringstream o;
    o << "id" << id;
    return o.rdbuf()->str();
  }

  std::string PGWriter::getId() {
    return toId(idmap_last++);
  }

  std::string PGWriter::getId( const void *p ) {
    idmap_ty::iterator find_iter = idmap.find(p);
    
    if ( find_iter == idmap.end() ) {
      idmap[p] = idmap_last;
      return getId();
    } else
      return toId(find_iter->second);
  }

  void dump( std::ostream &out, smoc_root_node &top ) {
    sc_get_curr_simcontext()->elaborate();
    
    PGWriter pgw( out );
    pgw << "<?xml version=\"1.0\"?>" << std::endl;
	pgw << "<!DOCTYPE networkgraph SYSTEM \"networkgraph.dtd\">" << std::endl;
	pgw << "<networkgraph name=\"smoc_modes::dump\">" << std::endl;
    pgw.indentUp();
    top.assemble( pgw );
    pgw << "<architecturegraph name=\"architecture graph\" id=\""<< pgw.getId() << "\">" << std::endl;
    pgw << "</architecturegraph>" << std::endl;
    pgw <<  "<mappings>" << std::endl;
    pgw <<  "</mappings>" << std::endl;
    pgw.indentDown();
    pgw << "</networkgraph>" << std::endl;
  }
};
