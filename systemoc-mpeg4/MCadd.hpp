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

#ifndef _INCLUDED_MCadd_HPP
#define _INCLUDED_MCadd_HPP

#include "callib.hpp"

//using Expr::guard;

/*
  Motion Compensation

*/

class m_mc_add: public smoc_actor {
public:
  /* input and output ports */
  smoc_port_in<cal_list<int>::t >  LAST; //last MB?
  smoc_port_in<cal_list<int>::t >  TEX;  //Texture?
  smoc_port_in<int>                MV;
	
  smoc_port_out<cal_list<int>::t >  VID;
	
	
  /* constructor            */
  m_mc_add(sc_module_name name, int WINSIZE, int MAXMBWIDTH );
  /*	WINSIZE is the search window size, measured in macroblocks
    - should be an odd number, probably
    MAXMB width is the maximum line width, measured in macroblocks
  */
private:	
  /* parameters */
  const int winsize;
  //WINSIZE is the search window size, measured in macroblocks
  //- should be an odd number, probably
  const int maxmbwidth;
  //MAXMB width is the maximum line width, measured in macroblocks	
	
private:
  int MBwidth;
  int MBheight;
  int rounding_type;

  // Expected coordinates for the next block
  int next_x;
  int next_y;
  int next_n;

  // Number of blocks in interal and external memory,
  // expected counts in steady state.
  int int_buf_count;
  int int_buf_count_ss;
  int ext_buf_count;
  int ext_buf_count_ss;
  int frame_count;
  int frame_number;

  // block number of block most recently added to internal memory
  // (i.e. ranges from 0 to ( 6 * MBwidth * MBheight - 1 )
  int head_index;
  
  /* macroblock buffer */
  int MBbuf_size;  //buffer size
  int MBbuf_ptr;   //pointer (in form of an index) to buffer
  cal_list<cal_list<int>::t >::t MBbuf;  //buffer
  

private:
  //init a list of given size with the value init_value
  template < typename T1 >
  typename cal_list<T1>::t initList(const T1 &init_value, const int size);
	
  /* ring buffer implementation */
  // add element to ring buffer
  template < typename T1 >
  void MBbuf_add(const T1 &element);
	
  //read element from ring-buffer without changing any pointer
  //offset is relative to current pointer position (in other words:
  //where we have read last)
  cal_list<int>::t MBbuf_get(const int offset);
	
private:
  int detect_skipped(int x, int y, int n);
  // Advance next_* to refer to the next block in the sequence
  void next_block(void);
	
private:
  int xadjust( int n, int dx );
  int yadjust( int n, int dy );

  // Get individual pixel for motion comp
  // dx and dy must already be clipped to frame boundary
  // x,y,n are macroblock coords
  int get_pixel( int x, int y, int n, int dx, int dy );
  
  // Limit motion dx or dy to image boundary
  int xyclip( int xy, int xymax, int dxy );

  // Convert MB x and block number to pixel x
  int xpixel( int x, int n );

  // Convert MB y and block number to pixel y
  int ypixel( int y, int n );

  cal_list<int>::t comp_block( int x, int y, int n, int mvx, int mvy );
  int clip( int x );
  cal_list<int>::t combine( cal_list<int>::t a, cal_list<int>::t b );

private:
  //states of state machine
  smoc_firing_state state_process;
  smoc_firing_state state_update;  

  //construction of the SysteMoG State-Machine
  //called in the constructor
  void construct_fsm(void);


private:
  /* Firing Actions of state machine */
  void action_newFrame(void);
  void action_docmd_missingBlock(void);
  //bool guard_docmd_missingBlock(void) const;
	
  /* commands */
  // 0   motion only, mv  = 0
  // 1   motion only, mv != 0
  // 4   motion and texture
  // 6   texture only
  	
  void action_docmd_textureOnly(void);
  //bool guard_docmd_textureOnly(void) const;
  void action_docmd_nothing(void);
  //bool guard_docmd_nothing(void) const;
  void action_docmd_motionOnly(void);
  //bool guard_docmd_motionOnly(void) const;
  void action_docmd_both(void);
  //bool guard_docmd_both(void) const;
  
  // Previous frame queue needs to fill up
  void action_mbbuf_tooEmptyInt(void);
  //bool guard_mbbuf_tooEmptyInt(void) const;
  void action_mbbuf_tooEmptyExt(void);
  //bool guard_mbbuf_tooEmptyExt(void) const;
  void action_mbbuf_tooFull(void);
  //bool guard_mbbuf_tooFull(void) const;
  void action_mbbuf_justRight(void);
  //bool guard_mbbuf_justRight(void) const;

private:

};

//Constructor
m_mc_add::m_mc_add(sc_module_name name, int WINSIZE, int MAXMBWIDTH )
  : smoc_actor(name, state_process),
    winsize(WINSIZE),maxmbwidth(MAXMBWIDTH),
    MBwidth(0),
    MBheight(0),
    rounding_type(0),
    int_buf_count(0),
    ext_buf_count(0),
    frame_number(0)
{
	
  //init macroblock buffer
  MBbuf_size = 6 * (((WINSIZE-1) * MAXMBWIDTH) + WINSIZE);
  MBbuf_ptr = 0;
  // initialize buffer content with zero
  MBbuf = initList( initList( 0, 64 ), MBbuf_size );

  construct_fsm();
}

//construction of the state-machine
void m_mc_add::construct_fsm(void){
  state_process = 
    //newFrame
    (MV(6 )&&
     MV.getValueAt(0) == -1) >> //cmd
    CALL(m_mc_add::action_newFrame)  >> state_process
    
    //docmd_missingBlock
    |(MV(6 )&&
      MV.getValueAt(0) >= 0 &&  //cmd
      //  guard(&m_mc_add::guard_docmd_missingBlock)) >>
      (
       (MV.getValueAt(1) != var(next_x)) || //x
       (MV.getValueAt(2) != var(next_y)) || //y
       (MV.getValueAt(3) != var(next_n)) //n
       )
      ) >>
    (VID(1)) >>
    CALL(m_mc_add::action_docmd_missingBlock)  >> state_update
    
    //docmd_textureOnly
    |(MV(6 )&&
      MV.getValueAt(0) == 6 &&  //cmd
      //guard(&m_mc_add::guard_docmd_textureOnly) &&
      (
       (MV.getValueAt(1) == var(next_x)) && //x
       (MV.getValueAt(2) == var(next_y)) && //y
       (MV.getValueAt(3) == var(next_n)) //n
       ) &&
      TEX(1)) >>
    (VID(1)) >>
    CALL(m_mc_add::action_docmd_textureOnly)  >> state_update
    
    //docmd_nothing
    |(MV(6 )&&
      MV.getValueAt(0) == 0 &&  //cmd
      //      guard(&m_mc_add::guard_docmd_nothing)) >>
      (
       (MV.getValueAt(1) == var(next_x)) && //x
       (MV.getValueAt(2) == var(next_y)) && //y
       (MV.getValueAt(3) == var(next_n))  //n
       )
      ) >>
    (VID(1)) >>
    CALL(m_mc_add::action_docmd_nothing)  >> state_update
    
    //docmd_motionOnly (1)
    |(MV(6 )&&
      MV.getValueAt(0) == 0 &&  //cmd
      //guard(&m_mc_add::guard_docmd_motionOnly)
      (
       (MV.getValueAt(1) == var(next_x)) && //x
       (MV.getValueAt(2) == var(next_y)) && //y
       (MV.getValueAt(3) == var(next_n))  //n
       )
      ) >>
    (VID(1)) >>
    CALL(m_mc_add::action_docmd_motionOnly)  >> state_update

    //docmd_motionOnly (2)
    |(MV(6 )&&
      MV.getValueAt(0) == 1 &&  //cmd
      //guard(&m_mc_add::guard_docmd_motionOnly)
      (
       (MV.getValueAt(1) == var(next_x)) && //x
       (MV.getValueAt(2) == var(next_y)) && //y
       (MV.getValueAt(3) == var(next_n))  //n
       )
      ) >>
    (VID(1)) >>
    CALL(m_mc_add::action_docmd_motionOnly)  >> state_update
    
    //action_docmd_both
    |(MV(6 )&&
      MV.getValueAt(0) == 4 &&  //cmd
      //guard(&m_mc_add::guard_docmd_both) &&
      (
       (MV.getValueAt(1) == var(next_x)) && //x
       (MV.getValueAt(2) == var(next_y)) && //y
       (MV.getValueAt(3) == var(next_n))  //n
       ) &&
      TEX(1)) >>
    (VID(1)) >>
    CALL(m_mc_add::action_docmd_both)  >> state_update;



  state_update = 
    
    //action_mbbuf_tooEmptyInt
    (LAST(1 )&&
     //guard(&m_mc_add::guard_mbbuf_tooEmptyInt)
     (var(int_buf_count) < var(int_buf_count_ss))
     ) >>
    CALL(m_mc_add::action_mbbuf_tooEmptyInt)  >> state_process
    
    //action_mbbuf_tooEmptyExt
    |(
      //guard(&m_mc_add::guard_mbbuf_tooEmptyExt) 
      (var(int_buf_count) == var(int_buf_count_ss)) && 
      (var(ext_buf_count) < var(ext_buf_count_ss))
      ) >>
    CALL(m_mc_add::action_mbbuf_tooEmptyExt)  >> state_process
    
    //action_mbbuf_tooFull
    |(
      //guard(&m_mc_add::guard_mbbuf_tooFull) >>
      ((var(int_buf_count) > var(int_buf_count_ss)) ||
       (var(ext_buf_count) > var(ext_buf_count_ss))
       )
      ) >> 
    CALL(m_mc_add::action_mbbuf_tooFull)  >> state_process
    
    //action_mbbuf_justRight
    |(LAST(1 )&&
      //guard(&m_mc_add::guard_mbbuf_justRight)
      (
       (var(int_buf_count) == var(int_buf_count_ss)) &&
       (var(ext_buf_count) == var(ext_buf_count_ss))
       )
      ) >>
    CALL(m_mc_add::action_mbbuf_justRight)  >> state_process;
}

//type for parameter size: must be derived by type-analysis
template < typename T1 >
typename cal_list<T1>::t m_mc_add::initList(const T1 &init_value, const int size){
  //we supposed int type. In principle, template must be used!
	
  typename cal_list<T1>::t return_value;
  for(int i=1; i <= size; i++)
    return_value.push_back(init_value);
	
  return (return_value);
}

// code analysis of CAL-actor can reveal: T1 will be of type
// cal_list<int>::t
template < typename T1 >
void m_mc_add::MBbuf_add(const T1 &element){
	
  //with this construction, the first access 
  //to the ring buffer is to position	1, not 0
  //(copied from CAL-code)
	
  MBbuf_ptr = MBbuf_ptr + 1;
  if (MBbuf_ptr >= MBbuf_size){
    MBbuf_ptr = 0;
  }
    
  //copy element into ring buffer
  MBbuf[MBbuf_ptr] = element;
}

//return type has been derived by code-analysis
cal_list<int>::t m_mc_add::MBbuf_get(const int offset){	
	
  //calculate position where to read
  int ptr = MBbuf_ptr - offset;
	
  //corrected read pointer
  int ptr_wrapped;
    
  //correct read position, if pointer "left" memory region
  ptr_wrapped = ptr < 0 			? 	ptr + MBbuf_size : 
    //shit happens, if offset
    //had been verly large
    ptr >= MBbuf_size ? 	ptr - MBbuf_size:
    //shit happens, if offset
    //had been very small
    ptr;
    									
  //read value
  return MBbuf[ptr_wrapped];
    
}

void m_mc_add::action_newFrame(void){
  //reference input tokens (input binding)
  // int cmd          = MV[0];
  int x            = MV[1];
  int y            = MV[2];
  int rounding     = MV[3];
  // int &unused1   = MV[4];
  // int &unused2	  = MV[5];
	
  //no output tokens
  if ((x != MBwidth) || (y != MBheight)){
    int_buf_count_ss = 6 * (((winsize-1) * x) + winsize);
    int_buf_count    = 6 * cal_rshift( ((winsize-1) * x) + winsize, 1 );
    frame_count      = x * y * 6;
    head_index       = int_buf_count + 5;
    ext_buf_count_ss = frame_count + int_buf_count - int_buf_count_ss;
      
#ifdef DEBUG_OUTPUT
    cout << "Start of frame (" << x << " x " << y << ")  rounding = " << rounding_type;
    cout << "frame count = " << frame_count << ", int_buf_count = " << int_buf_count << "(ss ";
    cout << int_buf_count_ss << "), ext_buf_count_ss = " << ext_buf_count_ss << endl;
#endif //DEBUG_OUTPUT

    MBwidth  = x;
    MBheight = y;
  }
  rounding_type = rounding;
  next_x = 0;
  next_y = 0;
  next_n = 0;
  frame_number = frame_number + 1;
#ifdef DEBUG_OUTPUT
  if (frame_number == 190)
    cout << "Frame " << frame_number << ", rounding = " << rounding_type << endl;
#endif //DEBUG_OUTPUT
}

//types derived by code analysis
int m_mc_add::detect_skipped(int x, int y, int n){
  int return_value;
	
  return_value = ((x == next_x) && (y == next_y) && (n == next_n)) ? 0 : 1;
	
  return return_value;
}

void m_mc_add::next_block(void){
  next_n = next_n + 1;
  if (next_n == 6){
    next_n = 0;
    next_x = next_x + 1;
    if (next_x == MBwidth){
      next_x = 0;
      next_y = next_y + 1;
      if (next_y == MBheight){
	next_y = 0;
      }
    }
  }
}

void m_mc_add::action_docmd_missingBlock(void){
  // reference input tokens (input binding)
  // int cmd = MV[0];
  // int x   = MV[1];
  // int y   = MV[2];
  // int n   = MV[3];
  // int mvx = MV[4];
  // int mvy = MV[5];
	
  //output binding
	
  cout << "Missing block at (" << next_x << "," << next_y << "," << next_n << ")" << endl;
  next_block();
	
  VID[0] = initList(next_n < 4 ? 0 : 128,64);
}

/*
  bool m_mc_add::guard_docmd_missingBlock(void) const{
  return(detect_skipped( x, y, n ) == 1);
  }
*/

// cmds
// 0   motion only, mv  = 0
// 1   motion only, mv != 0
// 4   motion and texture
// 6   texture only

void m_mc_add::action_docmd_textureOnly(void){
  // reference input tokens (input binding)
  //int cmd   = MV[0];
  int x     = MV[1];
  int y     = MV[2];
  int n     = MV[3];
  //int mvx   = MV[4];
  //int mvy   = MV[5];
	
  const cal_list<int>::t &tex = TEX[0];
	
  if ((frame_number == 190) && (y == 0) && (x < 2) && (n < 4)) {
    cout << "TEX only at (" << next_x << "," << next_y << "," << next_n << ")" << endl;
  }
#ifdef DEBUG_OUTPUT
  if ((x == 0) && (y == 0) && (n == 3)){
    cout << "INTRA at 0,0,3" << endl;
    cout << tex << endl;
  }
#endif //DEBUG_OUTPUT
  next_block();
	
  VID[0] = tex;	
}
/*
bool m_mc_add::guard_docmd_textureOnly(void) const{
  return(detect_skipped( x, y, n ) == 0);
}
*/

int m_mc_add::xadjust( int n, int dx ){
  int return_value;
	
  if (n < 4) {
    return_value = (6 * cal_rshift( dx, 4 ));
      
    if (cal_bitand( dx, 8 ) == 0){
      return_value += 0;
    }else{
      if ((n == 0) || (n == 2)){
	return_value += 1;
      }else{
	return_value += 5;
      }
    }
  }else{
    return_value = 6 * cal_rshift( dx, 3 );	
  }
	
  return return_value;
}

int m_mc_add::yadjust( int n, int dy ){
  int return_value;
	
  if (n < 4) {
    return_value = (6 * MBwidth * cal_rshift( dy, 4 ));
    	
    if (cal_bitand( dy, 8 ) == 0){
      return_value += 0;
    }else{
      if ((n == 0) || (n == 1)) {
	return_value += 2;
      }else{
	return_value += (6 * MBwidth) - 2;
      }
    }
  }else{
    return_value = 6 * MBwidth * cal_rshift( dy, 3 );
  }
    
  return return_value;
}

// Get individual pixel for motion comp
// dx and dy must already be clipped to frame boundary
// x,y,n are macroblock coords
int  m_mc_add::get_pixel( int x, int y, int n, int dx, int dy ){
  return (MBbuf_get((head_index - xadjust(n,dx)) - yadjust(n,dy))[ cal_bitand( dx, 7) + cal_lshift( cal_bitand( dy, 7), 3) ]);
}

// Limit motion dx or dy to image boundary
int m_mc_add::xyclip( int xy, int xymax, int dxy ){
  return (
	  ((xy + dxy) < 0)      ? -xy :
	  (xy + dxy >= xymax) ? xymax - xy - 1 : dxy
	  );
}

// Convert MB x and block number to pixel x
int m_mc_add::xpixel( int x, int n ){
  int return_value;
  
  if ((n == 0) || (n == 2)){
    return_value = cal_lshift( x, 4 );
  }else{
    if ((n == 1) || (n == 3)){
      return_value = cal_lshift( x, 4 ) + 8;
    }else{
      return_value = cal_lshift( x, 3 );
    }
  }
  return return_value;
}


// Convert MB y and block number to pixel y
int m_mc_add::ypixel( int y, int n ){
  int return_value;

  if ((n == 0) || (n == 1)){
    return_value = cal_lshift(y,4);
  }else{
    if ((n == 2) || (n == 3)){
      return_value = cal_lshift( y, 4 ) + 8;
    }else{
      return_value = cal_lshift( y, 3 );
    }
  }
  return return_value;
}

//output type derived by code analysis
cal_list<int>::t m_mc_add::comp_block( int x, int y, int n, int mvx, int mvy ){
  int lmvx = cal_rshift( mvx, 1 );
  int lmvy = cal_rshift( mvy, 1 );
  int maxx = cal_lshift(MBwidth , n < 4 ? 4 : 3);
  int maxy = cal_lshift(MBheight, n < 4 ? 4 : 3);
  int ulx = xpixel(x,n);
  int uly = ypixel(y,n);
  
  cal_list<int>::t return_value;
  
  cal_list<int>::t b;
  
  //not sure, that correct!! (order of loops!)
  for(int yy=0; yy <= 8; yy++){
    for(int xx=0; xx <= 8; xx++){
      b.push_back(get_pixel( x, y, n, 
			     xyclip( ulx, maxx, xx + lmvx ),
			     xyclip( uly, maxy, yy + lmvy ))
		  );
    }
  }
  
  
  if (cal_bitand( mvx, 1) == 0){
    if (cal_bitand( mvy, 1) == 0){
      // No interpolation

      //order of loops?
      for(int yy = 0; yy <= 7; yy++){
	for(int xx = 0; xx <= 7; xx++){
	  return_value.push_back(b[(yy * 9) + xx ]);
	} //for
      }// for
    }else{
      // Y interpolation only
      
      //order of loops?
      for(int yy = 0; yy <= 7; yy++){
	for(int xx = 0; xx <= 7; xx++){
	  return_value.push_back(cal_rshift( 
					    b[(yy*9) + xx] + b[(yy*9) + xx + 9 ] + 1 - rounding_type
					    , 1
					    ));
	}//for
      }//for
    }//if mvy
  }else{
    if (cal_bitand( mvy, 1) == 0){
      // X interpolation only
      
      //order of loops?
      for(int yy = 0; yy <= 7; yy++){
	for(int xx = 0; xx <= 7; xx++){
	  return_value.push_back(cal_rshift(
					    b[(yy*9)+xx] +  b[(yy*9) + xx + 1] + 1 - rounding_type
					    , 1
					    ));
	}//for
      }//for
    }else{
      // X and Y interpolation
      for(int yy = 0; yy <= 7; yy++){
	for(int xx = 0; xx <= 7; xx++){
	  return_value.push_back(cal_rshift( 
					    b[(yy*9) + xx] +
					    b[(yy*9) + xx +  1] + 
					    b[(yy*9) + xx +  9] + 
					    b[(yy*9) + xx + 10] + 2 - rounding_type
					    , 2
					    ));
	}//for
      }//for
    }//if mvy
  }//if mvx

  return return_value;
}

void m_mc_add::action_docmd_nothing(void){
  //input binding
  //int cmd  = MV[0];
  //int x    = MV[1];
  //int y    = MV[2];
  //int n    = MV[3];
  //int mvx  = MV[4];
  //int mvy  = MV[5];

#ifdef DEBUG_OUTPUT
  if ((frame_number == 190) && (y == 0) && (x < 2) && (n < 4)){
    cout << "No change at (" << next_x << "," << next_y << "," << next_n << ")" << endl;
  }
#endif //DEBUG_OUTPUT
  next_block();

  //output
  VID[0] = MBbuf_get(head_index);
}
/*
  bool m_mc_add::guard_docmd_nothing(void) const {
  return(detect_skipped( x, y, n ) == 0);
  }
*/

void m_mc_add::action_docmd_motionOnly(void){
  //input binding
  //int cmd  = MV[0];
  int x    = MV[1];
  int y    = MV[2];
  int n    = MV[3];
  int mvx  = MV[4];
  int mvy  = MV[5];

  //generate output
  VID[0] = comp_block(x, y, n, mvx, mvy);

#ifdef DEBUG_OUTPUT
  if (frame_number == 190) && (y == 0) && (x < 2) && (n < 4){
    cout << "MV( " << mvx << "," << mvy << ") only at (";
    cout << next_x << "," << next_y << "," << next_n << ")" << endl;
  }
#endif //DEBUG_OUTPUT

  next_block(); 

}
/*
  bool m_mc_add::guard_docmd_motionOnly(void) const{
  return(detect_skipped( x, y, n ) == 0);
  }
*/

int m_mc_add::clip( int x ){
  return(x < 0 ? 0 : 
	 x > 255 ? 255 : x);
}

//return type determined by code-analysis
//input types also
cal_list<int>::t m_mc_add::combine( cal_list<int>::t a, cal_list<int>::t b ){
  cal_list<int>::t return_value;

  for(int i=0; i <= 63; i++){
    return_value.push_back(clip(a[i] + b[i]));
  }

  return return_value;
}

void m_mc_add::action_docmd_both(void){
  //input binding
  //  int cmd  = MV[0];
  int x    = MV[1];
  int y    = MV[2];
  int n    = MV[3];
  int mvx  = MV[4];
  int mvy  = MV[5];

  const cal_list<int>::t &tex = TEX[0];

  //action
  cal_list<int>::t b = comp_block( x, y, n, mvx, mvy );

#ifdef DEBUG_OUTPUT
  if ((y == 0) && (x == 0) && (n == 3)){
    cout << "MV( " << mvx << "," << mvy;
    cout << ") and TEX at (" << next_x << "," << next_y << "," << next_n <<")" << endl;
    cout << tex;
  }
#endif //DEBUG_OUTPUT

  next_block();

  //generation of output

  VID[0] = combine( tex, b );
}
/*
  bool m_mc_add::guard_docmd_both(void) const{
  return(detect_skipped( x, y, n ) == 0);
  }
*/

// Previous frame queue needs to fill up
void m_mc_add::action_mbbuf_tooEmptyInt(void){
  //input bindung

  const cal_list<int>::t &b = LAST[0];

  MBbuf_add( b );
  int_buf_count = int_buf_count + 1;
}

/*
  bool m_mc_add::guard_mbbuf_tooEmptyInt(void) const{
  return(int_buf_count < int_buf_count_ss);
  }
*/

void m_mc_add::action_mbbuf_tooEmptyExt(void){
  ext_buf_count = ext_buf_count + 1;
}
/*
bool m_mc_add::guard_mbbuf_tooEmptyExt(void) const{
  return(
	 (int_buf_count == int_buf_count_ss) && 
	 (ext_buf_count < ext_buf_count_ss)
	 );
}
*/

void m_mc_add::action_mbbuf_tooFull(void){
  cout << "Unsupported action: mbbuf.tooFull in actor MCadd" << endl;
}
/*
bool m_mc_add::guard_mbbuf_tooFull(void) const{
  return(
	 (int_buf_count > int_buf_count_ss) ||
	 (ext_buf_count > ext_buf_count_ss)
	 );
}
*/

void m_mc_add::action_mbbuf_justRight(void){
  //input bindung
  const cal_list<int>::t &b = LAST[0];

  MBbuf_add( b );
}
/*
bool m_mc_add::guard_mbbuf_justRight(void) const{
  return(
	 (int_buf_count == int_buf_count_ss) &&
	 (ext_buf_count == ext_buf_count_ss)
	 );
}
*/

/* *************************************************************************
   Difficulties:
   - type analysis
   - order of loops
   - hierarchical states
   - and, or, comma in guards ?
   - guard not possible
   ************************************************************************* */


#endif // _INCLUDED_MCadd_HPP
