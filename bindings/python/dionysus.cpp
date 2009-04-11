#include <utilities/log.h>
#include <boost/python.hpp>

namespace bp = boost::python;

void export_simplex();
void export_filtration();
void export_static_persistence();
void export_chain();
void export_zigzag_persistence();

void export_rips();

#ifdef LOGGING
void            enable_log(std::string s)
{
    stdoutLog.subscribeTo(RLOG_CHANNEL(s.c_str()));
}
#endif

BOOST_PYTHON_MODULE(_dionysus)
{
    export_simplex();
    export_filtration();
    export_static_persistence();
    export_chain();

    export_zigzag_persistence();

    export_rips();

#ifdef LOGGING
    bp::def("enable_log",           &enable_log);
#endif
};
