#include <topology/filtration.h>
#include <boost/iterator.hpp>
#include "simplex.h"
#include <iostream>

#include <boost/python.hpp>
using namespace boost::python;


#include "filtration.h"      // defines ListFiltration, ListTraits, ListRandomAccessIterator
#include "utils.h"           // defines PythonCmp

boost::shared_ptr<ListFiltration>  init_from_list(list lst, object cmp)
{
    boost::shared_ptr<ListFiltration>  p(new ListFiltration(ListTraits::begin(lst),
                                                            ListTraits::end(lst),
                                                            PythonCmp(cmp)));
    return p;
}

FiltrationPythonIterator
lf_begin(const ListFiltration& lf)
{ return lf.begin(); }

FiltrationPythonIterator
lf_end(const ListFiltration& lf)
{ return lf.end(); }

unsigned
lf_getitem(const ListFiltration& lf, unsigned i)       
{ return *(lf_begin(lf) + i); }


void export_filtration()
{
    class_<ListFiltration>("Filtration", no_init)
        .def("__init__",        make_constructor(&init_from_list))

        .def("__getitem__",     &lf_getitem)
        .def("__iter__",        range(&lf_begin, &lf_end))
        .def("__len__",         &ListFiltration::size)
    ;
}
