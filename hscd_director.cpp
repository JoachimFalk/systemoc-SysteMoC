// vim: set sw=2 ts=8:

#include <hscd_director.hpp>

std::auto_ptr<Director> Director::singleton(new Director());

class Component
: public Resource {
public:
  virtual
  void compute( const char *name ) {
    std::cout << "PG node " << name << " start execution " << sc_simulation_time() << std::endl;
    wait( 10, SC_NS);
    std::cout << "PG node " << name << " stop execution " << sc_simulation_time() << std::endl;
  }
} myComponent;

Resource& Director::getResource( const char *name ) {
  return myComponent;
}



