#ifndef __RIPS_H__
#define __RIPS_H__

#include <vector>
#include <string>
#include "simplex.h"
#include "filtrationsimplex.h"

/**
 * Rips complex class
 *
 * Distances_ is expected to define types IndexType and DistanceType as well as 
 *               provide operator()(...) which given two IndexTypes should return 
 *               the distance between them. There should be methods begin() and end() 
 *               for iterating over IndexTypes as well as a method size().
 */
template<class Distances_, class Simplex_ = SimplexWithVertices<typename Distances_::IndexType> >
class Rips
{
    public:
        typedef             Distances_                                      Distances; 
        typedef             typename Distances::IndexType                   IndexType;
        typedef             typename Distances::DistanceType                DistanceType;

        typedef             Simplex_                                        Simplex;
        typedef             std::vector<Simplex>                            SimplexVector;

        class               Evaluator;

    public:
                            Rips(const Distances& distances): 
                                distances_(distances)                       {}

        void                generate(Dimension k, DistanceType max);        /// generate k-skeleton of the Rips complex
        const SimplexVector&
                            simplices() const                               { return simplices_; }
        size_t              size() const                                    { return simplices_.size(); }

        const Distances&    distances() const                               { return distances_; }
        DistanceType        max_distance() const;

        void                print() const;

    private:
        struct              ComparePair;

        const Distances&    distances_;
        SimplexVector       simplices_;
};

template<class Distances_, class Simplex_>
class Rips<Distances_, Simplex_>::Evaluator: public ::Evaluator<Simplex_>
{
    public:
        typedef             Simplex_                                        Simplex;

                            Evaluator(const Distances& distances): 
                                distances_(distances)                       {}

        virtual DistanceType   
                            value(const Simplex& s) const;

    private:
        const Distances&    distances_;
};

/**
 * ExplicitDistances stores the pairwise distances of Distances_ instance passed at construction. 
 * It's a protypical Distances template argument for the Rips complex.
 */
template<class Distances_>
class ExplicitDistances
{
    public:
        typedef             Distances_                                      Distances;
        typedef             size_t                                          IndexType;
        typedef             typename Distances::DistanceType                DistanceType;

                            ExplicitDistances(const Distances& distances);

        DistanceType        operator()(IndexType a, IndexType b) const;

        size_t              size() const                                    { return size_; }
        IndexType           begin() const                                   { return 0; }
        IndexType           end() const                                     { return size(); }

    private:
        std::vector<DistanceType>                   distances_;
        size_t                                      size_;
};

#include "rips.hpp"

#endif // __RIPS_H__
