#ifndef __FILTRATION_H__
#define __FILTRATION_H__

#include <vector>
#include <iostream>
#include "complex-traits.h"
#include "utilities/indirect.h"
#include "utilities/property-maps.h"


// Class: Filtration
//
// Filtration keeps track of the ordering of the simplices in a complex. 
// The most significant function it provides is <boundary()> which converts
// the boundary of a simplex at a given index into a list of indices.
//
// TODO: this is really specialized for an std::vector<> Complex; eventually generalize
// TODO: should we derive from Order?
template<class Complex_, 
         class Index_ =             size_t, 
         class ComplexTraits_ =     ComplexTraits<Complex_> >
class Filtration
{
    public:
        // Typedefs: Template parameters
        typedef                 Index_                                          IntermediateIndex;
        typedef                 Complex_                                        Complex;
        typedef                 ComplexTraits_                                  ComplexTraits;

        // Typedefs: Complex
        typedef                 typename ComplexTraits::Index                   ComplexIndex;
        typedef                 typename ComplexTraits::Simplex                 Simplex;
        typedef                 typename ComplexTraits::SimplexIndexMap         SimplexIndexMap;
        typedef                 typename Simplex::Boundary                      SimplexBoundary;
        typedef                 std::vector<IntermediateIndex>                  IndexBoundary;

        // Typedefs: Order
        typedef                 std::vector<ComplexIndex>                       Order;
        typedef                 typename Order::const_iterator                  Index;
        typedef                 std::vector<IntermediateIndex>                  ReverseOrder;
        typedef                 typename ReverseOrder::const_iterator           ReverseOrderIndex;

        // Constructor: Filtration(bg, end, cmp)
                                template<class Comparison>
                                Filtration(ComplexIndex bg, ComplexIndex end, const Comparison& cmp = Comparison());

        const Simplex&          simplex(Index i) const                          { return **i; }

        // Function: boundary(i, bdry, map)
        // Computes boundary of a given index `i` in terms of other indices
        template<class Cycle, class Map>
        void                    boundary(const Index& i, Cycle& bdry, const Map& map) const;

        Index                   begin() const                                   { return order_.begin(); }
        Index                   end() const                                     { return order_.end(); }
        size_t                  size() const                                    { return order_.size(); }

        std::ostream&           operator<<(std::ostream& out) const;

    private:
        Order                   order_;
        ReverseOrder            reverse_order_;
        OffsetMap<ComplexIndex, 
                  ReverseOrderIndex>        
                                complex_order_map_;
        SimplexIndexMap         simplex_index_map_;
};

template<class C, class I, class CT>
std::ostream&
operator<<(std::ostream& out, const Filtration<C,I,CT>& f)                      { return f.operator<<(out); }


template<class Functor_, class Filtration_>
class ThroughFiltration
{
    public:
        typedef                 Filtration_                                     Filtration;
        typedef                 Functor_                                        Functor;

        typedef                 typename Functor::result_type                   result_type;
        typedef                 typename Filtration::Index                      first_argument_type;

                                ThroughFiltration(const Filtration& filtration,
                                                  const Functor&    functor):
                                    filtration_(filtration),
                                    functor_(functor)                           {}

        result_type             operator()(first_argument_type a) const         { return functor_(filtration_.simplex(a)); }

    private:
        const Filtration&       filtration_;
        const Functor&          functor_;
};

template<class Filtration, class Functor>
ThroughFiltration<Functor, Filtration>
evaluate_through_filtration(const Filtration& filtration, const Functor& functor)
{ return ThroughFiltration<Functor, Filtration>(filtration, functor); }

#include "filtration.hpp"

#endif // __FILTRATION_H__
