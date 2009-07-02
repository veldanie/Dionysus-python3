#include <boost/utility.hpp>
#include <queue>
#include <vector>

#include <utilities/log.h>
#include <utilities/indirect.h>

#ifdef LOGGING
static rlog::RLogChannel* rlCohomology =                DEF_CHANNEL("topology/cohomology",        rlog::Log_Debug);
#endif

template<class BirthInfo, class SimplexData>
class CohomologyPersistence<BirthInfo, SimplexData>::CompareSNode
{
    public:
        bool        operator()(const SNode& s1, const SNode& s2) const                  { return s1.si->order < s2.si->order; }
};

template<class BirthInfo, class SimplexData>
template<class BI>
typename CohomologyPersistence<BirthInfo, SimplexData>::IndexDeathPair
CohomologyPersistence<BirthInfo, SimplexData>::
add(BI begin, BI end, BirthInfo birth, const SimplexData& sd)
{
    // Create simplex representation
    simplices_.push_back(SHead(sd, simplices_.empty() ? 0 : (simplices_.back().order + 1)));
    SimplexIndex    si = boost::prior(simplices_.end());

    // Find out if there are cocycles that evaluate to non-zero on the new simplex
    typedef         std::list<CocycleIndex>                                             Candidates;
    Candidates      candidates, candidates_bulk;
    rLog(rlCohomology, "Boundary");
    for (BI cur = begin; cur != end; ++cur)
    {
        rLog(rlCohomology, "  %d", (*cur)->order);
        for (typename ZRow::const_iterator zcur = (*cur)->row.begin(); zcur != (*cur)->row.end(); ++zcur)
            candidates_bulk.push_back(zcur->cocycle);
    }

    candidates_bulk.sort(make_indirect_comparison(std::less<Cocycle>()));
    
    rLog(rlCohomology,  "  Candidates bulk");
    for (typename Candidates::iterator cur  = candidates_bulk.begin(); 
                                       cur != candidates_bulk.end(); ++cur)
        rLog(rlCohomology, "    %d", (*cur)->order);

    // Remove duplicates --- this is really Z_2, we need a more sophisticated
    {
        typename Candidates::const_iterator cur = candidates_bulk.begin();
        while (cur != candidates_bulk.end())
        {
            typename Candidates::const_iterator next = cur;
            unsigned count = 0;
            while (next != candidates_bulk.end() && *next == *cur) { ++next; ++count; }
    
            if (count % 2)
                candidates.push_back(*cur);
    
            cur = next;
        }
    }

    // Birth
    if (candidates.empty())
    {
        rLog(rlCohomology,  "Birth");
        
        unsigned order = cocycles_.empty() ? 0 : cocycles_.front().order + 1;
        cocycles_.push_front(Cocycle(birth, order));

        // set up the cocycle
        ZColumn& cocycle = cocycles_.front().cocycle;
        cocycle.push_back(si);
        cocycle.front().cocycle = cocycles_.begin();
        si->row.push_back(cocycles_.front().cocycle.front());

        return std::make_pair(si, Death());
    }

    // Death
    rLog(rlCohomology,  "Death");

#if 0
    // Debug only, output candidates
    rLog(rlCohomology,  "  Candidates");
    for (typename Candidates::iterator cur  = candidates.begin(); 
                                       cur != candidates.end(); ++cur)
        rLog(rlCohomology, "    %d", (*cur)->order);
#endif

    Cocycle& z          = *candidates.front();
    Death d             = z.birth;

    // add z to everything else in candidates
    for (typename Candidates::iterator cur  = boost::next(candidates.begin()); 
                                       cur != candidates.end(); ++cur)
        add_cocycle(**cur, z);

    for (typename ZColumn::iterator cur = z.cocycle.begin(); cur != z.cocycle.end(); ++cur)
        cur->unlink();
    
    cocycles_.erase(candidates.front());

    return std::make_pair(si, d);
}
        
template<class BirthInfo, class SimplexData>
void
CohomologyPersistence<BirthInfo, SimplexData>::
show_cycles() const
{
    std::cout << "Cocycles" << std::endl;
    for (typename Cocycles::const_iterator cur = cocycles_.begin(); cur != cocycles_.end(); ++cur)
    {
        std::cout << cur->order << ": ";
        for (typename ZColumn::const_iterator zcur = cur->cocycle.begin(); zcur != cur->cocycle.end(); ++zcur)
            std::cout << zcur->si->order << ", ";
        std::cout << std::endl;
    }
}

template<class BirthInfo, class SimplexData>
void
CohomologyPersistence<BirthInfo, SimplexData>::
add_cocycle(Cocycle& to, Cocycle& from)
{
    rLog(rlCohomology,  "Adding cocycle %d to %d", from.order, to.order);

    ZColumn     nw;

    std::set_symmetric_difference(to.cocycle.begin(),   to.cocycle.end(),           // this is catered to Z_2
                                  from.cocycle.begin(), from.cocycle.end(), 
                                  std::back_insert_iterator<ZColumn>(nw), 
                                  CompareSNode());

    for (typename ZColumn::iterator cur = to.cocycle.begin(); cur != to.cocycle.end(); ++cur)
        cur->unlink();

    to.cocycle.swap(nw);

    for (typename ZColumn::iterator cur = to.cocycle.begin(); cur != to.cocycle.end(); ++cur)
    {
        cur->si->row.push_back(*cur);
        cur->cocycle = nw[0].cocycle;
    }
}
