#ifndef __PYTHON_FILTRATION_H__
#define __PYTHON_FILTRATION_H__

#include <topology/filtration.h>
#include <boost/python.hpp>
#include "python-simplex.h"

namespace bp = boost::python;

// Random access iterator into python's list (using integer indices)
class ListRandomAccessIterator:
    public boost::iterator_adaptor<ListRandomAccessIterator,                // Derived
                                   boost::counting_iterator<unsigned>,      // Base
                                   SimplexVD>                               // Value
{
    public:
        typedef                 ListRandomAccessIterator                                        Self;
        typedef                 boost::iterator_adaptor<ListRandomAccessIterator,           
                                                        boost::counting_iterator<unsigned>,     
                                                        SimplexVD>                              Parent;
                    
                                ListRandomAccessIterator()                                      {}

                                ListRandomAccessIterator(bp::list l, unsigned i):
                                    Parent(i), l_(l)                                            {}

    private:
        friend class boost::iterator_core_access;
        friend class FiltrationPythonIterator;

        Parent::reference       dereference() const
        {
            const SimplexVD& s = bp::extract<const SimplexVD&>(l_[*(this->base())]);
            return const_cast<SimplexVD&>(s);       // FIXME: get rid of const_cast
        }

        bp::list                l_;
};

// ComplexTraits describing complexes of type list
struct ListTraits
{
    typedef     bp::list                                        Complex;
    typedef     SimplexVD                                       Simplex;
    typedef     ListRandomAccessIterator                        Index;
    typedef     std::less<Index>                                IndexComparison;

    typedef     BinarySearchMap<Simplex, Index,
                                Simplex::VertexComparison>      SimplexIndexMap;

    static SimplexIndexMap      simplex_index_map(const Complex& l)             { return SimplexIndexMap(begin(l), end(l)); }
    static SimplexIndexMap      simplex_index_map(Index bg, Index end)          { return SimplexIndexMap(bg, end); }

    static unsigned             size(const Complex& l)                          { return bp::extract<unsigned>(l.attr("__len__")()); }
    static Index                begin(const Complex& l)                         { return Index(l, 0); }
    static Index                end(const Complex& l)                           { return Index(l, size(l)); }
};

typedef         Filtration<bp::list, unsigned, ListTraits>          ListFiltration;

#endif
