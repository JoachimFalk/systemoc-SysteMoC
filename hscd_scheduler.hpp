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

#ifndef _INCLUDED_HSCD_SCHEDULER_HPP
#define _INCLUDED_HSCD_SCHEDULER_HPP

#include <systemc.h>
#include <hscd_node_types.hpp>
#include <hscd_structure.hpp>
// #include <hscd_rendezvous.hpp>

#include <list>

class hscd_scheduler_asap
: public sc_module{
public:
  template <typename T>
  hscd_scheduler_asap( sc_module_name name, const std::list<T> &nl )
    : sc_module(name) {}
};

class hscd_top {
private:
  std::list<hscd_choice_active_node *>   nl;
  hscd_scheduler_asap             sched;
  
  std::list<hscd_choice_active_node *> &setTop( hscd_choice_active_node *top ) {
    nl.push_front(top); return nl;
  }
public:
  hscd_top(hscd_choice_active_node *top)
    : sched("xxxx", setTop(top)) {}
};



#endif // _INCLUDED_HSCD_SCHEDULER_HPP
