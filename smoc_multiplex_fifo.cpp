// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#include <systemoc/smoc_config.h>
#include <systemoc/smoc_multiplex_fifo.hpp>
#include <systemoc/smoc_ngx_sync.hpp>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/hscd_vpc_Director.h>
#endif //SYSTEMOC_ENABLE_VPC

using namespace SysteMoC::NGX;
using namespace SysteMoC::NGXSync;

smoc_multiplex_vfifo_kind::smoc_multiplex_vfifo_kind( const chan_init &i )
  : smoc_nonconflicting_chan(i.name),
    Queue3Ptr(pSharedFifoMem->fifoDepth),
    fifoId(i.fifoId),
    pSharedFifoMem(i.pSharedFifoMem),
    tokenId(0)
{
  pSharedFifoMem->registerVFifo(this);

  // NGX --> SystemC
  if(NGXConfig::getInstance().hasNGX()) {

    Fifo::ConstPtr fifo =
      objAs<Fifo>(NGXCache::getInstance().get(this));

    if(fifo) {
      fsize = fifo->size().get() + 1;
    }
    else {
      // XML node missing or no Fifo
    }
  }

  // for lazy % overflow protection fsize must be less than half the datatype
  //  size
  assert(fsize < (MAX_TYPE(size_t) >> 1));
}

smoc_multiplex_vfifo_kind::~smoc_multiplex_vfifo_kind() {
  pSharedFifoMem->deregisterVFifo(this);
}
