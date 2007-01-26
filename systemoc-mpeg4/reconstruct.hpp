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

class m_reconstruct: public smoc_actor {
public:
  smoc_port_in<cal_list<int>::t>  DATA;
  smoc_port_in<int>               FLAGS;
  smoc_port_in<int>               PARAM;
  smoc_port_in<int>               A, B, C, DC;
  smoc_port_out<cal_list<int>::t> OUT;
  smoc_port_out<int>              OFLAGS;
private:
  const int DCVAL;
  const int LBSIZE;
  
// Die ersten 64 Werte stellen dar, wie die Originalscanfolge (Zeile für Zeile) be JPEG 
// wieder aus den Zigzac-gescannten Werten regeneriert werden kann. Bsp.: die 5 sagt, dass
// das Bildelement (0. Zeile, 2. Spalte) als 6. Element gescannt wurde (beginnt bei 0)
// Die zweiten und dritten 64 Werte werden gebraucht bei Intra-Frame-Codierung
  static const int zigzag[3][64];
  
// initList generiert eine Liste mit size Elementen und initialisiert alle Werte mit dem Wert v
  template <typename T1>
  typename cal_list<T1>::t initList(const T1 &v, int size ) {
    typename cal_list<T1>::t retval;
    for (int i = 0; i < size; i++ )
      retval.push_back(v);
    return retval;
  }
  
// Note: LBSIZE should be 6 * (max_line_width_in_mb + 2 )
// FIXME: lb should be mutable, not assignable.

// lb steht für einen lokalen Puffer, der pro Block 15 Werte speichert, die bei Intra-Frame gebraucht werden.
// lb_in stellt den aktuell zu bearbeitenden Block dar.
// block_count auch; Unterschied: block_count wird bei intra-Frame immer, lb_in modulo LBSIZE erhöht.
  cal_list<cal_list<int>::t>::t lb;
  int lb_in;
  int block_count;
  
// scan liefert die Liste von 64 Elementen eines Blocks zurück mit der Reihenfolge, die durch 
// die Zigzag-Scan-Ordnung pat gegeben ist
  cal_list<int>::t scan(const cal_list<int>::t &d, const int pat[]) {
    cal_list<int>::t retval;
    for (int i = 0; i <= 63; i++) {
      retval.push_back(d[pat[i]]);
    }
    return retval;
  }
  
// 1. Aktion: Ist der Guard type<0, dann wird eof ("end of frame") ausgeführt.
// Dabei wird lediglich die Variable block_count wieder of 0 gesetzt.
  void action_eof() {
    // pre action code
    // input binding
    //const int type = PARAM[0];
    //const int x    = PARAM[1];
    //const int y    = PARAM[2];
    //const int n    = PARAM[3];
    // action code
    block_count = 0;
  }

// 2. Aktion: Ist der Guard type=1, dann wird als Ausgangsfolge ein Block erzeugt
// mit der durch zigzag[0] bestimmten Reihenfolge (vgl. JPEG)
// Als Ausgangsflags wird type=1 weitergegeben sowie die Werte q und scaler durchgereicht.
  void action_inter() {
    // pre action code
    // input binding
    //const int               type   = PARAM[0];
    //const int               x      = PARAM[1];
    //const int               y      = PARAM[2];
    //const int               n      = PARAM[3];
    //const int               ac     = FLAGS[0];
    const int               q      = FLAGS[1];
    const int               scaler = FLAGS[2];
    const cal_list<int>::t &d      = DATA[0];
    // action code
    
    // post action code
    OUT[0] = scan(d, zigzag[0]); OFLAGS[0] = 1; OFLAGS[1] = q; OFLAGS[2] = scaler;
  }
  
// 3. Aktion: getdc setzt den Wert im local buffer des aktuellen Blocks lb_in auf den Wert dc.
  void action_getdc() {
    // pre action code
    // input binding
    const int dc = DC[0];
    // action code
    lb[lb_in][0] = dc;
    // post action code
  }
  
// lb_get_val holt aus dem Puffer buf einen Wert zurück. Dabei bestimmen
// pos: lb_in;
// size: Gesamtgröße von buf;
// seq: sequencer
// head: block_count
// index: Offset
  int lb_get_val (cal_list<cal_list<int>::t>::t buf, int pos, int size, int seq, int head, int index) {
    return seq == 0
      ? DCVAL
      : ( pos + (seq - head) < 0
          ? buf[pos + ( seq - head ) + size][index]
          : buf[pos + ( seq - head )       ][index]
        );
  }
  
// abs berechnet den Betrag einer Zahl
  int abs(int x) {
    return x < 0
      ? -x
      : x;
  }

// 4. Aktion: Sehr ähnlich wie Aktion inter, außer dass hier noch die drei Werte 
// A, B, und C verarbeitet werden. Diese bestimmen, ob die Zigzag-Tabelle 1 oder 2 
// verwendet wird. Die Aktion wird ausgeführt, wenn der Guard type=0 ist.
  void action_intra() {
    // pre action code
    // input binding
    //const int               type   = PARAM[0];
    //const int               x      = PARAM[1];
    //const int               y      = PARAM[2];
    //const int               n      = PARAM[3];
    const int               ac     = FLAGS[0];
    const int               q      = FLAGS[1];
    const int               scaler = FLAGS[2];
    const cal_list<int>::t &dd     = DATA[0];
    const int               a      = A[0];
    const int               b      = B[0];
    const int               c      = C[0];
    // action code
    int dca = lb_get_val( lb, lb_in, LBSIZE, a, block_count, 0 );
    int dcb = lb_get_val( lb, lb_in, LBSIZE, b, block_count, 0 );
    int dcc = lb_get_val( lb, lb_in, LBSIZE, c, block_count, 0 );
    int horiz = abs( dcb - dcc );
    int vert  = abs( dca - dcb );
    int zsel = 0;
    int pred;
    int t;
    // lokale Kopie der Eingabedaten (1 Block) erzeugen, da diese hier verändert werden
    // bevor die Ausgabe erfolgt.
    cal_list<int>::t d;
    for ( int i = 0; i <= 63; i++ )
      d.push_back( dd[i] );
    if (vert < horiz) {
      pred = dcc;
      if (ac != 0) {
        zsel = 2;
        if (c > 0) {
          t = zigzag[2][1]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, c, block_count, 1 ); 
          t = zigzag[2][2]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, c, block_count, 2 ); 
          t = zigzag[2][3]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, c, block_count, 3 ); 
          t = zigzag[2][4]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, c, block_count, 4 ); 
          t = zigzag[2][5]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, c, block_count, 5 ); 
          t = zigzag[2][6]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, c, block_count, 6 ); 
          t = zigzag[2][7]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, c, block_count, 7 ); 
        }
      }
    } else {
      pred = dca;
      if (ac != 0) {
        zsel = 1;
        if (a > 0) {
          t = zigzag[1][ 8]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, a, block_count, 8  ); 
          t = zigzag[1][16]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, a, block_count, 9  ); 
          t = zigzag[1][24]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, a, block_count, 10 ); 
          t = zigzag[1][32]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, a, block_count, 11 ); 
          t = zigzag[1][40]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, a, block_count, 12 ); 
          t = zigzag[1][48]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, a, block_count, 13 ); 
          t = zigzag[1][56]; d [t] = d [t] + lb_get_val( lb, lb_in, LBSIZE, a, block_count, 14 ); 
        }
      }
    }
    // In folgender Berechnung soll eine Integer-Division sein.
    d[0] = (d[0]  * scaler ) + ( ( pred + cal_rshift(scaler,2) ) / scaler ) * scaler ;
    
    // Hier werden block_count erhöht und lb_in modulo LBSIZE; 
    block_count = block_count + 1;
    lb_in = lb_in + 1;
    if (lb_in >= LBSIZE) {
      lb_in = 0;
    }
    // Zuletzt werden die erste Zeile und erste Spalte des nächsten lokalen Puffers auf die Werte 
    // des letzten Blocks gesetzt.
    {
      //lb [lb_in] := [ d [zigzag [zsel, i]] : for i in [ 0, 1, 2, 3, 4, 5, 6, 7, 8, 16, 24, 32, 40, 48, 56 ] ] ;
      const int indices[] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 16, 24, 32, 40, 48, 56 };
      for ( unsigned int iter = 0; iter < sizeof(indices)/sizeof(indices[0]); ++iter )
        lb[lb_in][iter] = d[zigzag[zsel][indices[iter]]];
    }
    // post action code
    OUT[0] = scan(d, zigzag[zsel]); OFLAGS[0] = 0; OFLAGS[1] = q; OFLAGS[2] = scaler;
  }
  
  smoc_firing_state read, wait;
public:
  m_reconstruct(sc_module_name name, int DCVAL, int LBSIZE)
    : smoc_actor(name, read),
      DCVAL(DCVAL), LBSIZE(LBSIZE),
      lb( initList( initList( 0, 15 ), LBSIZE  ) ),
      lb_in(0), block_count(0)  {
    // Anfangszustand: read. Von hier aus geht es nach read über die 1. Aktion eof (falls guard type<0),
    // bzw. ebenfalls nach read über Aktion 2, (falls guard type=1) bzw. nach Zustand wait (Aktion3)
    // (falls guard type=0).
    // Von Zustand wait geht es immer wieder zurück in den Zustand read, sobald die 3. Aktion
    // (getdc) ausgeführt wurde.
    read = (PARAM.getAvailableTokens() >= 4 &&
            PARAM.getValueAt(0) < 0)            >>
           CALL(m_reconstruct::action_eof)      >> read
         | (PARAM.getAvailableTokens() >= 4 &&
            FLAGS.getAvailableTokens() >= 3 &&
            DATA.getAvailableTokens()  >= 1 &&
            PARAM.getValueAt(0) == 1)           >>
           (OUT.getAvailableSpace()    >= 1 &&
            OFLAGS.getAvailableSpace() >= 3)    >>
           CALL(m_reconstruct::action_inter)    >> read
         | (PARAM.getAvailableTokens() >= 4 &&
            FLAGS.getAvailableTokens() >= 3 &&
            DATA.getAvailableTokens()  >= 1 &&
            A.getAvailableTokens()     >= 1 &&
            B.getAvailableTokens()     >= 1 &&
            C.getAvailableTokens()     >= 1 &&
            PARAM.getValueAt(0) == 0)           >> 
           (OUT.getAvailableSpace()    >= 1 &&
            OFLAGS.getAvailableSpace() >= 3)    >>
           CALL(m_reconstruct::action_intra)    >> wait;
    
    wait = (DC.getAvailableTokens()    >= 1)    >>
           CALL(m_reconstruct::action_getdc)    >> read;
  }
};

const int m_reconstruct::zigzag[3][64] = {
  {  0,  1,  5,  6, 14, 15, 27, 28,  2,  4,  7, 13, 16, 26, 29, 42,
     3,  8, 12, 17, 25, 30, 41, 43,  9, 11, 18, 24, 31, 40, 44, 53,
    10, 19, 23, 32, 39, 45, 52, 54, 20, 22, 33, 38, 46, 51, 55, 60,
    21, 34, 37, 47, 50, 56, 59, 61, 35, 36, 48, 49, 57, 58, 62, 63 },
  {  0,  4,  6, 20, 22, 36, 38, 52,  1,  5,  7, 21, 23, 37, 39, 53,
     2,  8, 19, 24, 34, 40, 50, 54,  3,  9, 18, 25, 35, 41, 51, 55,
    10, 17, 26, 30, 42, 46, 56, 60, 11, 16, 27, 31, 43, 47, 57, 61,
    12, 15, 28, 32, 44, 48, 58, 62, 13, 14, 29, 33, 45, 49, 59, 63 },
  {  0,  1,  2,  3, 10, 11, 12, 13,  4,  5,  8,  9, 17, 16, 15, 14,
     6,  7, 19, 18, 26, 27, 28, 29, 20, 21, 24, 25, 30, 31, 32, 33,
    22, 23, 34, 35, 42, 43, 44, 45, 36, 37, 40, 41, 46, 47, 48, 49,
    38, 39, 50, 51, 56, 57, 58, 59, 52, 53, 54, 55, 60, 61, 62, 63 }
};
