#include <string>
#include <iostream>
#include <memory>

#include <cosupport/oneof.hpp>

class a_ty {
  int x;
};

class b_ty {
  double x;
};

class c_ty {
  float x;
  long long y;
};

typedef CoSupport::oneof<a_ty, b_ty, c_ty> abc_ty;

int main(int argc, char* argv[]) {

  a_ty a = a_ty();
  b_ty b = b_ty();
  c_ty c = c_ty();

  abc_ty abc = abc_ty();

  // write to oneof
  abc = a;
  static_cast<const a_ty &>(abc);

  // read from oneof
  a = abc;

  // read wrong type -> assertion
  //c = abc;
  
  
  return 0;
}

