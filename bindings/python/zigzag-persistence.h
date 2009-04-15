#ifndef __PYTHON_ZIGZAG_PERSISTENCE_H__
#define __PYTHON_ZIGZAG_PERSISTENCE_H__

#include <topology/zigzag-persistence.h>
#include <topology/image-zigzag-persistence.h>
#include <boost/python.hpp>

namespace dionysus {
namespace python   {

//typedef         int                             BirthID;
//typedef         boost::python::long_            BirthID;
typedef         boost::python::object           BirthID;

typedef         ZigzagPersistence<BirthID>      ZZPersistence;
typedef         ImageZigzagPersistence<BirthID> IZZPersistence;

} } // namespace dionysus::python

#endif
