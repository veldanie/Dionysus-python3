#ifndef __PYTHON_FILTRATION_H__
#define __PYTHON_FILTRATION_H__

#include <topology/filtration.h>
#include <boost/python.hpp>
#include "simplex.h"
#include "utils.h"                      // for ListRandomAccessIterator

namespace bp = boost::python;

namespace dionysus { 
namespace python   {

// ComplexTraits describing complexes of type list
struct ListTraits
{
    typedef     bp::list                                        Complex;
    typedef     SimplexObject                                   Simplex;
    typedef     ListRandomAccessIterator<Simplex>               Index;
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

} } // namespace dionysus::python

#endif
