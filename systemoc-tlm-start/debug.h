
// debug.h

/*
 * no guard, processed everytime and so can be included multiple times (to
 * define or undefine debug statements for some code section - debug_on.h and
 * debug_off.h will do this for you).
 *
 */

// processed multiple times. undef macros to avoid warnings.
#undef DBG
#undef DBG_OUT
#undef DBG_SC_OUT
#undef DBG_SCN_OUT
#undef DBG_DOBJ

// make DBG.*() statements disapear in non-debug builds
//#ifdef ENABLE_DEBUG
#ifdef alksdjflaksjfflsajfljasdf
  #define DBG(e) e
  #define DBG_OUT(s) std::cerr << "DBG: " << s
  #define DBG_SC_OUT(s) std::cerr << "DBG [" << sc_time_stamp() << "]: " << s
  #define DBG_SCN_OUT(s) std::cerr << "DBG [" << sc_time_stamp() << ", " \
                                  << name() << "]: " << s
  #define DBG_DOBJ(o) std::cerr << " Object " #o ": " << o << std::endl
#else
  #define DBG(e) do {} while(0)
  #define DBG_OUT(s) do {} while(0)
  #define DBG_SC_OUT(s) do {} while(0)
  #define DBG_SCN_OUT(s) do {} while(0)
  #define DBG_DOBJ(s) do {} while(0)
#endif

#undef ENABLE_DEBUG
