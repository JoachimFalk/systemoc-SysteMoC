#include "panchamround.hpp"

PanchamRound::PanchamRound()
{
}


PanchamRound::~PanchamRound()
{
}

sc_uint<32>
PanchamRound::perform(sc_uint<32> a, sc_uint<32> b, sc_uint<32> c, sc_uint<32> d,
                      // Note that for a 128-bit long input message, X[k] = M[k] = m :
                      sc_uint<32> m, sc_uint<32> s,
                      sc_uint<32> t,     // t-th sample of abs(sin(i)), i = 1, 2, ..., 64
                      sc_uint<2>  round) // round number (1-4).
{
  sc_uint<32> add_result;
  sc_uint<32> rotate_result1;
  sc_uint<32> rotate_result2;
   
  switch (round)
  {
  case ROUND1_MASK:
     add_result = a + F(b,c,d) + m + t;
     rotate_result1 = add_result << s;
     rotate_result2 = add_result >> (32-s);
     return b + (rotate_result1 | rotate_result2);
     break;
  case ROUND2_MASK:
     add_result = (a + G(b,c,d) + m + t);
     rotate_result1 = add_result << s;
     rotate_result2 = add_result >> (32-s);
     return b + (rotate_result1 | rotate_result2);
     break;
  case ROUND3_MASK:
     add_result = (a + H(b,c,d) + m + t);
     rotate_result1 = add_result << s;
     rotate_result2 = add_result >> (32-s);
     return b + (rotate_result1 | rotate_result2);
     break;
  case ROUND4_MASK:
     add_result = (a + I(b,c,d) + m + t);
     rotate_result1 = add_result << s;
     rotate_result2 = add_result >> (32-s);
     return b + (rotate_result1 | rotate_result2);
     break;
  };
}

//--------------------------------
//
// Function declarations
//
//--------------------------------
// Step 4 functions F, G, H and I
sc_uint<32>
PanchamRound::F(sc_uint<32> x, sc_uint<32> y, sc_uint<32> z)
{
   return (x&y)|((~x)&z);
}

sc_uint<32>
PanchamRound::G(sc_uint<32> x, sc_uint<32> y, sc_uint<32> z)
{
   return (x&z)|(y&(~z));
}

sc_uint<32>
PanchamRound::H(sc_uint<32> x, sc_uint<32> y, sc_uint<32> z)
{
   return (x^y^z);
}

sc_uint<32>
PanchamRound::I(sc_uint<32> x, sc_uint<32> y, sc_uint<32> z)
{
   return (y^(x|(~z)));
}
