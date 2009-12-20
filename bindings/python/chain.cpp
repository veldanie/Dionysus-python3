#include <boost/iterator/indirect_iterator.hpp>

#include <boost/python.hpp>
#include <boost/python/iterator.hpp>
#include <boost/python/return_internal_reference.hpp>
namespace bp = boost::python;

#include "chain.h"
namespace dp = dionysus::python;


boost::indirect_iterator<dp::VChain::const_iterator>    chain_begin(dp::VChain& c)                  { return boost::make_indirect_iterator(c.begin()); }
boost::indirect_iterator<dp::VChain::const_iterator>    chain_end(dp::VChain& c)                    { return boost::make_indirect_iterator(c.end()); }

void export_chain()
{
    bp::class_<dp::VChain>("Chain")
        .def("__iter__",    bp::range<bp::return_internal_reference<1> >(&chain_begin, &chain_end))
        .def("__len__",     &dp::VChain::size)
    ;
}
