
#include "channels.hpp"

#include <cassert>

//
std::ostream &operator << (std::ostream &out, const codeword_t val) {
  out << (unsigned int) val;
  return out;
}

#ifndef NDEBUG
std::ostream &operator << (std::ostream &out, const JpegChannel_t &x) {
  out << "[JpegChannel_t:";
  if (!JS_ISCTRL(x)) {
    out << "raw(" << JS_DATA_GET(x) << "),"
        << "tup(" << JS_TUP_GETIDCTAMPLCOEFF(x) << ","
                  << JS_TUP_GETRUNLENGTH(x) << ","
                  << JS_TUP_GETCATEGORY(x) << "),"
        << "qco(" << JS_QCOEFF_GETIDCTCOEFF(x) << "),"
        << "ico(" << JS_COEFF_GETIDCTCOEFF(x) << "),"
        << "com(" << JS_COMPONENT_GETVAL(x) << ")";
  } else {
    switch (static_cast<CtrlCmd_t>(JS_GETCTRLCMD(x))) {
      case CTRLCMD_USEHUFF:
        out << "CTRLCMD_USEHUFF("
            << JS_CTRL_USEHUFF_GETCOMP(x) << ","
            << JS_CTRL_USEHUFF_GETDCTBL(x) << ","
            << JS_CTRL_USEHUFF_GETACTBL(x) << ")";
        break;
      case CTRLCMD_DISCARDHUFF:
        out << "CTRLCMD_DISCARDHUFF("
            << JS_CTRL_DISCARDHUFFTBL_GETHUFFID(x) << ","
            << JS_CTRL_DISCARDHUFFTBL_GETTYPE(x) << ")";
        break;
      case CTRLCMD_NEWSCAN:
        out << "CTRLCMD_NEWSCAN("
            << JS_CTRL_NEWSCAN_GETCOMP(x,0)
            << JS_CTRL_NEWSCAN_GETCOMP(x,1)
            << JS_CTRL_NEWSCAN_GETCOMP(x,2)
            << JS_CTRL_NEWSCAN_GETCOMP(x,3)
            << JS_CTRL_NEWSCAN_GETCOMP(x,4)
            << JS_CTRL_NEWSCAN_GETCOMP(x,5) << ")";
        break;
      case CTRLCMD_SCANRESTART:
        out << "CTRLCMD_SCANRESTART";
        break;
      case CTRLCMD_USEQT:
        out << "CTRLCMD_USEQT("
            << JS_CTRL_USEQT_GETQTID(x) << ","
            << JS_CTRL_USEQT_GETCOMPID(x) << ")";
        break;
      case CTRLCMD_DISCARDQT:
        out << "CTRLCMD_DISCARDQT("
            << JS_CTRL_DISCARDQT_GETQTID(x) << ")";
        break;
      case CTRLCMD_INTERNALCOMPSTART:
        out << "CTRLCMD_INTERNALCOMPSTART("
            << JS_CTRL_INTERNALCOMPSTART_GETCOMPID(x) << ")";
        break;
      case CTRLCMD_DEF_RESTART_INTERVAL:
        out << "CTRLCMD_DEF_RESTART_INTERVAL("
            << JS_CTRL_RESTART_GET_INTERVAL(x) << ")";
        break;
      case CTRLCMD_NEWFRAME:
        out << "CTRLCMD_NEWFRAME("
            << JS_CTRL_NEWFRAME_GET_DIMX(x) << ","
            << JS_CTRL_NEWFRAME_GET_DIMY(x) << ","
            << JS_CTRL_NEWFRAME_GET_COMPCOUNT(x) << ")";
        break;
      default:
        assert(!"Unhandle control command in JpegChannel_t");
    }
  }
  out << "]";
  return out;
}
#endif
