#include <boost/python.hpp>
#include <boost/python/iterator.hpp>

namespace bp = boost::python;

#include "chain.h"


void export_chain()
{
    bp::class_<VChain>("Chain")
        .def("__iter__",    bp::iterator<VChain>())
        .def("__len__",     &VChain::size)
    ;
}
