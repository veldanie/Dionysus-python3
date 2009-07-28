#ifndef __WEIGHTED_RIPS_H__
#define __WEIGHTED_RIPS_H__

#include <vector>
#include <string>
#include "simplex.h"
#include "rips.h"
#include <boost/iterator/counting_iterator.hpp>

/**
 * WeightedRips class
 *
 * Class providing basic operations to work with Rips complexes. It implements Bron-Kerbosch algorithm, 
 * and provides simple wrappers for various functions.
 *
 * Distances_ is expected to define types IndexType and DistanceType as well as 
 *               provide operator()(...) which given two IndexTypes should return 
 *               the distance between them. There should be methods begin() and end() 
 *               for iterating over IndexTypes as well as a method size().
 */
template<class Distances_, class Simplex_ = Simplex<typename Distances_::IndexType, typename Distances_::DistanceType> >
class WeightedRips : public Rips<Distances_, Simplex_>
{
    public:

        /* redeclaring the typedefs because they cannot be inherited at compile-time */
        typedef             Distances_                                      Distances; 
        typedef             typename Distances::IndexType                   IndexType;
        typedef             typename Distances::DistanceType                DistanceType;

        typedef             Simplex_                                        Simplex;
        typedef             typename Simplex::Vertex                        Vertex;             // should be the same as IndexType
        typedef             typename Simplex::VertexContainer               VertexContainer;

        class               Evaluator;
        class               Comparison;

    public:
                            WeightedRips(const Distances& distances):
                                Rips<Distances_, Simplex_>(distances)                             {}

};

template<class Distances_, class Simplex_>
class WeightedRips<Distances_, Simplex_>::Evaluator: public Rips<Distances_, Simplex_>::Evaluator
{
    public:
                            Evaluator(const Distances& distances): 
                                Rips<Distances_, Simplex_>::Evaluator(distances)                       {}

        DistanceType       operator()(const Simplex& s) const { return s.data(); }
};

template<class Distances_, class Simplex_>
class WeightedRips<Distances_, Simplex_>::Comparison: public Rips<Distances_, Simplex_>::Comparison
{
    public:
                            Comparison(const Distances& distances):
                                Rips<Distances_, Simplex_>::Comparison(distances)                            {}

        bool                operator()(const Simplex& s1, const Simplex& s2) const    
        { 
                            if (s1.dimension() != s2.dimension())
                                return s1.dimension() < s2.dimension();
                            return s1.data() < s2.data();
        }
};

#include "weighted-rips.hpp"

#endif // __WEIGHTED_RIPS_H__
