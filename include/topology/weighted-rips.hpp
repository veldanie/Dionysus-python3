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
static rlog::RLogChannel* rlRips =                  DEF_CHANNEL("rips/info", rlog::Log_Debug);  /* TODO: change rips/ to weighted-rips/ ? */
static rlog::RLogChannel* rlRipsDebug =             DEF_CHANNEL("rips/debug", rlog::Log_Debug); /* TODO: same here */
#endif // LOGGING

#ifdef COUNTERS
static Counter*  cClique =                          GetCounter("rips/clique");                  /* TODO: same here */
#endif // COUNTERS
