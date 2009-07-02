#ifndef __PYTHON_UTILS_H__
#define __PYTHON_UTILS_H__

#include <boost/python.hpp>
namespace bp = boost::python;

namespace dionysus {
namespace python   {

struct PythonCmp
{
    template<class T>
    bool            operator()(T x1, T x2) const            { return cmp_(x1, x2) < 0; }

                    PythonCmp(bp::object cmp): cmp_(cmp)    {}

    bp::object cmp_;
};

} } // namespace dionysus::python

#endif
