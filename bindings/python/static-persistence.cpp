#include <topology/static-persistence.h>

#include <boost/python.hpp>
#include <boost/python/iterator.hpp>
#include <boost/python/return_internal_reference.hpp>
using namespace boost::python;

#include "filtration.h"
#include "static-persistence.h"
#include "chain.h"


/* SPersistence */
boost::shared_ptr<SPersistence>     init_from_filtration(object f)
{
    ListFiltration& lf = extract<ListFiltration&>(f);
    boost::shared_ptr<SPersistence> p(new SPersistence(lf));
    return p;
}

boost::counting_iterator<SPersistenceIndex>     py_sp_begin(SPersistence& sp)                   { return sp.begin(); }
boost::counting_iterator<SPersistenceIndex>     py_sp_end(SPersistence& sp)                     { return sp.end(); }
unsigned                                        distance(SPersistence& sp, 
                                                         const SPersistenceIndex& i)            { return i - sp.begin(); }


/* SPersistenceIndex */
SPersistenceIndex                               pair(const SPersistenceIndex& i)                { return i->pair; }
bool                                            sign(const SPersistenceIndex& i)                { return i->sign(); }
const VChain&                                   cycle(const SPersistenceIndex& i)               { return i->cycle; }
bool                                            index_eq(const SPersistenceIndex& i, 
                                                         const SPersistenceIndex& j)            { return i == j; }


void export_static_persistence()
{
    class_<SPersistenceIndex>("SPNode")
        .add_property("pair",   &pair)
        .def("sign",            &sign)
        .def("cycle",           &cycle,         bp::return_internal_reference<1>())
        .def("__eq__",          index_eq)
    ;

    class_<SPersistence>("StaticPersistence", no_init)
        .def("__init__",        make_constructor(&init_from_filtration))
        
        .def("pair_simplices",  (void (SPersistence::*)())  &SPersistence::pair_simplices)
        .def("__call__",        &distance)

        .def("__iter__",        range(&py_sp_begin, &py_sp_end))
        .def("__len__",         &SPersistence::size)
    ;
}
