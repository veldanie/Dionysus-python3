#ifndef __PYTHON_STATIC_PERSISTENCE_H__
#define __PYTHON_STATIC_PERSISTENCE_H__

#include <topology/static-persistence.h>

namespace dionysus {
namespace python   {

typedef         StaticPersistence<>             SPersistence;
typedef         SPersistence::OrderElement      SPersistenceNode;
typedef         SPersistence::OrderIndex        SPersistenceIndex;

} } // namespace dionysus::python

#endif
