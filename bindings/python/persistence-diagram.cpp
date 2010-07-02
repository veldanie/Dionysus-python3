#include<topology/persistence-diagram.h>
#include<utilities/types.h>

#include<boost/python.hpp>
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

// TBD: Make the Data argument optional. Use bp::None ?
template<class PDPoint>
boost::shared_ptr< PDPoint >    init_from_coords( bp::object x_obj, bp::object y_obj, dp::Data d ){

    RealType x = bp::extract<RealType>( x_obj ); RealType y = bp::extract<RealType>( y_obj );
    boost::shared_ptr< PDPoint > p( new PDPoint( x, y, d ) );
    return p;

}

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
    .def( "__init__", bp::make_constructor( &init_from_coords<dp::PointD> ) )
    .add_property( "x", &get_x_coord<dp::PointD> )
    .add_property( "y", &get_y_coord<dp::PointD> )
    .add_property( "data", &get_data<dp::PointD> )
    .def( repr(bp::self) )
    ;

}



template<class PersistenceDiagramD, class PointD>
boost::shared_ptr<PersistenceDiagramD>    init_from_points_sequence( bp::object point_sequence ){

    typedef bp::stl_input_iterator<PointD> PointDIterator;

    PointDIterator beg = PointDIterator( point_sequence ), end = PointDIterator( );
    // The following line is commented out until we can figure out the Evaluator class in make_point
    //boost::shared_ptr<PersistenceDiagramD> p(  new PersistenceDiagramD( beg, end );
    boost::shared_ptr<PersistenceDiagramD> p(  new PersistenceDiagramD( ) );

    for( PointDIterator cur=beg;  cur!=end; cur++ )
        (*p).push_back( *cur );
    return p;

}

void export_persistence_diagram( ){

    bp::class_<dp::PersistenceDiagramD>("PersistenceDiagram")
    .def( "__init__", bp::make_constructor(  &init_from_points_sequence< dp::PersistenceDiagramD, dp::PointD > ) )
    .def( "append", &dp::PersistenceDiagramD::push_back )
    .add_property( "points", bp::range( &dp::PersistenceDiagramD::begin, &dp::PersistenceDiagramD::end ) )
    .def( repr(bp::self) )
    .def( "__sub__", &bottleneck_distance<dp::PersistenceDiagramD,dp::PersistenceDiagramD> )
    ;

    bp::def( "BottleneckDistance", bottleneck_distance<dp::PersistenceDiagramD,dp::PersistenceDiagramD> );
}
