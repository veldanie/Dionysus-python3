#include <topology/simplex.h>
#include <utilities/indirect.h>
#include <iostream>

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
namespace bp = boost::python;

#include "simplex.h"                // defines SimplexVD, Vertex, and Data
namespace dp = dionysus::python;


/* Various wrappers for exposing Simplex to Python */
// `vertices` property
template<class V, class T>
typename Simplex<V,T>::VertexContainer::const_iterator
                                    vertices_begin(const Simplex<V,T>& s)       { return s.vertices().begin(); }
template<class V, class T>
typename Simplex<V,T>::VertexContainer::const_iterator
                                    vertices_end(const Simplex<V,T>& s)         { return s.vertices().end(); }

// Constructor from iterator
template<class V, class T>
boost::shared_ptr<Simplex<V,T> >    init_from_iterator(bp::object iter)                      
{ 
    boost::shared_ptr<Simplex<V,T> > p(new Simplex<V,T>(bp::stl_input_iterator<V>(iter), bp::stl_input_iterator<V>()));
    return p;
}

// Simplex hash
template<class V, class T>
size_t                              hash_simplex(const Simplex<V,T>& s)
{
    return boost::hash_range(s.vertices().begin(), s.vertices().end());
}

template<class V, class T>
size_t                              eq_simplex(const Simplex<V,T>& a, const Simplex<V,T>& b)
{
    return vertex_comparison(a,b) == 0;
}


/* Comparisons */
// VertexComparison
template<class V, class T>
int                                 vertex_comparison(const Simplex<V,T>& a, const Simplex<V,T>& b)
{
    return ThreeOutcomeCompare<typename Simplex<V,T>::VertexComparison>().compare(a,b);
}


void export_simplex()
{
    bp::class_<dp::SimplexVD>("Simplex")
        .def("__init__",            bp::make_constructor(&init_from_iterator<dp::Vertex, dp::Data>))

        .def("add",                 &dp::SimplexVD::add)
        .add_property("boundary",   bp::range(&dp::SimplexVD::boundary_begin, &dp::SimplexVD::boundary_end))
        .def("contains",            &dp::SimplexVD::contains)
        .def("join",                (void (dp::SimplexVD::*)(const dp::SimplexVD&)) &dp::SimplexVD::join)
        .def("dimension",           &dp::SimplexVD::dimension)
        
        .add_property("vertices",   bp::range(&vertices_begin<dp::Vertex, dp::Data>, &vertices_end<dp::Vertex, dp::Data>))
        .def(repr(bp::self))

        .def("__hash__",            &hash_simplex<dp::Vertex, dp::Data>)
        .def("__eq__",              &eq_simplex<dp::Vertex, dp::Data>)
    ;

    bp::class_<dp::SimplexObject>("SimplexObject")
        .def("__getattribute__",    &dp::SimplexObject::getattribute)
    ;

    bp::def("vertex_cmp",           &vertex_comparison<dp::Vertex, dp::Data>);
}
