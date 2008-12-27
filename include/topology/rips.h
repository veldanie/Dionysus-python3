#ifndef __RIPS_H__
#define __RIPS_H__

#include <vector>
#include <string>
#include "simplex.h"

/**
 * RipsBase class
 *
 * Base class for the generator of Rips complexes.
 *
 * Distances_ is expected to define types IndexType and DistanceType as well as 
 *               provide operator()(...) which given two IndexTypes should return 
 *               the distance between them. There should be methods begin() and end() 
 *               for iterating over IndexTypes as well as a method size().
 */
template<class Distances_, class Simplex_ = Simplex<typename Distances_::IndexType> >
class RipsBase
{
    public:
        typedef             Distances_                                      Distances; 
        typedef             typename Distances::IndexType                   IndexType;
        typedef             typename Distances::DistanceType                DistanceType;

        typedef             Simplex_                                        Simplex;
        typedef             std::vector<Simplex>                            SimplexVector;

        class               Evaluator;
        class               Comparison;
        struct              ComparePair;

    public:
                            RipsBase(const Distances& distances): 
                                distances_(distances)                       {}

        const Distances&    distances() const                               { return distances_; }
        DistanceType        max_distance() const;
        
        DistanceType        distance(const Simplex& s1, const Simplex& s2) const;

    private:
        const Distances&    distances_;
};
        
template<class Distances_, class Simplex_ = Simplex<typename Distances_::IndexType> >
class RipsGenerator: public RipsBase<Distances_, Simplex_>
{
    public:
        typedef             RipsBase<Distances_, Simplex_>                  Parent;
        typedef             typename Parent::Distances                      Distances;
        typedef             typename Parent::Simplex                        Simplex;
        typedef             typename Parent::SimplexVector                  SimplexVector;
        typedef             typename Parent::DistanceType                   DistanceType;
        typedef             typename Parent::IndexType                      IndexType;
        typedef             typename Parent::ComparePair                    ComparePair;

                            RipsGenerator(const Distances& distances):
                                Parent(distances)                           {}

        using               Parent::distances;

        /// generate k-skeleton of the Rips complex
        void                generate(SimplexVector& v, Dimension k, DistanceType max) const;
};

// Much more memory efficient, but also much slower
template<class Distances_, class Simplex_ = Simplex<typename Distances_::IndexType> >
class RipsGeneratorMemory: public RipsBase<Distances_, Simplex_>
{
    public:
        typedef             RipsBase<Distances_, Simplex_>                  Parent;
        typedef             typename Parent::Distances                      Distances;
        typedef             typename Parent::Simplex                        Simplex;
        typedef             typename Parent::SimplexVector                  SimplexVector;
        typedef             typename Parent::DistanceType                   DistanceType;
        typedef             typename Parent::IndexType                      IndexType;
        typedef             typename Parent::ComparePair                    ComparePair;

                            RipsGeneratorMemory(const Distances& distances):
                                Parent(distances)                           {}

        using               Parent::distances;
        using               Parent::distance;

        /// generate k-skeleton of the Rips complex
        void                generate(SimplexVector& v, Dimension k, DistanceType max) const;
};


template<class Distances_, class Simplex_>
class RipsBase<Distances_, Simplex_>::Evaluator
{
    public:
        typedef             Simplex_                                        Simplex;

                            Evaluator(const Distances& distances): 
                                distances_(distances)                       {}

        DistanceType        operator()(const Simplex& s) const;

    private:
        const Distances&    distances_;
};

template<class Distances_, class Simplex_>
class RipsBase<Distances_, Simplex_>::Comparison
{
    public:
        typedef             Simplex_                                        Simplex;

                            Comparison(const Distances& distances):
                                eval_(distances)                            {}

        bool                operator()(const Simplex& s1, const Simplex& s2) const    { return eval_(s1) < eval_(s2); }

    private:
        Evaluator           eval_;
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
