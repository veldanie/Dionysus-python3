#include <topology/simplex.h>
#include <utilities/indirect.h>
#include <iostream>

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/shared_ptr.hpp>
using namespace boost::python;

/* Various wrappers for exposing Simplex to Python */
// `data` property
template<class V, class T>
typename Simplex<V,T>::Data         get_data(const Simplex<V,T>& s)             { return s.data(); }
template<class V, class T>
void                                set_data(Simplex<V,T>& s, 
                                             typename Simplex<V,T>::Data d)     { s.data() = d; }
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

// Constructor from iterator and data
template<class V, class T>
boost::shared_ptr<Simplex<V,T> >    init_from_iterator_data(object iter, T data)                      
{ 
    boost::shared_ptr<Simplex<V,T> > p(new Simplex<V,T>(stl_input_iterator<V>(iter), stl_input_iterator<V>(), data));
    return p;
}

/* Comparisons */
// VertexComparison
template<class V, class T>
int                                 vertex_comparison(const Simplex<V,T>& a, const Simplex<V,T>& b)
{
    return ThreeOutcomeCompare<typename Simplex<V,T>::VertexComparison>().compare(a,b);
}

// DataComparison
template<class V, class T>
int                                 data_comparison(const Simplex<V,T>& a, const Simplex<V,T>& b)
{
    return ThreeOutcomeCompare<typename Simplex<V,T>::DataComparison>().compare(a,b);
}

// DataDimensionComparison
template<class V, class T>
int                                 data_dimension_comparison(const Simplex<V,T>& a, const Simplex<V,T>& b)
{
    return ThreeOutcomeCompare<typename Simplex<V,T>::DataDimensionComparison>().compare(a,b);
}

#include "python-simplex.h"         // defines SimplexVD, Vertex, and Data

void export_simplex()
{
    class_<SimplexVD>("Simplex")
        .def("__init__",            make_constructor(&init_from_iterator<Vertex, Data>))
        .def("__init__",            make_constructor(&init_from_iterator_data<Vertex, Data>))

        .def("add",                 &SimplexVD::add)
        .add_property("boundary",   range(&SimplexVD::boundary_begin, &SimplexVD::boundary_end))
        .def("contains",            &SimplexVD::contains)
        .def("join",                (void (SimplexVD::*)(const SimplexVD&)) &SimplexVD::join)
        .def("dimension",           &SimplexVD::dimension)
        .add_property("data",       &get_data<Vertex,Data>, &set_data<Vertex,Data>)
        
        .add_property("vertices",   range(&vertices_begin<Vertex,Data>, &vertices_end<Vertex,Data>))
        .def(repr(self))
    ;

    def("vertex_cmp",               &vertex_comparison<Vertex, Data>);
    def("data_cmp",                 &data_comparison<Vertex, Data>);
    def("data_dim_cmp",             &data_dimension_comparison<Vertex, Data>);
}
