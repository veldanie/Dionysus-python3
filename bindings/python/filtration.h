#ifndef __PYTHON_FILTRATION_H__
#define __PYTHON_FILTRATION_H__

#include <topology/filtration.h>
#include <boost/python.hpp>
#include "simplex.h"

namespace bp = boost::python;

// Random access iterator into python's list (using integer indices)
class ListRandomAccessIterator:
    public boost::iterator_adaptor<ListRandomAccessIterator,                // Derived
                                   boost::counting_iterator<unsigned>,      // Base
                                   SimplexObject,                           // Value
                                   boost::use_default,
                                   SimplexObject>
{
    public:
        typedef                 ListRandomAccessIterator                                        Self;
        typedef                 boost::iterator_adaptor<ListRandomAccessIterator,           
                                                        boost::counting_iterator<unsigned>,     
                                                        SimplexObject,
                                                        boost::use_default,
                                                        SimplexObject>                          Parent;
                    
                                ListRandomAccessIterator()                                      {}

                                ListRandomAccessIterator(bp::list l, unsigned i):
                                    Parent(i), l_(l)                                            {}

    private:
        friend class boost::iterator_core_access;
        friend class FiltrationPythonIterator;

        Parent::reference       dereference() const                                             { return bp::object(l_[*(this->base())]); }

        bp::list                l_;
};

// ComplexTraits describing complexes of type list
struct ListTraits
{
    typedef     bp::list                                        Complex;
    typedef     SimplexObject                                   Simplex;
    typedef     ListRandomAccessIterator                        Index;
    typedef     std::less<Index>                                IndexComparison;

    typedef     BinarySearchMap<SimplexVD, Index,
                                SimplexVD::VertexComparison>    SimplexIndexMap;

    static SimplexIndexMap      simplex_index_map(const Complex& l)             { return SimplexIndexMap(begin(l), end(l)); }
    static SimplexIndexMap      simplex_index_map(Index bg, Index end)          { return SimplexIndexMap(bg, end); }

    static unsigned             size(const Complex& l)                          { return bp::len(l); }
    static Index                begin(const Complex& l)                         { return Index(l, 0); }
    static Index                end(const Complex& l)                           { return Index(l, size(l)); }
};

typedef         Filtration<bp::list, unsigned, ListTraits>          ListFiltration;


// Filtration python iterator interface    
class FiltrationPythonIterator:
    public boost::iterator_adaptor<FiltrationPythonIterator,    // Derived
                                   ListFiltration::Index,       // Base
                                   unsigned>                    // Value
{
    public:
        typedef                 FiltrationPythonIterator                                        Self;
        typedef                 boost::iterator_adaptor<Self,           
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

#endif
