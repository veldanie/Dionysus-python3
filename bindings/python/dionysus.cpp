#include <utilities/log.h>
#include <boost/python.hpp>

namespace bp = boost::python;

void export_simplex();
void export_filtration();
void export_static_persistence();
void export_chain();
void export_birthid();
void export_zigzag_persistence();
void export_cohomology_persistence();

void export_rips();
#ifndef NO_CGAL
void export_alphashapes2d();
void export_alphashapes3d();
#endif

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

    export_birthid();
    export_zigzag_persistence();
    export_cohomology_persistence();

    export_rips();
#ifndef NO_CGAL
    export_alphashapes2d();
    export_alphashapes3d();
#endif

#ifdef LOGGING
    bp::def("enable_log",           &enable_log);
#endif
};
