#include <algorithm>
#include <utility>
#include <boost/utility.hpp>
#include <iostream>
#include <utilities/log.h>
#include <utilities/counter.h>

#ifdef LOGGING
static rlog::RLogChannel* rlRips =                  DEF_CHANNEL("rips/info", rlog::Log_Debug);
static rlog::RLogChannel* rlRipsDebug =             DEF_CHANNEL("rips/debug", rlog::Log_Debug);
#endif // LOGGING

#ifdef COUNTERS
static Counter*  cClique =                          GetCounter("rips/clique");
#endif // COUNTERS

template<class Distances_, class Simplex_>
struct RipsBase<Distances_, Simplex_>::ComparePair
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
RipsGenerator<DistanceType_, Simplex_>::
generate(SimplexVector& simplices, Dimension k, DistanceType max) const
{
    // Order all the edges
    typedef std::vector< std::pair<IndexType, IndexType> >      EdgeVector;
    EdgeVector      edges;
    for (IndexType a = distances().begin(); a != distances().end(); ++a)
    {
        Simplex ssx; ssx.add(a);
        simplices.push_back(ssx);
        for (IndexType b = boost::next(a); b != distances().end(); ++b)
        {
            if (distances()(a,b) <= max)
                edges.push_back(std::make_pair(a,b));
        }
    }
    std::sort(edges.begin(), edges.end(), ComparePair(distances()));

    // Generate simplices
    std::vector<std::vector<size_t> >       vertex_star(distances().size());
    for(typename EdgeVector::const_iterator cur = edges.begin(); cur != edges.end(); ++cur)
    {
        rLog(rlRipsDebug, "Current edge: %d %d", cur->first, cur->second);

        // Create the edge
        Simplex edge; edge.add(cur->first); edge.add(cur->second);
        simplices.push_back(edge);
        if (k <= 1) continue;

        vertex_star[cur->first].push_back(simplices.size() - 1); 
        vertex_star[cur->second].push_back(simplices.size() - 1);

        // Go through a star
        size_t sz = vertex_star[cur->first].size() - 1;
        for (size_t i = 0; i < sz; ++i)
        {
            const Simplex& ssx = simplices[vertex_star[cur->first][i]];
            // FIXME: eventually can uncomment, missing Empty::operator<<()  
            // rLog(rlRipsDebug, "  %s", tostring(ssx).c_str());
            bool accept = true;
            for (typename Simplex::VertexContainer::const_iterator v = ssx.vertices().begin(); v != ssx.vertices().end(); ++v)
            {
                if (*v == cur->first) continue;
                
                if (  distances()(*v, cur->second) >  distances()(cur->first, cur->second) ||
                    ((distances()(*v, cur->second) == distances()(cur->first, cur->second)) && 
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
                // rLog(rlRipsDebug, "  Accepting: %s", tostring(tsx).c_str());
         
                // Update stars
                if (tsx.dimension() < k - 1)
                    for (typename Simplex::VertexContainer::const_iterator v =  static_cast<const Simplex&>(tsx).vertices().begin(); 
                                                                           v != static_cast<const Simplex&>(tsx).vertices().end(); 
                                                                           ++v)
                        vertex_star[*v].push_back(simplices.size() - 1);
            }
        }
    }
}

template<class DistanceType_, class Simplex_>
void
RipsGeneratorMemory<DistanceType_, Simplex_>::
generate(SimplexVector& simplices, Dimension k, DistanceType max) const
{
    for (IndexType v = distances().begin(); v != distances().end(); ++v)
    {
        simplices.push_back(Simplex());
        simplices.back().add(v);
    }
    size_t last_vertex = simplices.size() - 1;
    size_t begin_previous_dimension = 0;
    size_t end_previous_dimension = simplices.size() - 1;
    typename Simplex::VertexComparison vcmp;

    for (Dimension d = 1; d < k; ++d)
    {
        //rLog(rlRips, "Generating dimension %d", d);
        //rLog(rlRips, "  Begin previous dimension: %d", begin_previous_dimension);
        //rLog(rlRips, "  End previous dimension:   %d", end_previous_dimension);
        for (size_t i = 0; i <= last_vertex; ++i)
        {
            for (size_t j = begin_previous_dimension; j <= end_previous_dimension; ++j)
                if (!simplices[j].contains(simplices[i]) &&
                     vcmp(simplices[i], simplices[j]) && 
                     distance(simplices[i], simplices[j]) <= max)
                {
                    simplices.push_back(Simplex(simplices[j]));
                    simplices.back().join(simplices[i]);
                }
        }
        begin_previous_dimension = end_previous_dimension + 1;
        end_previous_dimension = simplices.size() - 1;
    }
}

template<class Distances_, class Simplex_>
typename RipsBase<Distances_, Simplex_>::DistanceType
RipsBase<Distances_, Simplex_>::
distance(const Simplex& s1, const Simplex& s2) const
{
    DistanceType mx = 0;
    for (typename Simplex::VertexContainer::const_iterator      a = s1.vertices().begin();   a != s1.vertices().end();    ++a)
        for (typename Simplex::VertexContainer::const_iterator  b = s2.vertices().begin();   b != s2.vertices().end();    ++b)
            mx = std::max(mx, distances_(*a,*b));
    return mx;
}

template<class Distances_, class Simplex_>
typename RipsBase<Distances_, Simplex_>::DistanceType
RipsBase<Distances_, Simplex_>::
max_distance() const
{
    DistanceType mx = 0;
    for (IndexType a = distances_.begin(); a != distances_.end(); ++a)
        for (IndexType b = boost::next(a); b != distances_.end(); ++b)
            mx = std::max(mx, distances_(a,b));
    return mx;
}

template<class Distances_, class Simplex_>
typename RipsBase<Distances_, Simplex_>::DistanceType
RipsBase<Distances_, Simplex_>::Evaluator::
operator()(const Simplex& s) const
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
