#include <topology/rips.h>
#include <boost/python.hpp>
#include "python-rips.h"            // defines RipsWithDistances

#include <iostream>

namespace bp = boost::python;

/* Various wrappers for exposing Rips to Python */
// Constructor from distances
boost::shared_ptr<RipsWithDistances>        init_from_distances(bp::object distances)
{ 
    boost::shared_ptr<RipsWithDistances>    p(new RipsWithDistances(distances));
    return p;
}

void export_rips()
{
    bp::class_<RipsWithDistances>("Rips", bp::no_init)
        .def("__init__",            make_constructor(&init_from_distances))
        .def("generate",            &RipsWithDistances::generate)
        .def("generate",            &RipsWithDistances::generate_candidates)
        .def("vertex_cofaces",      &RipsWithDistances::vertex_cofaces)
        .def("vertex_cofaces",      &RipsWithDistances::vertex_cofaces_candidate)
        .def("edge_cofaces",        &RipsWithDistances::edge_cofaces)
        .def("edge_cofaces",        &RipsWithDistances::edge_cofaces_candidates)

        .def("cmp",                 &RipsWithDistances::cmp)
        .def("eval",                &RipsWithDistances::eval)
    ;
}
