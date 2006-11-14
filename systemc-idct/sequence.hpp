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

class sequence: publich smoc_actor {
public:
  sc_fifo_in<int> BUF;
  sc_fifo_in<int> DATA;
  sc_fifo_out<int> A, B, C;

private:
  
  // Blocks are assigned a sequence number starting at 1. The output
  // consists of the sequence numbers for the A, B, C block positions
  // of each block. 0 means that the block is not present in the stream.
  // The sequence numbers can be used to index into a line buffer to get
  // ACDC prediction values.

  int mb_x;            // Current macroblock position
  int mb_y;
  int mb_seq[6];       // Sequence numbers for blocks in current MB
  int mb_x_last;       // Previous MB position
  int mb_y_last;
  int mb_seq_last[6];  // Sequence numbers of previous MB
  int block_count = 0;

  // The input stream is cloned, and read in up to the MB directly above
  // the current MB position. buf_next is the next delayed MB (used to tell
  // when buf is completely read in. buf_last is the MB before buf.
  int buf_x;
  int buf_y;
  int buf_seq[6];
  int buf_x_last;
  int buf_y_last;
  int buf_seq_last[6];
  int buf_x_next;
  int buf_y_next;
  int buf_seq_next[6];
  int buf_count = 0;
  int buf_eof;

  int this_n; // Current block number

  bool keep_reading_buf (int bxnext, int bynext, int bx, int by, int mby, int mby) {
    return( (  bynext < (mby - 1) ) ||
            ( (bynext == (mby - 1)) && (bxnext <= mbx) ) ||
            ( (by < (mby - 1) ) )  ||
            ( (by == (mby - 1)) && (bx < mbx) )); 
  }

  bool guard1() {return(buf_count < block_count);}
  bool guard2() {return(buf_count == block_count);}
  bool guard3() {return(DATA[0] < 0);}  /* type<=0 */
  bool guard4() {return(DATA[0] == 1);}
  bool guard5() {return(BUF[0] == 0);}  /* type==0 */
  bool guard6() {return(buf_eof == 0);}
  bool guard7() {return( (this_n == 0) && 
                  ((buf_eof == 1) || (! keep_reading(buf_x_next,buf_y_next,buf_x,buf_y,mb_x,mb_y))));}
  bool guard8() {return( (this_n == 1) && 
                  ((buf_eof == 1) || (! keep_reading(buf_x_next,buf_y_next,buf_x,buf_y,mb_x,mb_y)));}
  bool guard9() {return( (this_n == 2) && 
                  ((buf_eof == 1) || (! keep_reading(buf_x_next,buf_y_next,buf_x,buf_y,mb_x,mb_y)));}
  bool guard10() {return( (this_n == 3) && 
                  ((buf_eof == 1) || (! keep_reading(buf_x_next,buf_y_next,buf_x,buf_y,mb_x,mb_y)));} 
  bool guard11() {return( (this_n >= 3) && 
                  ((buf_eof == 1) || (! keep_reading(buf_x_next,buf_y_next,buf_x,buf_y,mb_x,mb_y)));}

  // Hier kommen die Aktionen

  void flush_buf()   {if (BUF[0] != 1) buf_count = buf_count + 1;};
  
  void reset_actor() {int i;
                      block_count = 0;
    			    mb_x = -1;
    			    mb_y = -1;
                      mb_x_last = -1;
                      mb_y_last = -1;
                      for (i=0; i<=5; i++) mb_seq[i] = 0;
                      buf_count = 0;
                      buf_x = -1;
                      buf_y = -1;
                      buf_x_last = -1;
                      buf_y_last = -1;
                      buf_x_next = -1;
                      buf_y_next = -1;
                      for (i=0; i<=5; i++) buf_seq[i] = 0;
                      for (i=0; i<=5; i++) buf_seq_next[i] = 0;
                      buf_eof = 0;
  }

  void eof_detect()  {block_count = block_count + 1;}

  void discard_block() {}

  void read_block()  {int i;
                      block_count = block_count + 1;
			    if (DATA[1] != mb_x) || (DATA[2] != mb_y)) {
				  // Processing a new macroblock
      			  mb_x_last = mb_x;
      			  mb_y_last = mb_y;
      			  mb_seq_last = mb_seq;
      			  mb_x = DATA[1];
      			  mb_y = DATA[2];
      			  for (i=0; i<=5; i++) mb_seq[i] = 0;
                          }
                      // Watch if next assigment is correct according to spec!
                      mb_seq[DATA[3]] = block_count;
                      this_n = DATA[3];
  }

  void read_buf()    {int i;
                      int a=0;
                      if (BUF[0] != 1) {
				 buf_count = buf_count + 1;
                         if (BUF[1] != buf_x_next) || (BUF[1] != buf_y_next) {				   
        			   buf_x_last = buf_x;
                           buf_y_last = buf_y;
                           buf_seq_last = buf_seq;
                           buf_x = buf_x_next;
                           buf_y = buf_y_next;
                           buf_seq = buf_seq_next;
                           for (i=0; i<=5; i++) buf_seq_next[i] = 0;
        			   buf_x_next = BUF[1];
                           buf_y_next := BUF[2];
                         }
                         if (BUF[0] < 0) {
        			   buf_eof = 1; }
                         else {
                           // Watch if next assigment is correct according to spec!
                           mb_seq[DATA[3]] = buf_count;
                      }
  }

  void predict.b0() {int a=0; 
                     int b=0;
                     int c=0;
                     if (mb_x_last == (mb_x-1)) && (mb_y_last == mb_y)
                        a = mb_seq_last[1];
                     if (buf_x_last == (mb_x-1)) && (buf_y_last == (mb_y-1))
                        b = buf_seq_last[3];
                     if (buf_x == mb_x) && (buf_y == (mb_y-1))
                        c = buf_seq[2];
                     // Ausgänge erzeugen
                     A[0]=a; B[0]=b; C[0]=c;
  }
  
  void predict.b1() {int a=mb_seq[0]; 
                     int b=0;
                     int c=0;
                     if (buf_x == mb_x) && (buf_y == (mb_y-1)) {
                        b = buf_seq[2];
                        c = buf_seq[3];
                        }
                     // Ausgänge erzeugen
                     A[0]=a; B[0]=b; C[0]=c;
  }
  
  void predict.b2() {int a=0; 
                     int b=0;
                     int c=mb_seq[0];
                     if (mb_x_last == (mb_x-1)) && (mb_y_last == mb_y) {
                        a = mb_seq_last[3];
                        b = mb_seq_last[2];
                        }
                     // Ausgänge erzeugen
                     A[0]=a; B[0]=b; C[0]=c;
  }


  void predict.b3() {int a=mb_seq[2]; 
                     int b=mb_seq[0];
                     int c=mb_seq[1];
                     // Ausgänge erzeugen
                     A[0]=a; B[0]=b; C[0]=c;
  }
 
  
  void predict.b45() {int a=0; 
                      int b=0;
                      int c=0;
                      if (mb_x_last == (mb_x-1)) && (mb_y_last == mb_y)
                        a = mb_seq_last[this_n];
                      if (buf_x_last == (mb_x-1)) && (buf_y_last == (mb_y-1))
                        b = buf_seq_last[this_n];
                      if (buf_x == mb_x) && (buf_y == (mb_y-1))
                        c = buf_seq[this_n];
                      // Ausgänge erzeugen
                      A[0]=a; B[0]=b; C[0]=c;
  }
  
smoc_firing_state reset, read, process;

public:
  sequence(sc_module_name name)
    : smoc_actor(name, reset), {
    reset   = (BUF.getAvailableTokens() >= 4 &&
              guard(&sequence::guard1) )     >>
              CALL(sequence::flush_buf)   >> reset 
            | (guard(&sequence::guard2) ) >>
		  CALL(sequence::reset_actor)    >> read;
    read    = (DATA.getAvailableTokens() >= 4 &&
              guard(&sequence:guard3) )      >>
              call(&sequence:eof_detect)     >> reset
            | (DATA.getAvailableTokens() >= 4 && 
              guard(&sequence:guard4) )      >> 
		  CALL(sequence::discard_block)  >> read
            | (DATA.getAvailableTokens() >= 4 &&
              guard(&sequence:guard5) )      >>
              call(&sequence:read_block)     >> process;  
    process = (BUF.getAvailableTokens() >= 4) &&
              guard(&sequence::guard6) )     >>
              CALL(sequence::read_buf)       >> sequence
            | (guard(&sequence::guard7)      >>
              call(&sequence:predict.b0)     >> read
            | (guard(&sequence::guard8)      >>
              call(&sequence:predict.b1)     >> read
		| (guard(&sequence::guard9)      >>
              call(&sequence:predict.b2)     >> read
		| (guard(&sequence::guard10)      >>
              call(&sequence:predict.b3)     >> read
		| (guard(&sequence::guard11)      >>
              call(&sequence:predict.b45)     >> read;
  }
};
