#include<topology/persistence-diagram.h>
#include<utilities/types.h>

#include<boost/python.hpp>
#include<boost/python/init.hpp>
#include<boost/shared_ptr.hpp>
#include<boost/python/stl_iterator.hpp>
#include <boost/python/def.hpp>


namespace bp = boost::python;


namespace dionysus{
namespace python{

typedef bp::object Data;
typedef PDPoint<Data> PointD;
typedef PersistenceDiagram<Data> PersistenceDiagramD;

} } //namespace dionysus::python

namespace dp = dionysus::python;


template<class PDPoint>
RealType    get_x_coord( const PDPoint& p ){
    return p.x( );
}

template<class PDPoint>
RealType    get_y_coord( const PDPoint& p ){
    return p.y( );
}

template<class PDPoint>
bp::object    get_data( const PDPoint& p ){
    return p.data( );
}

void export_point( ){

    bp::class_<dp::PointD>("Point")
    .def( bp::init< RealType, RealType, bp::optional<dp::Data>  >( ) )
    .add_property( "x", &get_x_coord<dp::PointD> )
    .add_property( "y", &get_y_coord<dp::PointD> )
    .add_property( "data", &get_data<dp::PointD> )
    .def( repr(bp::self) )
    ;

}



template<class PersistenceDiagram, class Point>
boost::shared_ptr<PersistenceDiagram>    init_from_points_sequence( bp::object point_sequence ){

    typedef bp::stl_input_iterator<Point> PointIterator;

    PointIterator beg = PointIterator( point_sequence ), end = PointIterator( );
    // The following line is commented out until we can figure out the Evaluator class in make_point
    //boost::shared_ptr<PersistenceDiagram> p(  new PersistenceDiagram( beg, end );
    boost::shared_ptr<PersistenceDiagram> p(  new PersistenceDiagram( ) );

    for( PointIterator cur=beg;  cur!=end; cur++ )
        (*p).push_back( *cur );
    return p;

}


template<class PersistenceDiagram>
Dimension get_dimension( PersistenceDiagram dgm ){
    return dgm.dimension( );
}

template<class PersistenceDiagram>
SizeType get_length( PersistenceDiagram dgm ){
    return dgm.size( );
}

void export_persistence_diagram( ){

    bp::class_<dp::PersistenceDiagramD>("PersistenceDiagram")
    .def( "__init__", bp::make_constructor(  &init_from_points_sequence< dp::PersistenceDiagramD, dp::PointD > ) )
    .def( bp::init< Dimension >( ) )
    .def( "append", &dp::PersistenceDiagramD::push_back )
    .add_property( "dimension", &get_dimension<dp::PersistenceDiagramD> )
    .def( repr(bp::self) )
    .def( "__iter__", bp::range( &dp::PersistenceDiagramD::begin, &dp::PersistenceDiagramD::end ) )
    .def( "__len__", &get_length<dp::PersistenceDiagramD> )
    ;

    bp::def( "bottleneck_distance", bottleneck_distance<dp::PersistenceDiagramD,dp::PersistenceDiagramD> );
}
