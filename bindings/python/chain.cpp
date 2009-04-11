#include <topology/chain.h>
#include "python-static-persistence.h"
#include <boost/python.hpp>

using namespace boost::python;

typedef     SPersistence::Chain                                 VChain;


VChain::const_iterator      begin(const VChain& c)              { return c.begin(); }
VChain::const_iterator      end(const VChain& c)                { return c.end(); }

void export_chain()
{
    class_<VChain>("Chain")
        .def("__iter__",    range(&begin, &end))
    ;
}
