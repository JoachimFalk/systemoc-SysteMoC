// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_DIRECTOR_HPP
#define _INCLUDED_HSCD_DIRECTOR_HPP

#include <systemc.h>

#include <memory>

class Resource {
public:
  virtual
  void compute( const char *name ) = 0;
};

class Director {
private:
  static std::auto_ptr<Director> singleton;
  
  Director() {
    std::cout << "foo" << std::endl;
  }
public:
  static Director& getInstance() {
//    if ( singleton == NULL )
//      singleton = new Director();
    return *singleton;
  }
  
  Resource& getResource( const char *name );
  
  virtual ~Director() {
    std::cout << "bar" << std::endl;
  }
};


#endif // _INCLUDED_HSCD_DIRECTOR_HPP
