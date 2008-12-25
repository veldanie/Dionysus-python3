#include <topology/filtration.h>
#include <boost/iterator.hpp>
#include "python-simplex.h"
#include <iostream>

#include <boost/python.hpp>
using namespace boost::python;


#include "python-filtration.h"      // defines ListFiltration, ListTraits, ListRandomAccessIterator

// Filtration python iterator interface    
class FiltrationPythonIterator:
    public boost::iterator_adaptor<FiltrationPythonIterator,        // Derived
                                   ListFiltration::Index,           // Base
                                   unsigned>                        // Value
{
    public:
        typedef                 FiltrationPythonIterator                                        Self;
        typedef                 boost::iterator_adaptor<FiltrationPythonIterator,           
                                                        ListFiltration::Index,     
                                                        unsigned>                               Parent;

                                FiltrationPythonIterator(ListFiltration::Index i):
                                    Parent(i)                                                   {}

    private:
        friend class boost::iterator_core_access;

        Parent::reference dereference() const
        {
            // FIXME: I hate the const_cast here, how do I get rid of it?
            return const_cast<unsigned&>(this->base()->base().base());
        }
};

FiltrationPythonIterator
py_begin(const ListFiltration& lf)
{ return lf.begin(); }

FiltrationPythonIterator
py_end(const ListFiltration& lf)
{ return lf.end(); }

    
struct PythonCmp
{
    template<class T>
    bool            operator()(T x1, T x2) const        { return cmp_(x1, x2) < 0; }

                    PythonCmp(object cmp): cmp_(cmp)    {}

    object cmp_;
};

boost::shared_ptr<ListFiltration>  init_from_list(list lst, object cmp)
{
    boost::shared_ptr<ListFiltration>  p(new ListFiltration(ListTraits::begin(lst),
                                                            ListTraits::end(lst),
                                                            PythonCmp(cmp)));
    return p;
}

void export_filtration()
{
    class_<ListFiltration>("Filtration", no_init)
        .def("__init__",        make_constructor(&init_from_list))
        //.def("simplex", )

        .def("__iter__",        range(&py_begin, &py_end))
        .def("__len__",         &ListFiltration::size)
    ;
}
