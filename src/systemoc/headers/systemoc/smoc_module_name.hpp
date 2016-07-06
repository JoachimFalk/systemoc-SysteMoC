#ifndef INCLUDED_SYSTEMOC_SMOC_MODULE_NAME_HPP
#define INCLUDED_SYSTEMOC_SMOC_MODULE_NAME_HPP

#include <systemc>
#include <iostream>

class smoc_module_name;

class smoc_object_manager {
public:
  static smoc_object_manager& getInstance()
  {
    static smoc_object_manager m;
    return m;
  }

  void pushName(smoc_module_name* n) {
    nameStack.push_back(n);
  }   

  smoc_module_name* popName() {
    if(nameStack.empty())
      return 0;
    smoc_module_name* top = nameStack.back();
    nameStack.pop_back();
    return top;
  }

private:
  std::list<smoc_module_name*> nameStack;

  smoc_object_manager() {}

  smoc_object_manager(const smoc_object_manager&);
  smoc_object_manager& operator=(const smoc_object_manager&);
};

class smoc_module_name : public sc_module_name {
public:
  smoc_module_name(const char* name)
    : sc_module_name(name),
      pushed(true)
  {
    std::cerr << "smoc_module_name::smoc_module_name(const char*)" << std::endl;
    smoc_object_manager::getInstance().pushName(this);
  }

  smoc_module_name(const smoc_module_name& name)
    : sc_module_name(name),
      pushed(false)
  {}

  ~smoc_module_name()
  {
    if(pushed) {
      std::cerr << "smoc_module_name::~smoc_module_name()" << std::endl;
      smoc_module_name* n =
        smoc_object_manager::getInstance().popName();
      assert(n == this);
      // notify registered objects
    }
  }

private:
  bool pushed;
};

#endif // INCLUDED_SYSTEMOC_SMOC_MODULE_NAME_HPP
