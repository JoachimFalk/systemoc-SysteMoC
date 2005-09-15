// vim: set sw=2 ts=8:

#include <smoc_pggen.hpp>
#include <sstream>

namespace smoc_modes {

  const char   PGWriter::indent_buf[] = 
     "                                                                "
     "                                                                "
     "                                                                "
     "                                                                ";
  const size_t PGWriter::indent_buf_len = sizeof(PGWriter::indent_buf) - 1;

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

  void dump( std::ostream &out, const smoc_modes_base_structure &top ) {
    PGWriter pgw( out );
    pgw << "<?xml version=\"1.0\"?>" << std::endl;
	pgw << "<!DOCTYPE specificationgraph SYSTEM \"specgraph.dtd\">" << std::endl;
	pgw << "<specificationgraph name=\"smoc_modes::dump\">" << std::endl;
    pgw.indentUp();
    top.assemble( pgw );
    pgw << "<architecturegraph name=\"architecture graph\" id=\""<< pgw.getId() << "\">" << std::endl;
    pgw << "</architecturegraph>" << std::endl;
    pgw <<  "<mappings>" << std::endl;
    pgw <<  "</mappings>" << std::endl;
    pgw.indentDown();
    pgw << "</specificationgraph>" << std::endl;
  }
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
