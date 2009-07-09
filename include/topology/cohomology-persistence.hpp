#include <boost/utility.hpp>
#include <queue>
#include <vector>

#include <utilities/log.h>
#include <utilities/indirect.h>

#ifdef LOGGING
static rlog::RLogChannel* rlCohomology =                DEF_CHANNEL("topology/cohomology",        rlog::Log_Debug);
#endif

template<class BirthInfo, class SimplexData, class Field>
class CohomologyPersistence<BirthInfo, SimplexData, Field>::CompareSNode
{
    public:
        bool        operator()(const SNode& s1, const SNode& s2) const                  { return s1.si->order < s2.si->order; }
};

template<class BirthInfo, class SimplexData, class Field>
template<class BI>
typename CohomologyPersistence<BirthInfo, SimplexData, Field>::IndexDeathPair
CohomologyPersistence<BirthInfo, SimplexData, Field>::
add(BI begin, BI end, BirthInfo birth, bool store, const SimplexData& sd)
{
    // Create simplex representation
    simplices_.push_back(SHead(sd, simplices_.empty() ? 0 : (simplices_.back().order + 1)));
    SimplexIndex    si = boost::prior(simplices_.end());

    // Find out if there are cocycles that evaluate to non-zero on the new simplex
    typedef         std::list<CocycleCoefficientPair>           Candidates;
    Candidates      candidates, candidates_bulk;
    rLog(rlCohomology, "Boundary");
    bool sign = true;       // TODO: this is very crude, fix later
    for (BI cur = begin; cur != end; ++cur)
    {
        rLog(rlCohomology, "  %d %d", (*cur)->order, sign);
        for (typename ZRow::const_iterator zcur = (*cur)->row.begin(); zcur != (*cur)->row.end(); ++zcur)
            candidates_bulk.push_back(std::make_pair(zcur->ci, 
                                                     (sign ? zcur->coefficient : field_.neg(zcur->coefficient))));
        sign = !sign;
    }

    candidates_bulk.sort(make_first_comparison(make_indirect_comparison(std::less<Cocycle>())));
    
#if LOGGING    
    rLog(rlCohomology,  "  Candidates bulk");
    for (typename Candidates::iterator cur  = candidates_bulk.begin(); 
                                       cur != candidates_bulk.end(); ++cur)
        rLog(rlCohomology, "    %d %d", cur->first->order, cur->second);
#endif

    // Remove duplicates
    {
        typename Candidates::const_iterator cur = candidates_bulk.begin();
        while (cur != candidates_bulk.end())
        {
            typename Candidates::const_iterator next = cur;
            FieldElement sum = field_.zero();
            while (next != candidates_bulk.end() && next->first == cur->first) 
            { 
                sum = field_.add(sum, next->second);
                ++next; 
            }
    
            rLog(rlCohomology, "  Bulk candidate %d sum %d", cur->first->order, sum);
            if (!field_.is_zero(sum))
                candidates.push_back(CocycleCoefficientPair(cur->first, sum));
    
            cur = next;
        }
    }

    // Birth
    if (candidates.empty())
    {
        if (!store)
        {
            simplices_.pop_back();
            return std::make_pair(simplices_.begin(), Death());         // TODO: shouldn't return front
        }
        
        unsigned order = cocycles_.empty() ? 0 : cocycles_.front().order + 1;
        cocycles_.push_front(Cocycle(birth, order));
        
        rLog(rlCohomology,  "Birth: %d", cocycles_.front().order);

        // set up the cocycle
        ZColumn& cocycle = cocycles_.front().zcolumn;
        cocycle.push_back(SNode(si, field_.id(), cocycles_.begin()));
        si->row.push_back(cocycles_.front().zcolumn.front());
        rLog(rlCohomology,  "  Cocyle: %d", si->order);

        return std::make_pair(si, Death());
    }

    // Death
    rLog(rlCohomology,  "Death");

#if LOGGING
    // Debug only, output candidates
    rLog(rlCohomology,  "  Candidates");
    for (typename Candidates::iterator cur  = candidates.begin(); cur != candidates.end(); ++cur)
        rLog(rlCohomology, "    %d %d", cur->first->order, cur->second);
#endif

    CocycleCoefficientPair& z   = candidates.front();
    Death d                     = z.first->birth;

    // add z to everything else in candidates
    for (typename Candidates::iterator cur  = boost::next(candidates.begin()); 
                                       cur != candidates.end(); ++cur)
        add_cocycle(*cur, z);

    for (typename ZColumn::iterator cur = z.first->zcolumn.begin(); cur != z.first->zcolumn.end(); ++cur)
        cur->unlink();
    
    cocycles_.erase(candidates.front().first);

    return std::make_pair(si, d);
}
        
template<class BirthInfo, class SimplexData, class Field>
void
CohomologyPersistence<BirthInfo, SimplexData, Field>::
show_cocycles() const
{
    std::cout << "Cocycles: " << cocycles_.size() << std::endl;
    for (typename Cocycles::const_iterator cur = cocycles_.begin(); cur != cocycles_.end(); ++cur)
    {
        // std::cout << cur->order << " (" << cur->birth << "): ";
        for (typename ZColumn::const_iterator zcur = cur->zcolumn.begin(); zcur != cur->zcolumn.end(); ++zcur)
            std::cout << zcur->coefficient << " * " << zcur->si->order << ", ";
        std::cout << std::endl;
    }
}

template<class BirthInfo, class SimplexData, class Field>
void
CohomologyPersistence<BirthInfo, SimplexData, Field>::
add_cocycle(CocycleCoefficientPair& to, CocycleCoefficientPair& from)
{
    rLog(rlCohomology,  "Adding cocycle %d to %d", from.first->order, to.first->order);

    ZColumn         nw;
    FieldElement    multiplier = field_.neg(field_.div(to.second, from.second));
    CocycleIndex    ci = to.first;

    typename ZColumn::iterator tcur = to.first->zcolumn.begin();
    typename ZColumn::iterator fcur = from.first->zcolumn.begin();
    CompareSNode cmp;
    while (tcur != to.first->zcolumn.end() && fcur != from.first->zcolumn.end())
    {
        rLog(rlCohomology, "  %d %d", tcur->si->order, fcur->si->order);
        if (cmp(*tcur, *fcur))
        {
            nw.push_back(*tcur);
            ++tcur;
        }
        else if (cmp(*fcur, *tcur))
        {
            nw.push_back(SNode(fcur->si, field_.mul(multiplier, fcur->coefficient), ci));
            ++fcur;
        }
        else        // equality
        {
            FieldElement res = field_.mul(multiplier, tcur->coefficient);
            res = field_.add(fcur->coefficient, res);
            if (!field_.is_zero(res))
                nw.push_back(SNode(fcur->si, res, ci));
            ++tcur; ++fcur;
        }
    }
    for (; tcur != to.first->zcolumn.end(); ++tcur)
    {
        rLog(rlCohomology, "  %d", tcur->si->order);
        nw.push_back(SNode(*tcur));
    }
    for (; fcur != from.first->zcolumn.end(); ++fcur)
    {
        rLog(rlCohomology, "  %d", fcur->si->order);
        nw.push_back(SNode(fcur->si, field_.mul(multiplier, fcur->coefficient), ci));
    }


    for (typename ZColumn::iterator cur = to.first->zcolumn.begin(); cur != to.first->zcolumn.end(); ++cur)
        cur->unlink();

    to.first->zcolumn.swap(nw);

    for (typename ZColumn::iterator cur = to.first->zcolumn.begin(); cur != to.first->zcolumn.end(); ++cur)
        cur->si->row.push_back(*cur);
}
