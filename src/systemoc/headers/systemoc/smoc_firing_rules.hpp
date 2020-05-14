// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Christian Zebelein <christian.zebelein@fau.de>
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2013 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2014 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SYSTEMOC_SMOC_FIRING_RULES_HPP
#define _INCLUDED_SYSTEMOC_SMOC_FIRING_RULES_HPP

#include "../smoc/smoc_base_state.hpp"
using ::smoc::smoc_base_state;

#include "../smoc/smoc_junction_state.hpp"
using ::smoc::smoc_junction_state;

#include "../smoc/smoc_multi_state.hpp"
using ::smoc::IN;
using ::smoc::smoc_multi_state;

#include "../smoc/smoc_state.hpp"
using ::smoc::smoc_state;
typedef ::smoc::smoc_state smoc_hierarchical_state;

#include "../smoc/smoc_and_state.hpp"
using ::smoc::smoc_and_state;

#include "../smoc/smoc_xor_state.hpp"
using ::smoc::smoc_xor_state;

#include "../smoc/smoc_firing_state.hpp"
using ::smoc::smoc_firing_state;

#include "../smoc/smoc_firing_rule.hpp"
using ::smoc::smoc_firing_rule;

#include "../smoc/smoc_transition.hpp"
using ::smoc::smoc_transition;
using ::smoc::smoc_transition_list;

#endif /* _INCLUDED_SYSTEMOC_SMOC_FIRING_RULES_HPP */
