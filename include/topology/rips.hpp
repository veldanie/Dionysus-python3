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
        simplices_.push_back(ssx);
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
        simplices_.push_back(edge);
        if (k <= 1) continue;

        vertex_star[cur->first].push_back(simplices_.size() - 1); 
        vertex_star[cur->second].push_back(simplices_.size() - 1);

        // Go through a star
        size_t sz = vertex_star[cur->first].size() - 1;
        for (size_t i = 0; i < sz; ++i)
        {
            const Simplex& ssx = simplices_[vertex_star[cur->first][i]];
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
                simplices_.push_back(tsx);
                rLog(rlRips, "  Accepting: %s", tostring(tsx).c_str());
         
                // Update stars
                if (tsx.dimension() < k - 1)
                    for (typename Simplex::VertexContainer::const_iterator v = tsx.vertices().begin(); v != tsx.vertices().end(); ++v)
                        vertex_star[*v].push_back(simplices_.size() - 1);
            }
        }
    }
}

template<class Distances_, class Simplex_>
void
Rips<Distances_, Simplex_>::
print() const
{
    for (typename SimplexVector::const_iterator cur = simplices_.begin(); cur != simplices_.end(); ++cur)
        std::cout << *cur << std::endl;
}

template<class Distances_, class Simplex_>
typename Rips<Distances_, Simplex_>::DistanceType
Rips<Distances_, Simplex_>::
max_distance() const
{
    DistanceType mx = 0;
    for (IndexType a = distances_.begin(); a != distances_.end(); ++a)
        for (IndexType b = boost::next(a); b != distances_.end(); ++b)
            mx = std::max(mx, distances_(a,b));
    return mx;
}

template<class Distances_, class Simplex_>
typename Rips<Distances_, Simplex_>::DistanceType
Rips<Distances_, Simplex_>::Evaluator::
value(const Simplex& s) const
{
    DistanceType mx = 0;
    for (typename Simplex::VertexContainer::const_iterator      a = s.vertices().begin();   a != s.vertices().end();    ++a)
        for (typename Simplex::VertexContainer::const_iterator  b = boost::next(a);         b != s.vertices().end();    ++b)
            mx = std::max(mx, distances_(*a,*b));
    return mx;
}

template<class Distances_>
ExplicitDistances<Distances_>::
ExplicitDistances(const Distances& distances): 
    size_(distances.size()), distances_((distances.size() * (distances.size() + 1))/2)
{
    IndexType i = 0;
    for (typename Distances::IndexType a = distances.begin(); a != distances.end(); ++a)
        for (typename Distances::IndexType b = a; b != distances.end(); ++b)
        {
            distances_[i++] = distances(a,b);
        }
}

template<class Distances_>
typename ExplicitDistances<Distances_>::DistanceType
ExplicitDistances<Distances_>::
operator()(IndexType a, IndexType  b) const
{
    if (a > b) std::swap(a,b);
    return distances_[a*size_ - ((a*(a-1))/2) + (b-a)];
}
