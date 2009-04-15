#include <topology/zigzag-persistence.h>
#include <topology/image-zigzag-persistence.h>

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/shared_ptr.hpp>
namespace bp = boost::python;

#include "zigzag-persistence.h"             // defines ZZPersistence, IZZPersistence
#include "optional.h"
namespace dp = dionysus::python;


// ZigzagPersistence
bp::tuple                           zzp_add(dp::ZZPersistence& zzp, bp::object bdry, dp::BirthID birth)
{
    // Make ZColumn    
    // NB: it's extremely weird that I have to do it this way, 
    //     but for some reason I cannot just create boundary on the stack
    boost::shared_ptr<dp::ZZPersistence::ZColumn> 
                                            boundary(new dp::ZZPersistence::ZColumn(bp::stl_input_iterator<dp::ZZPersistence::SimplexIndex>(bdry), 
                                                                                    bp::stl_input_iterator<dp::ZZPersistence::SimplexIndex>()));
    boundary->sort(zzp.cmp);

    dp::ZZPersistence::SimplexIndex         i;
    dp::ZZPersistence::Death                d;
    boost::tie(i,d)                                 = zzp.add(*boundary, birth); 
    return bp::make_tuple(i,d);
}

dp::ZZPersistence::Death            zzp_remove(dp::ZZPersistence& zzp, dp::ZZPersistence::SimplexIndex s, dp::ZZPersistence::BirthID birth)
{
    return zzp.remove(s, birth); 
}


// ImageZigzagPersistence
bp::tuple                           izzp_add(dp::IZZPersistence& izzp, bp::object bdry, bool subcomplex, dp::BirthID birth)
{
    // Make ZColumn    
    // NB: it's extremely weird that I have to do it this way, 
    //     but for some reason I cannot just create boundary on the stack
    boost::shared_ptr<dp::IZZPersistence::ZColumn> 
                                            boundary(new dp::IZZPersistence::ZColumn(bp::stl_input_iterator<dp::IZZPersistence::SimplexIndex>(bdry), 
                                                                                     bp::stl_input_iterator<dp::IZZPersistence::SimplexIndex>()));
    boundary->sort(izzp.cmp);

    dp::IZZPersistence::SimplexIndex            i;
    dp::IZZPersistence::Death                   d;
    boost::tie(i,d)                                 = izzp.add(*boundary, subcomplex, birth); 
    return bp::make_tuple(i,d);
}

dp::IZZPersistence::Death           izzp_remove(dp::IZZPersistence& izzp, dp::IZZPersistence::SimplexIndex s, dp::IZZPersistence::BirthID birth)
{
    return izzp.remove(s, birth); 
}


// SimplexIndex
template<class T>
unsigned                            si_order(T& si)
{
    return si->order;
}


void export_zigzag_persistence()
{
    python_optional<dp::BirthID>();   

    bp::class_<dp::ZZPersistence::SimplexIndex>("SimplexIndex")
        .def("order",           &si_order<dp::ZZPersistence::SimplexIndex>);
    
    bp::class_<dp::IZZPersistence::SimplexIndex>("ISimplexIndex")
        .def("order",           &si_order<dp::IZZPersistence::SimplexIndex>);

    bp::class_<dp::ZZPersistence>("ZigzagPersistence")
        .def("add",             &zzp_add)
        .def("remove",          &zzp_remove)
    ;
    
    bp::class_<dp::IZZPersistence>("ImageZigzagPersistence")
        .def("add",             &izzp_add)
        .def("remove",          &izzp_remove)
    ;
}
