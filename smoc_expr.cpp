#include <smoc_expr.hpp>

namespace Expr {

void dump(const PASTNode &node) {
  std::cout << "Node: ";

/*
  if (node->isa<ASTNodeVType<bool> >() ) {
    std::cout << "bool(" << node->isa<ASTNodeVType<bool> >()->value() << ") ";
  } else if (node->isa<ASTNodeVType<int> >() ) {
    std::cout << "int(" << node->isa<ASTNodeVType<int> >()->value() << ") ";
  } else if (node->isa<ASTNodeVType<unsigned int> >() ) {
    std::cout << "unsigned int(" << node->isa<ASTNodeVType<unsigned int> >()->value() << ") ";
  } else if (node->isa<ASTNodeVType<long> >() ) {
    std::cout << "int(" << node->isa<ASTNodeVType<long> >()->value() << ") ";
  } else if (node->isa<ASTNodeVType<unsigned long> >() ) {
    std::cout << "unsigned int(" << node->isa<ASTNodeVType<unsigned long> >()->value() << ") ";
  } else if (node->isa<ASTNodeVType<double> >() ) {
    std::cout << "unsigned int(" << node->isa<ASTNodeVType<double> >()->value() << ") ";
  } else {
    std::cout << "unknown value_type ";
  }
  */
  
  if (node->isa<ASTNodeNonTerminal>()) {
    if ( node->isa<ASTNodeBinOp>() ) {
      boost::intrusive_ptr<ASTNodeBinOp> p = node->isa<ASTNodeBinOp>();
      
      std::cout << "BinOp " << p->getOpType() << " {" << std::endl;
      dump(p->getLeftNode());
      std::cout << "}, {" << std::endl;
      dump(p->getRightNode());
      std::cout << "}";
    } else {
      // unknown
      std::cout << "Unkown NonTerminal";
    }
  } else {
    assert( node->isa<ASTNodeTerminal>() );
    if ( node->isa<ASTNodeLiteral>() ) {
      std::cout << "Literal";
    } else if ( node->isa<ASTNodeVar>() ) {
      std::cout << "Var " << node->isa<ASTNodeVar>()->ptrVar();
    } else if ( node->isa<ASTNodeProc>() ) {
      std::cout << "Proc 0x" << std::hex << reinterpret_cast<unsigned long>
        (node->isa<ASTNodeProc>()->ptrProc());
    } else if ( node->isa<ASTNodeMemProc>() ) {
      union {
        struct { void *p; unsigned long o; } e1;
        ASTNodeMemProc::fun                  e2;
      } h;
      
      h.e2 = node->isa<ASTNodeMemProc>()->ptrMemProc();
      std::cout << "MemProc 0x" << std::hex << reinterpret_cast<unsigned long>(h.e1.p)
                <<   " obj " << node->isa<ASTNodeMemProc>()->ptrObj();
    } else {
      // unknown
      std::cout << "Unkown Terminal";
    }
  }
  std::cout << std::endl;
}

} // namespace Expr
