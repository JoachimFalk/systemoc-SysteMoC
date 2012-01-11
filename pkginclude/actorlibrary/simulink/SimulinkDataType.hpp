



#ifndef __INCLUDED__SIMULINKDATATYPE__HPP__
#define __INCLUDED__SIMULINKDATATYPE__HPP__

typedef unsigned char  uchar_T;
typedef unsigned short ushort_T;
typedef unsigned long  ulong_T; 

/*=======================================================================*
 * Fixed width word size data types:                                     *
 *   int8_T, int16_T, int32_T     - signed 8, 16, or 32 bit integers     *
 *   uint8_T, uint16_T, uint32_T  - unsigned 8, 16, or 32 bit integers   *
 *   real32_T, real64_T           - 32 and 64 bit floating point numbers *
 *=======================================================================*/
typedef char  int8_T;
typedef short int16_T;
typedef int   int32_T;

/* Unsigned 8-bit integer */
typedef unsigned char  uint8_T;
/* Unsigned 16-bit integer */
typedef unsigned short uint16_T;
/* Unsigned 32-bit integer */
typedef unsigned int   uint32_T;
/* single : single-precision floating point*/
//typedef float  real32_T;
//typedef int  real32_T;

/* double : double-precision floating point*/
typedef double real64_T;
/* 
   C++ support data types:
   
   Tpye Name            Bytes
   
   int, __int32         4
   long                 4
   __int8, char         1
   unsigned char        1
   __int16, short       2
   __int64,long long    8
   bool                 1
   float                4             3.4E +/- 38(7 digits)
   double, long double  8             1.7E +/- 308(15 digits) 
  */

#endif // __SIMULINKDATATYPE__