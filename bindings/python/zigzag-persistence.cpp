#include <topology/zigzag-persistence.h>
#include <topology/image-zigzag-persistence.h>

#include <boost/python.hpp>
#include <boost/python/stl_iterator.hpp>
#include <boost/shared_ptr.hpp>
namespace bp = boost::python;

#include "zigzag-persistence.h"             // defines ZZPersistence, IZZPersistence
#include "optional.h"


// ZigzagPersistence
bp::tuple                           zzp_add(ZZPersistence& zzp, bp::object bdry, BirthID birth)
{
    // Make ZColumn    
    // NB: it's extremely weird that I have to do it this way, 
    //     but for some reason I cannot just create boundary on the stack
    boost::shared_ptr<ZZPersistence::ZColumn> 
                                            boundary(new ZZPersistence::ZColumn(bp::stl_input_iterator<ZZPersistence::SimplexIndex>(bdry), 
                                                                                bp::stl_input_iterator<ZZPersistence::SimplexIndex>()));
    boundary->sort(zzp.cmp);

    ZZPersistence::SimplexIndex             i;
    ZZPersistence::Death                    d;
    boost::tie(i,d)                                 = zzp.add(*boundary, birth); 
    return bp::make_tuple(i,d);
}

ZZPersistence::Death                zzp_remove(ZZPersistence& zzp, ZZPersistence::SimplexIndex s, ZZPersistence::BirthID birth)
{
    return zzp.remove(s, birth); 
}


// ImageZigzagPersistence
bp::tuple                           izzp_add(IZZPersistence& izzp, bp::object bdry, bool subcomplex, BirthID birth)
{
    // Make ZColumn    
    // NB: it's extremely weird that I have to do it this way, 
    //     but for some reason I cannot just create boundary on the stack
    boost::shared_ptr<IZZPersistence::ZColumn> 
                                            boundary(new IZZPersistence::ZColumn(bp::stl_input_iterator<IZZPersistence::SimplexIndex>(bdry), 
                                                                                 bp::stl_input_iterator<IZZPersistence::SimplexIndex>()));
    boundary->sort(izzp.cmp);

    IZZPersistence::SimplexIndex            i;
    IZZPersistence::Death                   d;
    boost::tie(i,d)                                 = izzp.add(*boundary, subcomplex, birth); 
    return bp::make_tuple(i,d);
}

IZZPersistence::Death               izzp_remove(IZZPersistence& izzp, IZZPersistence::SimplexIndex s, IZZPersistence::BirthID birth)
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
    python_optional<BirthID>();   

    bp::class_<ZZPersistence::SimplexIndex>("SimplexIndex")
        .def("order",           &si_order<ZZPersistence::SimplexIndex>);
    
    bp::class_<IZZPersistence::SimplexIndex>("ISimplexIndex")
        .def("order",           &si_order<IZZPersistence::SimplexIndex>);

    bp::class_<ZZPersistence>("ZigzagPersistence")
        .def("add",             &zzp_add)
        .def("remove",          &zzp_remove)
    ;
    
    bp::class_<IZZPersistence>("ImageZigzagPersistence")
        .def("add",             &izzp_add)
        .def("remove",          &izzp_remove)
    ;
}
