#include <topology/simplex.h>
#include <utilities/indirect.h>
#include <iostream>

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/functional/hash.hpp>
using namespace boost::python;

#include "simplex.h"                // defines SimplexVD, Vertex, and Data


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
boost::shared_ptr<Simplex<V,T> >    init_from_iterator(object iter)                      
{ 
    boost::shared_ptr<Simplex<V,T> > p(new Simplex<V,T>(stl_input_iterator<V>(iter), stl_input_iterator<V>()));
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
    class_<SimplexVD>("Simplex")
        .def("__init__",            make_constructor(&init_from_iterator<Vertex, Data>))

        .def("add",                 &SimplexVD::add)
        .add_property("boundary",   range(&SimplexVD::boundary_begin, &SimplexVD::boundary_end))
        .def("contains",            &SimplexVD::contains)
        .def("join",                (void (SimplexVD::*)(const SimplexVD&)) &SimplexVD::join)
        .def("dimension",           &SimplexVD::dimension)
        
        .add_property("vertices",   range(&vertices_begin<Vertex,Data>, &vertices_end<Vertex,Data>))
        .def(repr(self))

        .def("__hash__",            &hash_simplex<Vertex, Data>)
        .def("__eq__",              &eq_simplex<Vertex, Data>)
    ;

    class_<SimplexObject>("SimplexObject")
        .def("__getattribute__",    &SimplexObject::getattribute)
    ;

    def("vertex_cmp",               &vertex_comparison<Vertex, Data>);
}
