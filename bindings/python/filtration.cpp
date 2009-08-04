#include <topology/filtration.h>
#include <boost/iterator.hpp>
#include "simplex.h"
#include <iostream>

#include <boost/python.hpp>
namespace bp = boost::python;


#include "filtration.h"      // defines ListFiltration, ListTraits
#include "utils.h"           // defines PythonCmp
namespace dp = dionysus::python;

boost::shared_ptr<dp::ListFiltration>  init_from_list(bp::list lst, bp::object cmp)
{
    boost::shared_ptr<dp::ListFiltration>  p(new dp::ListFiltration(dp::ListTraits::begin(lst),
                                                                    dp::ListTraits::end(lst),
                                                                    dp::PythonCmp(cmp)));
    return p;
}

dp::FiltrationPythonIterator
lf_begin(const dp::ListFiltration& lf)
{ return lf.begin(); }

dp::FiltrationPythonIterator
lf_end(const dp::ListFiltration& lf)
{ return lf.end(); }

unsigned
lf_getitem(const dp::ListFiltration& lf, unsigned i)       
{ return *(lf_begin(lf) + i); }


void export_filtration()
{
    bp::class_<dp::ListFiltration>("Filtration", bp::no_init)
        .def("__init__",        bp::make_constructor(&init_from_list))

        .def("__getitem__",     &lf_getitem)
        .def("__iter__",        bp::range(&lf_begin, &lf_end))
        .def("__len__",         &dp::ListFiltration::size)
    ;
}
