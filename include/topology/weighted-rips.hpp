#include <algorithm>
#include <utility>
#include <boost/utility.hpp>
#include <iostream>
#include <utilities/log.h>
#include <utilities/counter.h>
#include <utilities/indirect.h>
#include <boost/iterator/counting_iterator.hpp>
#include <functional>

#ifdef LOGGING
static rlog::RLogChannel* rlWeightedRips =                  DEF_CHANNEL("weightedrips/info", rlog::Log_Debug);
static rlog::RLogChannel* rlWeightedRipsDebug =             DEF_CHANNEL("weightedrips/debug", rlog::Log_Debug);
#endif // LOGGING

#ifdef COUNTERS
static Counter*  cClique =                          GetCounter("weightedrips/clique");
#endif // COUNTERS
