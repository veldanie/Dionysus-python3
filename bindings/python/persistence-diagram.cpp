#include<topology/persistence-diagram.h>
#include<utilities/types.h>

#include<boost/python.hpp>
#include<boost/python/init.hpp>
#include<boost/shared_ptr.hpp>
#include<boost/python/stl_iterator.hpp>
#include<boost/python/def.hpp>


namespace bp = boost::python;


namespace dionysus{
namespace python{

typedef bp::object Data;
typedef PDPoint<Data> PointD;
typedef PersistenceDiagram<Data> PersistenceDiagramD;

} } //namespace dionysus::python

namespace dp = dionysus::python;

// Easy way out of overloads
RealType        get_x (const dp::PointD& p)         { return p.x(); }
RealType        get_y (const dp::PointD& p)         { return p.y(); }
bp::object      get_data(const dp::PointD& p)       { return p.data(); }

void export_point( )
{
    bp::class_<dp::PointD>("Point")
        .def(                   bp::init<RealType, RealType, bp::optional<dp::Data> >())
        .add_property("x",      &get_x)
        .add_property("y",      &get_y)
        .add_property("data",   &get_data)
        .def(                   repr(bp::self))
    ;
}


boost::shared_ptr<dp::PersistenceDiagramD>      init_from_points_sequence(Dimension dimension, bp::object point_sequence)
{
    typedef     bp::stl_input_iterator<dp::PointD>  PointIterator;

    PointIterator beg = PointIterator(point_sequence), end = PointIterator();
    boost::shared_ptr<dp::PersistenceDiagramD> p(new dp::PersistenceDiagramD(dimension));

    for(PointIterator cur = beg; cur != end; cur++)
        (*p).push_back(*cur);
    return p;

}

RealType    bottleneck_distance_adapter(const dp::PersistenceDiagramD& dgm1, const dp::PersistenceDiagramD& dgm2)
{
    return bottleneck_distance(dgm1, dgm2);
}    

void export_persistence_diagram()
{
    bp::class_<dp::PersistenceDiagramD>("PersistenceDiagram")
        .def("__init__",            bp::make_constructor(&init_from_points_sequence))
        .def(                       bp::init<Dimension>())
        .def("append",              &dp::PersistenceDiagramD::push_back)
        .add_property("dimension",  &dp::PersistenceDiagramD::dimension)
        .def(                       repr(bp::self))
        .def("__iter__",            bp::range(&dp::PersistenceDiagramD::begin, &dp::PersistenceDiagramD::end))
        .def("__len__",             &dp::PersistenceDiagramD::size)
    ;

    bp::def("bottleneck_distance",  &bottleneck_distance_adapter);
}
