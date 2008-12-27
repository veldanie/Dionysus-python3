#include <topology/static-persistence.h>
#include "python-filtration.h"

#include <boost/python.hpp>
using namespace boost::python;

#include "python-static-persistence.h"

boost::shared_ptr<SPersistence>     init_from_filtration(object f)
{
    ListFiltration& lf = extract<ListFiltration&>(f);
    boost::shared_ptr<SPersistence> p(new SPersistence(lf));
    return p;
}

void                                pair_simplices(SPersistence& p)
{
    p.pair_simplices(); 
}

void export_static_persistence()
{
    class_<SPersistenceNode>("StaticPersistenceNode")
        .def_readonly("pair",   &SPersistenceNode::pair)
        .def("sign",            &SPersistenceNode::sign)
        .def_readonly("cycle",  &SPersistenceNode::cycle)
    ;

    class_<SPersistence>("StaticPersistence", no_init)
        .def("__init__",        make_constructor(&init_from_filtration))

        .def("pair_simplices",  &pair_simplices)

        .def("__iter__",        range(&SPersistence::begin, &SPersistence::end))
        .def("__len__",         &SPersistence::size)
    ;
}
