/*
 * Author: Dmitriy Morozov
 * Department of Computer Science, Duke University, 2005 -- 2008
 */

#ifndef __LOWERSTARFILTRATION_H__
#define __LOWERSTARFILTRATION_H__

#include <boost/serialization/access.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/nvp.hpp>

/**
 * Struct: MaxVertexComparison
 *
 * Functor that determines which simplex has a higher vertex with respect to VertexComparison_
 */
template<class Simplex_, class VertexComparison_>
struct MaxVertexComparison
{
    typedef                     VertexComparison_                                   VertexComparison;
    typedef                     Simplex_                                            Simplex;

                                MaxVertexComparison(const VertexComparison& vcmp):
                                    vcmp_(vcmp)                                     {}

    bool                        operator()(const Simplex& s1, const Simplex& s2) const
    {
        return std::max_element(s1.vertices().begin(), s1.vertices().end(), vcmp) <
               std::max_element(s2.vertices().begin(), s2.vertices().end(), vcmp);
    }

    VertexComparison            vcmp_;
};


/**
 * Map from i-th vertex to its index in the filtration.
 */
template<class Index_>
class VertexSimplexMap
{
    public:
        typedef                 Index_                                              Index;
        typedef                 std::vector<FiltrationIndex>                        VertexVector;
                                
                                VertexSimplexMap(Index begin, Index end, const Map& m)
        {
            for (FiltrationIndex cur = begin; cur != end; ++cur)
                if (m[cur].dimension() == 0)
                    vertices_.push_back(cur);
        }

    private:
        VertexVector            vertices_;
};

// TODO: transpose_vertices(Index, Filtration, Persistence, Visitor);

#endif // __LOWERSTARFILTRATION_H__
