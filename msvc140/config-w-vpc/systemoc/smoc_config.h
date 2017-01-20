/* vim: set sw=2 ts=8 syn=c: */
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SMOC_CONFIG_H
#define _INCLUDED_SMOC_CONFIG_H

/* Defined to SysteMoC version */
#define SYSTEMOC_VERSION "0.9"
#define SYSTEMOC_MAJOR_VERSION "0"
#define SYSTEMOC_MINOR_VERSION "9"
#define SYSTEMOC_MICRO_VERSION "0"

/* Defined if debug support is enabled */
#undef SYSTEMOC_ENABLE_DEBUG

/* Defined if SGX support is enabled */
#undef SYSTEMOC_ENABLE_SGX

/* Defined if transition tracing support is enabled */
#undef SYSTEMOC_ENABLE_TRANSITION_TRACE

/* Defined if systemoc tracing support is enabled */
#undef SYSTEMOC_ENABLE_DATAFLOW_TRACE

/* Defined if hooking infrastructure is provided */
#undef SYSTEMOC_ENABLE_HOOKING

/* Define if you want SystemC-VPC Support */
#undef SYSTEMOC_ENABLE_VPC

/* Define if you want Meastro Support */
#define SYSTEMOC_ENABLE_MAESTROMM

#if defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE) || defined(SYSTEMOC_ENABLE_TRANSITION_TRACE) || defined(SYSTEMOC_ENABLE_SGX)
# define SYSTEMOC_NEED_IDS
#endif

#if defined(SYSTEMOC_ENABLE_VPC)
# define SYSTEMOC_PORT_ACCESS_COUNTER
#endif

#if defined(SYSTEMOC_DEBUG) && defined(SYSTEMOC_ENABLE_VPC)
# define SYSTEMOC_DEBUG_VPC_IF
#endif

#endif /* _INCLUDED_SMOC_CONFIG_H */
