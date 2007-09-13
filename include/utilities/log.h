#ifndef __LOG_H__
#define __LOG_H__

#if LOGGING

#define RLOG_COMPONENT dionysus

#include <rlog/rlog.h>
#include <rlog/RLogChannel.h>
#include <rlog/StdioNode.h>
#include <sstream>

template<class T>
std::string tostring(const T& t) { std::ostringstream out; out << t; return out.str(); }

#else // LOGGING

#define rDebug(...)
#define rInfo(...)
#define rWarning(...)
#define rError(...)
#define rLog(...)

#define rAssert(...)
#define rAssertSilent(...)

#define DEF_CHANNEL(...) 0
#define RLOG_CHANNEL(...) 0

#endif // LOGGING

#endif //__LOG_H__
