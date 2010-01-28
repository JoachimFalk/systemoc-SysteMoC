
#include <iostream>

#include <systemoc/smoc_moc.hpp>

//#define ENABLE_DEBUG
// make DBG.*() statements disapear in non-debug builds
#ifdef ENABLE_DEBUG
  #define DBG(e) e
  #define DBG_OUT(s) std::cerr << "DBG " << name() << ": " << s
  #define DBG_DOBJ(o) std::cerr << " Object " #o ": " << o << std::endl
#else
  #define DBG(e) do {} while(0)
  #define DBG_OUT(s) do {} while(0)
  #define DBG_DOBJ(s) do {} while(0)
#endif

// OPTION: move type declarations to file e.g. channels.hpp
enum PacketType{
  A,
  B,
  C,
  D
};

typedef char Data;

/******************************************************************************
 *
 *
 */
class Packet {
public:
  Packet(PacketType type) : packetType(type) {
    data[1] = '\0';
    switch(type){
    case A:
      data[0] = 'A';
      break;
    case B:
      data[0] = 'B';
      break;
    case C:
      data[0] = 'C';
      break;
    default:
    case D:
      data[0] = 'D';
      break;
    }
  }

  bool isType(const PacketType type) const {
    return this->packetType==type;
  }

  //
  friend std::ostream &operator<<(std::ostream &out,
                                  const Packet &p);
private:
  PacketType  packetType;
  Data        data[2];
};

// is required for smoc_fifo and is used for serialization to XML
inline std::ostream &operator<<(std::ostream &out, const Packet &p) {
  out << "Packet {type=" << p.packetType << ", data=" << p.data << "}";
  return out;
}





/******************************************************************************
 *
 *
 */
class PacketSource :
  public smoc_actor
{
public:
  smoc_port_out<Packet> out;

  PacketSource(sc_module_name name) :
    smoc_actor(name, s_a)
  {
    s_a =
      out(1) >>
      // use parameterized action: parameter "A"
      CALL(PacketSource::generatePacket)(A) >>
      s_b;

    s_b =
      out(1) >>
      CALL(PacketSource::generatePacket)(B) >>
      s_d;

    s_d =
      out(1) >>
      CALL(PacketSource::generatePacket)(D) >>
      s_end;
  }

private:
  smoc_firing_state s_a, s_b, s_d, s_end;

  // parameterized action 
  void generatePacket(PacketType type)
  {
    Packet packet(type);
    out[0] = packet;
    DBG_OUT("PacketSource: writing packet\n");
  }
};


/******************************************************************************
 *
 *
 */
template<typename DATATYPE>
class Forward :
  public smoc_actor
{
public:
  smoc_port_in<DATATYPE>  in;
  smoc_port_out<DATATYPE> out;

  Forward(sc_module_name name) :
    smoc_actor(name, s_main)
  {
    s_main =
      in(1)                        >>
      out(1)                       >>
      CALL(Forward::forwardPacket) >>
      s_main;
  }

private:
  smoc_firing_state s_main;

  void forwardPacket() {
    out[0] = in[0];
    DBG_OUT("forward " << in[0] << std::endl);
  }
};




/******************************************************************************
 *
 *
 */
class PacketDumper :
  public smoc_actor
{
public:
  smoc_port_in<Packet> inPacket;

  PacketDumper(sc_module_name name) :
    smoc_actor(name, s_main)
  {
    s_main =
        (inPacket(1)) >>
        CALL(PacketDumper::dumpPacket) >>
        s_main;
  }

private:
  smoc_firing_state s_main;

  // dump packet to stdout
  void dumpPacket() {
    std::cout << this->name() << " recv: " << inPacket[0] << std::endl;
  }
};



/******************************************************************************
 *
 *
 */
class Dispatcher :
  public smoc_actor
{
public:
  smoc_port_in<Packet>  in;
  smoc_port_out<Packet> outLeft;
  smoc_port_out<Packet> outRight;

  Dispatcher(sc_module_name name) :
    smoc_actor(name, s_main)
  {
    s_main =
      (in(1)                              &&
       GUARD(Dispatcher::isPacketAorC)    &&
       !GUARD(Dispatcher::isPacketBorD) ) >>
      outLeft(1)                          >>
      CALL(Dispatcher::forwardLeft)       >> s_main
    |
      (in(1)                              &&
       !GUARD(Dispatcher::isPacketAorC)   &&
       GUARD(Dispatcher::isPacketBorD))   >>
      outRight(1)                         >>
      CALL(Dispatcher::forwardRight)      >> s_main
    ;
  }

private:
  smoc_firing_state s_main;

  bool isPacketAorC() const {
    return in[0].isType(A) || in[0].isType(C);
  }

  bool isPacketBorD() const {
    return in[0].isType(B) || in[0].isType(D);
  }

  void forwardLeft() {
    outLeft[0] = in[0];
  }

  void forwardRight() {
    outRight[0] = in[0];
  }
};








 
/******************************************************************************
 *
 *
 */
class HierarchyDemo :
  public smoc_graph
{
public:
  smoc_port_in<Packet>  in;
  smoc_port_out<Packet> outLeft;
  smoc_port_out<Packet> outRight;

  HierarchyDemo(sc_module_name name) :
    smoc_graph(name),
    dispatcher("dispatcher"),
    left("left"),
    right("right")
  {

    //bind actor ports to graph ports
    dispatcher.in(in);
    left.out(outLeft);
    right.out(outRight);
    
    // connect fifos:
    connectNodePorts(dispatcher.outLeft, left.in);
    connectNodePorts(dispatcher.outRight, right.in);
  }

private:
  Dispatcher      dispatcher;
  Forward<Packet> left;
  Forward<Packet> right;
};







 
/******************************************************************************
 *
 *
 */
class TopGraph :
  public smoc_graph
{
public:
  TopGraph(sc_module_name name) :
    smoc_graph(name),
    src("src"),
    hierarchy("hierarchy"),
    snk0("snk0"),
    snk1("snk1")
  {
    connectNodePorts(src.out,           hierarchy.in);
    connectNodePorts(hierarchy.outLeft,  snk0.inPacket);
    connectNodePorts(hierarchy.outRight, snk1.inPacket);
  }

private:
  PacketSource  src;
  HierarchyDemo hierarchy;
  PacketDumper  snk0;
  PacketDumper  snk1;
};



/******************************************************************************
 *
 *
 */
int sc_main (int argc, char **argv)
{
  TopGraph top("top");
  smoc_scheduler_top sched(top);

  sc_start();
  return 0;
}
