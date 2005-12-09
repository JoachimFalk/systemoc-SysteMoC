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

namespace smoc_modes {

  class eNoChannel : public std::exception {};
  class eNoInterface : public std::exception {};
  class eNoPort : public std::exception {};
  
  class PGWriter {
    friend class Node;
  protected:
    typedef  std::map<const void *,int> idmap_ty;
    
    std::ostream    &out;
    int              idmap_last;
    idmap_ty         idmap;
    
    static const char   indent_buf[];
    static const size_t indent_buf_len;
    
    int indent_lev;
    
    const char *
    indentation () const;
    
    std::string toId(int id);
  public:
    PGWriter( std::ostream &_out )
      : out(_out), idmap_last(0), indent_lev(0) {}
    
    void indentUp() { ++indent_lev; }
    void indentDown() { --indent_lev; }
    
    template <typename T>
    std::ostream &operator << (T t) { return out << indentation() << t; }
    
    std::string getId( const void *p );
    std::string getId();
    
    ~PGWriter( void ) {
      out.flush();
    }
  };

  class smoc_modes_base_structure {
    public:
      virtual
      void assemble( PGWriter &pgw ) const = 0;
      
      virtual
      ~smoc_modes_base_structure() {}
  };

  void dump( std::ostream &out, const smoc_modes_base_structure &top );
};

#if 0


class smoc_modes {
protected:
//  NProblemGraphEmbed &parent_pge;
public:
  smoc_modes_pg( const smoc_structure_base &x ) {}
  
  void assemble( int indent ) {
    oStream() << indentation(indent) << "<problemgraph name=\"" << getName() << "\" " << addId() << ">" << std::endl;
    for ( procmap_ty::iterator iter = procs.begin();
	  iter != procs.end();
	  ++iter ) {
      iter->second.assemble(indent+1);
    }
    for ( chanmap_ty::iterator iter = chans.begin();
	  iter != chans.end();
	  ++iter ) {
      iter->second.assemble(indent+1);
    }
    for ( ifacemap_ty::iterator iter = ifaces.begin();
	  iter != ifaces.end();
	  ++iter ) {
      iter->second.assemble(indent+1);
    }
    oStream() << indentation(indent) << "</problemgraph>" << std::endl;
  }
};
 
  
class Node {
private:
  HSCDPGWriter	&writer;
  const std::string	 name;
  int			 myid;
    
  char indent_buf[257];
protected:
  std::string addId( void ) const {
    std::ostringstream ss;
      
    ss << "id=\"" << getId() << "\"";
    return ss.str();
  }
    
  std::ostream &oStream() { return writer.out; }
    
  Node( HSCDPGWriter &_writer, std::string _name )
    : writer(_writer), name(_name), myid(++writer.idno) {}
  Node( Node &_node, std::string _name )
    : writer(_node.writer), name(_name), myid(++writer.idno) {}
public:
  operator  HSCDPGWriter &(void) {
    return writer;
  }

  std::string getId( void ) const {
    std::ostringstream ss;
      
    ss << "id" << myid;
    return ss.str();
  }
    
  std::string getName( void ) const { return name; }
};

class NProblemGraph;

class NPort: public Node {
protected:
  const std::string type;
      
public:
  NPort( Node &_node, std::string _name, std::string _type )
    : Node( _node, _name ), type(_type) {
    std::cerr << "Port: " << getName() << std::endl;
  }

  std::string getType( void ) const { return type; }

  void assemble( int indent ) {
    oStream() << indentation(indent) << "<port name=\"" << getName() << "\" type=\"" << type << "\" " << addId() << "/>" << std::endl;
  }

  ~NPort( void ) {
    //	  std::cerr << "~Port: " << getName() << std::endl;
  }
};
  
class NProblemGraphEmbed: public Node {
protected:
  typedef std::map< std::string,NProblemGraph *> pgmap_ty;
  pgmap_ty pgs;
  typedef std::map<std::string,NPort>         portmap_ty;
  portmap_ty ports;
public:
  NProblemGraphEmbed( HSCDPGWriter &_writer, std::string _name )
    : Node( _writer, _name ) {}
      
  NPort &Port( std::string name, std::string type ) {
    NPort p( *this, name, type );
    return ports.insert( pair<std::string,NPort>(p.getName(),p) ).first->second;
  }
      
  NPort &getPort( std::string name ) {
    portmap_ty::iterator iter = ports.find(name);
    
    if ( iter == ports.end() )
      throw eNoPort();
    return iter->second;
  }
      
  NProblemGraph &ProblemGraph( std::string name );
  NProblemGraph &getProblemGraph( std::string name );
    
  void assemble( int indent );
};
  
class NProcess: public NProblemGraphEmbed {
protected:
public:
  NProcess( Node &_node, std::string _name )
    : NProblemGraphEmbed( _node, _name ) {
    std::cerr << "Process: " << getName() << std::endl;
  }
	
  void assemble( int indent ) {
    oStream() << indentation(indent) << "<process name=\"" << getName() << "\" " << addId() << ">" << std::endl;
    NProblemGraphEmbed::assemble(indent+1);
    oStream() << indentation(indent) << "</process>" << std::endl;
  }

  ~NProcess( void ) {
    //	  std::cerr << "~Process: " << getName() << std::endl;
  }

};

class SpecificationGraph : public NProblemGraphEmbed {
public:
  SpecificationGraph( std::ostream &_out, std::string _name )
    : NProblemGraphEmbed( *new HSCDPGWriter(_out), _name) {}
    
  void assemble( int indent ) {
    oStream() << "<?xml version=\"1.0\"?>" << std::endl
	      << "<!DOCTYPE specificationgraph SYSTEM \"specgraph.dtd\">" << std::endl
	      << indentation(indent) << "<specificationgraph name=\"" << getName() << "\">" << std::endl;
    NProblemGraphEmbed::assemble(indent+1);
    oStream() << indentation(indent) << "</specificationgraph>" << std::endl;
  }
};
  
class NInterface: public Node {
protected:
  NPort		*conPort;
  NPort		*myPort;
public:
  NInterface( NPort &_conPort )
    : Node(_conPort,_conPort.getName()), conPort(&_conPort), myPort(NULL) {};
    
  void addPort ( NPort &_myPort  ) {
    assert( myPort == NULL  );
    myPort  = &_myPort;
  };
    
  void assemble( int indent ) {
    // assert( in != NULL && out != NULL );
    oStream() << indentation(indent)
	      << "<portmapping name=\"" << getName() << "\" "
	      << "from=\"" << (myPort != NULL ? myPort->getId() : "??") << "\" " 
	      << "to=\"" << (conPort  != NULL ? conPort->getId()  : "??") << "\" "
	      << addId() << "/>" << std::endl;
  }
};

class NChannel: public Node {
protected:
  NPort		*in;
  NPort		*out;
public:
  NChannel( Node &_node, std::string _name )
    : Node(_node,_name), in(NULL), out(NULL) {};
    
  void addIn ( NPort &_in  ) { assert( in == NULL  ); in  = &_in;  };
  void addOut( NPort &_out ) { assert( out == NULL ); out = &_out; };

  void assemble( int indent ) {
    // assert( in != NULL && out != NULL );
    oStream() << indentation(indent)
	      << "<edge name=\"" << getName() << "\" "
	      << "source=\"" << (out != NULL ? out->getId() : "??") << "\" " 
	      << "target=\"" << (in  != NULL ? in->getId()  : "??") << "\" "
	      << addId() << "/>" << std::endl;
  }
};
    
class NProblemGraph: public Node {
protected:
  typedef std::map<std::string,NProcess>   procmap_ty;
  procmap_ty  procs;
      
  typedef std::map<std::string,NChannel>   chanmap_ty;
  chanmap_ty  chans;

  typedef std::map<std::string,NInterface> ifacemap_ty;
  ifacemap_ty ifaces;

  NProblemGraphEmbed &parent_pge;
public:
  NProblemGraph( NProblemGraphEmbed &parent_pge, std::string name )
    : Node( parent_pge, name ), parent_pge(parent_pge) {
  }
  
  NChannel &Channel( std::string name ) {
    NChannel c( *this, name );
    return chans.insert( chanmap_ty::value_type(c.getName(),c) ).first->second;
  }

  NChannel &getChannel( std::string name ) {
    chanmap_ty::iterator iter = chans.find(name);
      
    if ( iter == chans.end() )
      throw eNoChannel();
    return iter->second;
  }

  NInterface &Interface( std::string name ) {
    NInterface i( parent_pge.getPort(name) );
    return ifaces.insert( ifacemap_ty::value_type(i.getName(),i) ).first->second;
  }

  NInterface &getInterface( std::string name ) {
    ifacemap_ty::iterator iter = ifaces.find(name);
    
    if ( iter == ifaces.end() )
      throw eNoInterface();
    return iter->second;
  }

  NProcess &Process( std::string name ) {
    NProcess p( *this, name );
    return procs.insert( pair<std::string,NProcess>(p.getName(),p) ).first->second;
  }

  void assemble( int indent ) {
    oStream() << indentation(indent) << "<problemgraph name=\"" << getName() << "\" " << addId() << ">" << std::endl;
    for ( procmap_ty::iterator iter = procs.begin();
	  iter != procs.end();
	  ++iter ) {
      iter->second.assemble(indent+1);
    }
    for ( chanmap_ty::iterator iter = chans.begin();
	  iter != chans.end();
	  ++iter ) {
      iter->second.assemble(indent+1);
    }
    for ( ifacemap_ty::iterator iter = ifaces.begin();
	  iter != ifaces.end();
	  ++iter ) {
      iter->second.assemble(indent+1);
    }
    oStream() << indentation(indent) << "</problemgraph>" << std::endl;
  }
};
  
NProblemGraph &NProblemGraphEmbed::ProblemGraph( std::string name ) {
  NProblemGraph *pg = new NProblemGraph( *this, name );
    
  pgs.insert( pgmap_ty::value_type(pg->getName(),pg) );
  return *pg;
}
  
NProblemGraph &NProblemGraphEmbed::getProblemGraph( std::string name ) {
  pgmap_ty::iterator iter = pgs.find(name);
    
  assert( iter != pgs.end() );
  return *iter->second;
}

void NProblemGraphEmbed::assemble( int indent ) {
  for ( portmap_ty::iterator iter = ports.begin();
	iter != ports.end();
	++iter ) {
    iter->second.assemble(indent);
  }
  for ( pgmap_ty::iterator iter = pgs.begin();
	iter != pgs.end();
	++iter ) {
    iter->second->assemble(indent);
  }
}

#endif

#endif // _INCLUDED_SMOC_PGGEN_HPP
