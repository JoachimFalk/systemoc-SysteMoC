/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 * 
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
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

#include <callib.hpp>

class m_dequant: public smoc_actor {
public:
  smoc_port_in<cal_list<int>::t >   IN;
  smoc_port_in<int>                 FLAGS;
  smoc_port_out<cal_list<int>::t >  OUT;
  smoc_port_out<int>                DC;
  smoc_port_out<int>                MIN;
private:
  int saturate( int x ) { // static type analysis necessary
    return x < -2048
      ? -2048
      : ( x > 2047
          ? 2047
          : x );
  }
  
  int limit(int x, int max) {
    return x > max
      ? max
      : x;
  }
  
  cal_list<int>::t deq( const cal_list<int>::t &v, int QP, int scale, int r) {
    cal_list<int>::t retval0;
    
    cal_list<int>::t _indexes = Integers( 0, 63 );
    for ( cal_list<int>::t::const_iterator i = _indexes.begin();
          i != _indexes.end();
          ++i ) {
      int retval1;
      if (v[*i] == 0) {
        retval1 = 0;
      } else {
        if ( *i == 0 && scale != 0 ) {
          retval1 = saturate( v [0] );
        } else {
          if ( v [*i] < 0 ) {
            retval1 = -limit( QP * (((-v[*i])*2) + 1) - r, 2048 );
          } else {
            retval1 = limit( QP * (((v[*i])*2) + 1) - r, 2047 );
          }
        }
      }
      retval0.push_back(retval1);
    }
    return retval0;
  }
  
  smoc_firing_state start;
public:
  m_dequant(sc_module_name name)
    : smoc_actor(name, start) {
    start = (IN(1 )&& 
             FLAGS(3 )&&
             FLAGS.getValueAt(0) == 1) >>
            (OUT(1 )&&
             MIN(1)) >>
            CALL(m_dequant::action_inter)  >> start
          | (IN(1 )&&
             FLAGS(3 )&&
             FLAGS.getValueAt(0) == 0) >>
            (OUT(1 )&&
             DC(1 )&&
             MIN(1)) >>
            CALL(m_dequant::action_intra)  >> start;
  }
private:
  void action_inter() {
    // pre action code
    // input binding
    const cal_list<int>::t &d     = IN[0];
    //int                  type   = FLAGS[0];
    int                    q      = FLAGS[1];
    int                    scaler = FLAGS[2];
    // action code
    
    int round = cal_bitxor( cal_bitand( q, 1 ), 1 );
    
    // post action code
    OUT[0] = deq(d,q,scaler, round);
    MIN[0] = scaler == 0 ? -256 : 0;
  }

  void action_intra() {
    // pre action code
    // input binding
    const cal_list<int>::t &d     = IN[0];
    //int                  type   = FLAGS[0];
    int                    q      = FLAGS[1];
    int                    scaler = FLAGS[2];
    // output binding
    cal_list<int>::t       dd;
    // action code
    
    int round = cal_bitxor( cal_bitand( q, 1 ), 1 );
    dd = deq( d, q, scaler, round );
    
    // post action code
    DC[0]  = dd[0];
    OUT[0] = dd;
    MIN[0] = scaler == 0 ? -256 : 0;
  }
};
