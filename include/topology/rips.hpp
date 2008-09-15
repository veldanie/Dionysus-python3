#include <algorithm>
#include <utility>
#include <boost/utility.hpp>
#include <iostream>
#include <utilities/log.h>

#ifdef LOGGING
static rlog::RLogChannel* rlRips =                  DEF_CHANNEL("rips/info", rlog::Log_Debug);
#endif // LOGGING

#ifdef COUNTERS
static Counter*  cClique =                          GetCounter("rips/clique");
#endif // COUNTERS

template<class Distances_, class Simplex_>
struct Rips<Distances_, Simplex_>::ComparePair
{
                            ComparePair(const Distances& distances): 
                                distances_(distances)                       {}

        bool                operator()(const std::pair<IndexType, IndexType>& a,
                                       const std::pair<IndexType, IndexType>& b)            {  return   distances_(a.first, a.second) <
                                                                                                        distances_(b.first, b.second);  }

        const Distances&    distances_;
};

template<class DistanceType_, class Simplex_>
void
Rips<DistanceType_, Simplex_>::
generate(Dimension k, DistanceType max)
{
    // Order all the edges
    typedef std::vector< std::pair<IndexType, IndexType> >      EdgeVector;
    EdgeVector      edges;
    for (IndexType a = distances_.begin(); a != distances_.end(); ++a)
    {
        Simplex ssx; ssx.add(a);
        simplices.push_back(ssx);
        for (IndexType b = boost::next(a); b != distances_.end(); ++b)
        {
            if (distances_(a,b) <= max)
                edges.push_back(std::make_pair(a,b));
        }
    }
    std::sort(edges.begin(), edges.end(), ComparePair(distances_));

    // Generate simplices
    std::vector<std::vector<size_t> >       vertex_star(distances_.size());
    for(typename EdgeVector::const_iterator cur = edges.begin(); cur != edges.end(); ++cur)
    {
        rLog(rlRips, "Current edge: %d %d", cur->first, cur->second);

        // Create the edge
        Simplex edge; edge.add(cur->first); edge.add(cur->second);
        simplices.push_back(edge);
        vertex_star[cur->first].push_back(simplices.size() - 1); 
        vertex_star[cur->second].push_back(simplices.size() - 1);

        // Go through a star
        size_t sz = vertex_star[cur->first].size() - 1;
        for (size_t i = 0; i < sz; ++i)
        {
            const Simplex& ssx = simplices[vertex_star[cur->first][i]];
            rLog(rlRips, "  %s", tostring(ssx).c_str());
            bool accept = true;
            for (typename Simplex::VertexContainer::const_iterator v = ssx.vertices().begin(); v != ssx.vertices().end(); ++v)
            {
                if (*v == cur->first) continue;
                
                if (  distances_(*v, cur->second) >  distances_(cur->first, cur->second) ||
                    ((distances_(*v, cur->second) == distances_(cur->first, cur->second)) && 
                     (*v > cur->first)))
                {
                    accept = false;
                    break;
                }
            }
            if (accept)
            {
                Simplex tsx(ssx); tsx.add(cur->second);
                simplices.push_back(tsx);
                rLog(rlRips, "  Accepting: %s", tostring(tsx).c_str());
         
                // Update stars
                if (tsx.dimension() < k)
                    for (typename Simplex::VertexContainer::const_iterator v = tsx.vertices().begin(); v != tsx.vertices().end(); ++v)
                        vertex_star[*v].push_back(simplices.size() - 1);
            }
        }
    }
}

template<class Distances_, class Simplex_>
void
Rips<Distances_, Simplex_>::
print() const
{
    for (typename SimplexVector::const_iterator cur = simplices.begin(); cur != simplices.end(); ++cur)
        std::cout << *cur << std::endl;
}
