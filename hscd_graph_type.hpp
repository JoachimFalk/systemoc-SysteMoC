// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_GRAPH_TYPE_HPP
#define _INCLUDED_HSCD_GRAPH_TYPE_HPP

#include <hscd_port.hpp>
#include <hscd_fifo.hpp>
//#include <hscd_rendezvous.hpp>
#include <hscd_node_types.hpp>
#ifndef __SCFE__
# include <hscd_pggen.hpp>
#endif
#include <hscd_structure.hpp>

#include <systemc.h>

#include <list>
#include <map>

typedef hscd_sdf_structure      hscd_sdf_constraintset;
typedef hscd_fifocsp_structure  hscd_fifocsp_constraintset;
typedef hscd_graph              hscd_df_constraintset;
// typedef hscd_graph_sdf<hscd_choice_node, hscd_rendezvous_kind, hscd_rendezvous>
//           hscd_csp_constraintset;

#endif // _INCLUDED_HSCD_GRAPH_TYPE_HPP
