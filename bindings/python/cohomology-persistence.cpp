#include <topology/cohomology-persistence.h>

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/shared_ptr.hpp>
namespace bp = boost::python;

#include "cohomology-persistence.h"             // defines CohomPersistence
#include "optional.h"
namespace dp = dionysus::python;


// CohomPersistence
boost::shared_ptr<dp::CohomPersistence>     init_from_prime(unsigned p = 11)
{
    dp::CohomPersistence::Field field(p);       // Zp

    boost::shared_ptr<dp::CohomPersistence> chp(new dp::CohomPersistence(field));
    return chp;
}

bp::tuple                                   chp_add(dp::CohomPersistence& chp, bp::object bdry, dp::BirthID birth)
{
    dp::CohomPersistence::SimplexIndex      i;
    dp::CohomPersistence::Death             d;
    boost::tie(i,d)                                 = chp.add(bp::stl_input_iterator<dp::CohomPersistence::SimplexIndex>(bdry),
                                                              bp::stl_input_iterator<dp::CohomPersistence::SimplexIndex>(),
                                                              birth); 
    return bp::make_tuple(i,d);
}

dp::CohomPersistence::ZColumn::const_iterator     
cocycle_zcolumn_begin(dp::CohomPersistence::Cocycle& ccl)                   
{ return ccl.zcolumn.begin(); }

dp::CohomPersistence::ZColumn::const_iterator     
cocycle_zcolumn_end(dp::CohomPersistence::Cocycle& ccl)                   
{ return ccl.zcolumn.end(); }


// SimplexIndex
template<class T>
unsigned                            si_order(T& si)
{
    return si->order;
}


void export_cohomology_persistence()
{
    bp::class_<dp::CohomPersistence::SimplexIndex>("CHSimplexIndex")
        .add_property("order",          &si_order<dp::CohomPersistence::SimplexIndex>)
    ;
    
    bp::class_<dp::CohomPersistence::SNode>("CHSNode", bp::no_init)
        .add_property("coefficient",    &dp::CohomPersistence::SNode::coefficient)
        .add_property("si",             &dp::CohomPersistence::SNode::si)
    ;

    bp::class_<dp::CohomPersistence>("CohomologyPersistence")
        .def("__init__",        bp::make_constructor(&init_from_prime))
        .def("add",             &chp_add)
        
        .def("__iter__",        bp::range(&dp::CohomPersistence::begin, &dp::CohomPersistence::end))
    ;

    bp::class_<dp::CohomPersistence::Cocycle>("Cocycle", bp::no_init)
        .add_property("birth",  &dp::CohomPersistence::Cocycle::birth)
        .def("__iter__",        bp::range(&cocycle_zcolumn_begin, &cocycle_zcolumn_end))
    ;
}
