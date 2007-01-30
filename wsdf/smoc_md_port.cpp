
#include <smoc_md_port.hpp>


namespace ns_smoc_vector_init {
	smoc_vector_init<unsigned long> ul_vector_init;
	smoc_vector_init<long> sl_vector_init;
};


namespace Expr {

	const smoc_root_port *ASTNodePortIteration::getPort() const  { 
		return &port; 
	}


	std::string           ASTNodePortIteration::getNodeType() const
  { 
		return "PortIteration"; 
	}

	
	std::string           ASTNodePortIteration::getNodeParam() const {
		assert(false);
	}


} // namespace Expr;
