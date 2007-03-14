
// debug.h

/*
 * no guard. can be included multiple times to define or undefine debug
 * statements for some code section - debug_on.h and debug_off.h will do this
 * for you so include them instead of this file.
 *
 */

// processed multiple times. undef macros to avoid warnings.
#undef DBG
#undef DBG_OUT
#undef DBG_SC_OUT
#undef DBG_DOBJ

// make sure this stream exists
#ifndef DBG_STREAM
  #define DBG_STREAM dbgout
#endif

// make DBG.*() statements disapear in non-debug builds
#ifdef ENABLE_DEBUG
  #define DBG(e) e
  #define DBG_OUT(s) DBG_STREAM <<  s
  #define DBG_SC_OUT(s) DBG_STREAM << "[" << sc_time_stamp() << "]: " << s
  #define DBG_DOBJ(o) DBG_STREAM << " Object " #o ": " << o << std::endl
#else
  #define DBG(e) do {} while(0)
  #define DBG_OUT(s) do {} while(0)
  #define DBG_SC_OUT(s) do {} while(0)
  #define DBG_DOBJ(s) do {} while(0)
#endif

#undef ENABLE_DEBUG
