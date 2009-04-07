#include <systemoc/smoc_config.h>

#include <systemoc/smoc_ngx_sync.hpp>

using namespace SystemCoDesigner::SGX;

namespace SysteMoC { namespace NGXSync {
  
  std::ostream& operator<<(std::ostream& out, const IdAttr& id)
  { return out << "id" << id.id; }
  
  AlreadyInitialized::AlreadyInitialized() :
    std::runtime_error("NGXConfig was already initialized")
  {}
  
  NgId IdPool::regObj(SCObj* obj, size_t index) { 
    if(smoc_modes::dumpFileSMX)
      return CoSupport::SMXIdManager::getInstance().addObj(obj, index);
    else
      return -1;
  }
  
  NgId IdPool::regObj(SCObj* obj, const NgId& id, size_t index) {
    if(smoc_modes::dumpFileSMX)
      return CoSupport::SMXIdManager::getInstance().addObj(obj, index);
    else
      return -1;
  }
  
  void IdPool::unregObj(const SCObj* obj)
  { CoSupport::SMXIdManager::getInstance().delObj(obj); }

  NgId IdPool::getId(const SCObj* obj, size_t index) const
  { return CoSupport::SMXIdManager::getInstance().getId(obj, index); }
  
  NgId IdPool::getId()
  { return CoSupport::SMXIdManager::getInstance().addAnon(); }

  IdAttr IdPool::printId(const SCObj* obj, size_t index) const
  { return IdAttr(getId(obj, index)); }
  
  IdAttr IdPool::printId()
  { return IdAttr(getId()); }

  IdAttr IdPool::printIdInvalid() const
  { return IdAttr(~static_cast<CoSupport::SMXId>(0)); }
  
  SCObj* IdPool::getObj(const NgId& id) const
  { return CoSupport::SMXIdManager::getInstance().getObj(id); }

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
  
  void NGXConfig::loadNGX(std::istream& ngx) {
    if(_ngx)
      throw AlreadyInitialized();
    _ngx = new NetworkGraphAccess(ngx);
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

  SGX::IdedObj::ConstPtr NGXCache::get(SCObj* obj, size_t index) {
    // lookup / create IndexMap
    SC2NGX::iterator i = sc2ngx.find(obj);
    if(i == sc2ngx.end()) i = CoSupport::DataTypes::pac_insert(sc2ngx, obj);

    // cache lookup
    IndexMap::const_iterator j = i->second.find(index);
    if(j != i->second.end()) return j->second;

    // idPool lookup (SGX is read only, so cache always)
    NgId id = idPool.getId(obj, index);
    SGX::IdedObj::ConstPtr iop = NGXConfig::getInstance().getNGX().objById(id);
    CoSupport::DataTypes::pac_insert(i->second, index, iop);
    
    return iop;
  }

  SCObj* NGXCache::get(SGX::IdedObj::ConstPtr iop) {
    // cache lookup
    NGX2SC::const_iterator i = ngx2sc.find(iop);
    if(i != ngx2sc.end()) return i->second;

    // idPool lookup (cache only if defined, may not
    // be registered yet)
    SCObj* obj = idPool.getObj(iop->id());
    if(obj) CoSupport::DataTypes::pac_insert(ngx2sc, iop, obj);

    return obj;
  }
  
  SCObj* NGXCache::get(SGX::IdedObj::ConstRef iop)
  { return get(&iop); }

  smoc_sysc_port* NGXCache::getCompiledPort(Port::ConstPtr port) {
    // cache lookup (if object exists it should be a port)
    SCObj* obj = get(*port);
    if(obj) {
      smoc_sysc_port* rp = dynamic_cast<smoc_sysc_port*>(obj);
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
        smoc_sysc_port* rp = dynamic_cast<smoc_sysc_port*>(obj);
        assert(rp);

        // sync. with idPool (FIXME: index?)
        idPool.regObj(rp, port->id(), 99);

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
        smoc_sysc_port* rp = dynamic_cast<smoc_sysc_port*>(obj);
        assert(rp);

        // sync. with idPool (FIXME: index?)
        idPool.regObj(rp, port->id(), 99);

        return rp;
      }

      test = test->innerConnectedPort();
    }

    return 0;
  }

} } // SysteMoC::NGXSync
