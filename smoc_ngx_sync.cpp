#include <systemoc/smoc_ngx_sync.hpp>

using namespace SysteMoC::NGX;

namespace SysteMoC { namespace NGXSync {
  
  IdAttr::IdAttr(const NGX::NgId& id) :
    id(id)
  {}

  std::ostream& operator<<(std::ostream& out, const IdAttr& id)
  { return out << "id" << id.id; }
  
  AlreadyInitialized::AlreadyInitialized() :
    std::runtime_error("NGXConfig was already initialized")
  {}
  
  IdPool::IdPool() :
    unnamedIds(UNNAMED)
  {}

  NgId IdPool::regObj(sc_core::sc_object* obj, size_t index) {
    // determine free id
    Id id = hash(obj->name());
    while(!namedIds.insert(id).second) {
#ifdef SYSTEMOC_DEBUG
      std::cerr << "hash collision: " << id << std::endl;
#endif
      id++;
    }
    return regObj(obj, NgId(NAMED + id), index);
  }
  
  NgId IdPool::regObj(
      sc_core::sc_object* obj, const NgId& id, size_t index)
  {
    ObjToId::iterator i = idByObj(obj);
    if(i == objToId.end())
      i = CoSupport::pac_insert(objToId, obj);
    // insert object into maps
    CoSupport::pac_insert(i->second, index, id);
    CoSupport::pac_insert(idToObj, id, obj);
#ifdef SYSTEMOC_DEBUG
    std::cerr << "registered " << obj->name() << ": " << id << std::endl;
#endif
    return id;
  }
  
  void IdPool::unregObj(const sc_core::sc_object* obj) {
    ObjToId::iterator i = idByObj(obj);
    if(i == objToId.end())
      return;
    // delete references from maps
    for(IndexMap::iterator idx = i->second.begin();
        idx != i->second.end();
        ++idx)
    {
      idToObj.erase(objById(idx->second));
      // FIXME: remove id from namedIds?
    }
    objToId.erase(i);
#ifdef SYSTEMOC_DEBUG
    std::cerr << "unregistered " << obj->name() << std::endl;
#endif
  }
  
  NgId IdPool::getId(const sc_core::sc_object* obj, size_t index) const {
    ObjToId::const_iterator i = idByObj(obj);
    if(i == objToId.end()) {
      std::cerr << "'" << obj->name() << "' not registered!"
                << std::endl;
      assert(0);
    }
    IndexMap::const_iterator idx = i->second.find(index);
    if(idx == i->second.end()) {
      std::cerr << "'" << obj->name() << "' has no id for index "
                << index << std::endl;
      assert(0);
    }
    return idx->second;
  }
  
  NgId IdPool::getId() {
    assert(unnamedIds < RESERVED);
    return NgId(unnamedIds++);
  }

  IdAttr IdPool::printId(const sc_core::sc_object* obj, size_t index) const
  { return IdAttr(getId(obj, index)); }
  
  IdAttr IdPool::printId()
  { return IdAttr(getId()); }
  
  sc_core::sc_object* IdPool::getObj(const NgId& id) const {
    IdToObj::const_iterator i = objById(id);
    return i == idToObj.end() ? 0 : i->second;
  }

  void IdPool::dump(std::ostream& out) const {
    out << "objects (named):" << std::endl;
    for(IdToObj::const_iterator i = idToObj.begin();
        i != idToObj.end();
        ++i)
    {
      out << "  " << i->first << ": " << i->second->name()
          << std::endl;
    }
    out << "objects (unnamed):" << std::endl;
    if(unnamedIds > UNNAMED) {
      out << "  " << UNNAMED << " - " << (unnamedIds - 1)
          << std::endl;
    }
  }

  IdPool::ObjToId::iterator IdPool::idByObj(const sc_core::sc_object* obj)
  { return objToId.find(const_cast<sc_core::sc_object*>(obj)); }
  
  IdPool::ObjToId::const_iterator IdPool::idByObj(const sc_core::sc_object* obj) const
  { return objToId.find(const_cast<sc_core::sc_object*>(obj)); }

  IdPool::IdToObj::iterator IdPool::objById(const NgId& id)
  { return idToObj.find(id); }
  
  IdPool::IdToObj::const_iterator IdPool::objById(const NgId& id) const
  { return idToObj.find(id); }

  IdPool idPool;
  
  NGXConfig::NGXConfig() :
    _ngx(0)
  {}
  
  NGXConfig& NGXConfig::getInstance() {
    static NGXConfig instance;
    return instance;
  }

  void NGXConfig::loadNGX(const std::string& ngx) {
    if(_ngx)
      throw AlreadyInitialized();
    _ngx = new NetworkGraphAccess(ngx.c_str());
  }

  bool NGXConfig::hasNGX() const
  { return _ngx; }

  const NetworkGraphAccess& NGXConfig::getNGX() const
  { assert(_ngx); return *_ngx; }

  NGXConfig::~NGXConfig()
  { if(_ngx) delete _ngx; _ngx = 0; }
  
  NGXCache::NGXCache()
  {}
  
  NGXCache& NGXCache::getInstance() {
    static NGXCache instance;
    return instance;
  }

  NGX::IdedObj::ConstPtr NGXCache::get(sc_core::sc_object* obj, size_t index) {
    // lookup / create IndexMap
    SC2NGX::iterator i = sc2ngx.find(obj);
    if(i == sc2ngx.end()) i = CoSupport::pac_insert(sc2ngx, obj);

    // cache lookup
    IndexMap::const_iterator j = i->second.find(index);
    if(j != i->second.end()) return j->second;

    // idPool lookup (NGX is read only, so cache always)
    NgId id = idPool.getId(obj, index);
    NGX::IdedObj::ConstPtr iop = NGXConfig::getInstance().getNGX().objById(id);
    CoSupport::pac_insert(i->second, index, iop);
    
    return iop;
  }

  sc_core::sc_object* NGXCache::get(NGX::IdedObj::ConstPtr iop) {
    // cache lookup
    NGX2SC::const_iterator i = ngx2sc.find(iop);
    if(i != ngx2sc.end()) return i->second;

    // idPool lookup (cache only if defined, may not
    // be registered yet)
    sc_core::sc_object* obj = idPool.getObj(iop->id());
    if(obj) CoSupport::pac_insert(ngx2sc, iop, obj);

    return obj;
  }
  
  sc_core::sc_object* NGXCache::get(NGX::IdedObj::ConstRef iop)
  { return get(&iop); }

  smoc_port_ast_iface* NGXCache::getCompiledPort(Port::ConstPtr port) {
    // cache lookup (if object exists it should be a port)
    sc_core::sc_object* obj = get(*port);
    if(obj) {
      smoc_port_ast_iface* rp = dynamic_cast<smoc_port_ast_iface*>(obj);
      assert(rp);
      return rp;
    }

    // search for port with same interface -> outwards
    Port::ConstPtr test = port->outerConnectedPort();
    while(test) {
      // only probe ports with same direction
      if(test->direction().get() != port->direction().get())
        break;

      // cache lookup (if object exists it should be a port)
      obj = get(*test);
      if(obj) {
        smoc_port_ast_iface *rp = dynamic_cast<smoc_port_ast_iface *>(obj);
        assert(rp);

        // sync. with idPool (FIXME: index?)
        idPool.regObj(dynamic_cast<smoc_port_sysc_iface *>(obj), port->id(), 99);

        return rp;
      }

      test = test->outerConnectedPort();
    }

    // search for port with same interface -> inwards
    test = port->innerConnectedPort();
    while(test) {
      // only probe ports with same direction
      if(test->direction().get() != port->direction().get())
        break;

      // cache lookup (if object exists it should be a port)
      obj = get(*test);
      if(obj) {
        smoc_port_ast_iface *rp = dynamic_cast<smoc_port_ast_iface *>(obj);
        assert(rp);

        // sync. with idPool (FIXME: index?)
        idPool.regObj(dynamic_cast<smoc_port_sysc_iface *>(obj), port->id(), 99);

        return rp;
      }

      test = test->innerConnectedPort();
    }

    return 0;
  }

} } // SysteMoC::NGXSync
