// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SYSTEMOC_SMOC_ACTOR_HPP
#define _INCLUDED_SYSTEMOC_SMOC_ACTOR_HPP

#include "../smoc/smoc_actor.hpp"

// Not needed for actor, but users will likely require this.
#include "smoc_firing_rules.hpp"

#include <systemoc/smoc_config.h>

using smoc::smoc_actor;

// Legacy macros
#define VAR(variable)       SMOC_VAR(variable)
#define TILL(event)         SMOC_TILL(event)
#define LITERAL(lit)        SMOC_LITERAL(lit)
#define CALL(func)          SMOC_CALL(func)
#define GUARD(func)         SMOC_GUARD(func)
#ifdef SYSTEMOC_ENABLE_MAESTRO
# define CALLI(ins,func)    this->call(ins, &func, #func)
# define GUARDI(ins,func)   this->guard(ins, &func, #func)
#endif //SYSTEMOC_ENABLE_MAESTRO

#endif /* _INCLUDED_SYSTEMOC_SMOC_ACTOR_HPP */
