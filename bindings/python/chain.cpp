#include <boost/python.hpp>
#include <boost/python/iterator.hpp>
namespace bp = boost::python;

#include "chain.h"
namespace dp = dionysus::python;


void export_chain()
{
    bp::class_<dp::VChain>("Chain")
        .def("__iter__",    bp::iterator<dp::VChain>())
        .def("__len__",     &dp::VChain::size)
    ;
}
