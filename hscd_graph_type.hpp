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
