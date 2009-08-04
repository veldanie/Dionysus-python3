#include <topology/static-persistence.h>

#include <boost/python.hpp>
#include <boost/python/iterator.hpp>
#include <boost/python/return_internal_reference.hpp>
namespace bp = boost::python;

#include "filtration.h"
#include "static-persistence.h"
#include "chain.h"
namespace dp = dionysus::python;


/* SPersistence */
boost::shared_ptr<dp::SPersistence>     init_from_filtration(bp::object f)
{
    dp::ListFiltration& lf = bp::extract<dp::ListFiltration&>(f);
    boost::shared_ptr<dp::SPersistence> p(new dp::SPersistence(lf));
    return p;
}

boost::counting_iterator<dp::SPersistenceIndex>     py_sp_begin(dp::SPersistence& sp)                   { return sp.begin(); }
boost::counting_iterator<dp::SPersistenceIndex>     py_sp_end(dp::SPersistence& sp)                     { return sp.end(); }
unsigned                                            distance(dp::SPersistence& sp, 
                                                             const dp::SPersistenceIndex& i)            { return i - sp.begin(); }


/* SPersistenceIndex */
dp::SPersistenceIndex                               pair(const dp::SPersistenceIndex& i)                { return i->pair; }
bool                                                sign(const dp::SPersistenceIndex& i)                { return i->sign(); }
const dp::VChain&                                   cycle(const dp::SPersistenceIndex& i)               { return i->cycle; }
bool                                                index_eq(const dp::SPersistenceIndex& i, 
                                                             const dp::SPersistenceIndex& j)            { return i == j; }


void export_static_persistence()
{
    bp::class_<dp::SPersistenceIndex>("SPNode")
        .add_property("pair",   &pair)
        .def("sign",            &sign)
        .def("cycle",           &cycle,         bp::return_internal_reference<1>())
        .def("__eq__",          index_eq)
    ;

    bp::class_<dp::SPersistence>("StaticPersistence", bp::no_init)
        .def("__init__",        bp::make_constructor(&init_from_filtration))
        
        .def("pair_simplices",  (void (dp::SPersistence::*)())  &dp::SPersistence::pair_simplices)
        .def("__call__",        &distance)

        .def("__iter__",        bp::range(&py_sp_begin, &py_sp_end))
        .def("__len__",         &dp::SPersistence::size)
    ;
}
