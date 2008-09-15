#ifndef __RIPS_H__
#define __RIPS_H__

#include <vector>
#include <string>
#include <topology/simplex.h>

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

    public:
                            Rips(const Distances& distances): 
                                distances_(distances)                       {}

        void                generate(Dimension k, DistanceType max);        /// generate k-skeleton of the Rips complex

        void                print() const;
        size_t              size() const                                    { return simplices.size(); }

    private:
        struct              ComparePair;

    private:
        const Distances&    distances_;

        SimplexVector       simplices;

};

#include "rips.hpp"

#endif // __RIPS_H__
