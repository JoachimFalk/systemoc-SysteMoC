#include <hscd_structure.hpp>

template <typename T_node_type>
void hscd_structure<T_node_type>::assemble( hscd_modes::PGWriter &pgw ) const {
  pgw << "<problemgraph name=\"" << this->name() << "\" id=\"" << pgw.getId(this) << "\">" << std::endl;
  pgw.indentUp();
  for ( typename nodes_ty::const_iterator iter = this->getNodes().begin();
        iter != this->getNodes().end();
        ++iter )
    (*iter)->assemble(pgw);
  for ( typename chan2ports_ty::const_iterator c_iter = this->getChans().begin();
        c_iter != this->getChans().end();
        ++c_iter ) {
    for ( typename ports_ty::const_iterator ps_iter = c_iter->second.begin();
          ps_iter != c_iter->second.end();
          ++ps_iter ) {
      if ( (*ps_iter)->isInput ) {
        for ( typename ports_ty::const_iterator pd_iter = c_iter->second.begin();
              pd_iter != c_iter->second.end();
              ++pd_iter ) {
          if ( !(*pd_iter)->isInput ) {
            pgw << "<edge name=\"" << c_iter->first->name() << "\" "
                << "source=\"" << pgw.getId(*ps_iter) << "\" " 
                << "target=\"" << pgw.getId(*pd_iter) << "\" "
                << "id=\"" << pgw.getId(c_iter->first) << "\"/>" << std::endl;
          }
        }
      }
    }
  }
  pgw.indentDown();
  pgw << "</problemgraph>" << std::endl;
}

template void hscd_structure<hscd_choice_node>::assemble(hscd_modes::PGWriter&) const;
template void hscd_structure<hscd_transact_node>::assemble(hscd_modes::PGWriter&) const;
template void hscd_structure<hscd_fixed_transact_node>::assemble(hscd_modes::PGWriter&) const;
