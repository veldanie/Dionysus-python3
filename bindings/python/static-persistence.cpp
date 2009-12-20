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
    dp::PythonFiltration& sf = bp::extract<dp::PythonFiltration&>(f);
    boost::shared_ptr<dp::SPersistence> p(new dp::SPersistence(sf));
    return p;
}

unsigned                                distance(dp::SPersistence& sp, 
                                                 const dp::SPersistenceIndex& i)            { return sp.iterator_to(i) - sp.begin(); }

/* SPNode */
const dp::SPersistenceNode&             pair(const dp::SPersistenceNode& sn)                { return *sn.pair; }

/* PersistenceSimplexMap */
const dp::SimplexVD&                    psmap_getitem(const dp::PersistenceSimplexMap& psmap, 
                                                      const dp::SPersistenceIndex& i)       { return psmap[i]; }

void export_static_persistence()
{
    bp::class_<dp::SPersistenceNode>("SPNode", bp::no_init)
        .def("pair",            &pair,                      bp::return_internal_reference<1>())
        .add_property("cycle",  &dp::SPersistenceNode::cycle)
        .def("sign",            &dp::SPersistenceNode::sign)
        .def("unpaired",        &dp::SPersistenceNode::unpaired)
    ;

    bp::class_<dp::SPersistence>("StaticPersistence", bp::no_init)
        .def("__init__",        bp::make_constructor(&init_from_filtration))
        
        .def("pair_simplices",  (void (dp::SPersistence::*)())  &dp::SPersistence::pair_simplices)
        .def("__call__",        &distance)
        .def("make_simplex_map",&dp::SPersistence::make_simplex_map<dp::PythonFiltration>)

        .def("__iter__",        bp::range<bp::return_internal_reference<1> >(&dp::SPersistence::begin, &dp::SPersistence::end))
        .def("__len__",         &dp::SPersistence::size)
    ;

    bp::class_<dp::PersistenceSimplexMap>("PersistenceSimplexMap", bp::no_init)
        .def("__getitem__",     &psmap_getitem,             bp::return_internal_reference<1>())
    ;
}
