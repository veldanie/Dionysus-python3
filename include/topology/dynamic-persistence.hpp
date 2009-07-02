#include <utilities/log.h>

#ifdef LOGGING
static rlog::RLogChannel* rlTranspositions =    DEF_CHANNEL("topology/persistence/transpositions", rlog::Log_Debug);
#endif // LOGGING

#ifdef COUNTERS
static Counter*  cTransposition =               GetCounter("persistence/transposition");
static Counter*  cTranspositionDiffDim =        GetCounter("persistence/transposition/diffdim");
static Counter*  cTranspositionCase12 =         GetCounter("persistence/transposition/case/1/2");
static Counter*  cTranspositionCase12s =        GetCounter("persistence/transposition/case/1/2/special");
static Counter*  cTranspositionCase112 =        GetCounter("persistence/transposition/case/1/1/2");
static Counter*  cTranspositionCase111 =        GetCounter("persistence/transposition/case/1/1/1");
static Counter*  cTranspositionCase22 =         GetCounter("persistence/transposition/case/2/2");
static Counter*  cTranspositionCase212 =        GetCounter("persistence/transposition/case/2/1/2");
static Counter*  cTranspositionCase211 =        GetCounter("persistence/transposition/case/2/1/1");
static Counter*  cTranspositionCase32 =         GetCounter("persistence/transposition/case/3/2");
static Counter*  cTranspositionCase31 =         GetCounter("persistence/transposition/case/3/1");
static Counter*  cTranspositionCase4 =          GetCounter("persistence/transposition/case/4");
#endif // COUNTERS


/* Trails */

template<class D, class CT, class Cmp, class OT, class CI, class CC, class E>
template<class Filtration>
DynamicPersistenceTrails<D,CT,Cmp,OT,CI,CC,E>::
DynamicPersistenceTrails(const Filtration& f, const OrderComparison& ocmp, const ConsistencyComparison& ccmp):
    Parent(f, ocmp), ccmp_(ccmp)
{}
        
template<class D, class CT, class Cmp, class OT, class CI, class CC, class E>
void
DynamicPersistenceTrails<D,CT,Cmp,OT,CI,CC,E>::
pair_simplices()
{ 
    Parent::pair_simplices(begin(), end(), PairingTrailsVisitor(begin(), ccmp_));
}

template<class D, class CT, class Cmp, class OT, class CI, class CC, class E>
template<class Visitor>
bool
DynamicPersistenceTrails<D,CT,Cmp,OT,CI,CC,E>::
transpose(OrderIndex i, const Visitor& visitor)
{
#if LOGGING
    typename Traits::OutputMap outmap(order());
#endif

    Count(cTransposition);
    typedef                 OrderIndex                                  Index;
    typedef                 typename Element::Trail::iterator           TrailIterator;

    visitor.transpose(i);
    
    Index i_prev = i++;

#if 0       // Persistence no longer has the notion of dimension
    if (i_prev->dimension() != i->dimension())
    {
        swap(i_prev, i);
        rLog(rlTranspositions, "Different dimension");
        Count(cTranspositionDiffDim);
        return false;
    }
#endif
    
    bool si = i_prev->sign(), sii = i->sign();
    if (si && sii)
    {
        rLog(rlTranspositions, "Trail prev: %s", i_prev->trail.tostring(outmap).c_str());

        // Case 1
        boost::optional<TrailIterator> i_in_i_prev = i_prev->trail.contains(i);
        if (i_in_i_prev)
        {
            rLog(rlTranspositions, "Case 1, U[i,i+1] = 1");
            i_prev->trail.remove(*i_in_i_prev);
        }

        Index k = i_prev->pair;
        Index l = i->pair;

        // Explicit treatment of unpaired simplex
        if (l == i)
        {
            swap(i_prev, i);
            rLog(rlTranspositions, "Case 1.2 --- unpaired");
            rLog(rlTranspositions, "%s", outmap(i_prev).c_str());
            Count(cTranspositionCase12);
            return false;
        } else if (k == i_prev)
        {
            if (!(l->cycle.contains(i_prev)))
            {
                // Case 1.2
                swap(i_prev, i);
                rLog(rlTranspositions, "Case 1.2 --- unpaired");
                rLog(rlTranspositions, outmap(i_prev).c_str());
                Count(cTranspositionCase12);
                return false;
            } else
            {
                // Case 1.2 --- special version (plain swap, but pairing switches)
                swap(i_prev, i);
                pairing_switch(i_prev, i);
                visitor.switched(i_prev, Case12);
                rLog(rlTranspositions, "Case 1.2 --- unpaired (pairing switch)");
                rLog(rlTranspositions, outmap(i_prev).c_str());
                Count(cTranspositionCase12s);
                return true;
            }
        }
        
        rLog(rlTranspositions, "l cycle: %s", l->cycle.tostring(outmap).c_str());
        if (!(l->cycle.contains(i_prev)))
        {
            // Case 1.2
            swap(i_prev, i);
            rLog(rlTranspositions, "Case 1.2");
            Count(cTranspositionCase12);
            return false;
        } else
        {
            // Case 1.1
            if (not2(ccmp_)(k,l))
            {
                // Case 1.1.1
                swap(i_prev, i);
                l->cycle.add(k->cycle, ccmp_);        // Add column k to l
                k->trail.add(l->trail, ccmp_);        // Add row l to k
                rLog(rlTranspositions, "Case 1.1.1");
                Count(cTranspositionCase111);
                return false;
            } else
            {
                // Case 1.1.2
                swap(i_prev, i);
                k->cycle.add(l->cycle, ccmp_);        // Add column l to k
                l->trail.add(k->trail, ccmp_);        // Add row k to l
                pairing_switch(i_prev, i);
                visitor.switched(i_prev, Case112);
                rLog(rlTranspositions, "Case 1.1.2");
                Count(cTranspositionCase112);
                return true;
            }
        }
    } else if (!si && !sii)
    {
        // Case 2
        if (!(i_prev->trail.contains(i)))
        {
            // Case 2.2
            swap(i_prev, i);
            rLog(rlTranspositions, "Case 2.2");
            Count(cTranspositionCase22);
            return false;
        } else
        {
            // Case 2.1
            Index low_i = i_prev->pair;
            Index low_ii = i->pair;
            i_prev->trail.add(i->trail, ccmp_);            // Add row i to i_prev
            i->cycle.add(i_prev->cycle, ccmp_);            // Add column i_prev to i
            swap(i_prev, i);    
            if (not2(ccmp_)(low_ii, low_i))
            {
                // Case 2.1.2
                i_prev->cycle.add(i->cycle, ccmp_);        // Add column i to i_prev (after transposition)
                i->trail.add(i_prev->trail, ccmp_);        // Add row i to i_prev
                pairing_switch(i_prev, i);
                visitor.switched(i_prev, Case212);
                rLog(rlTranspositions, "Case 2.1.2");
                Count(cTranspositionCase212);
                return true;
            } 
            
            // Case 2.1.1
            rLog(rlTranspositions, "Case 2.1.1");
            Count(cTranspositionCase211);
            return false;
        }
    } else if (!si && sii)
    {
        // Case 3
        if (!(i_prev->trail.contains(i)))
        {
            // Case 3.2
            swap(i_prev, i);
            rLog(rlTranspositions, "Case 3.2");
            Count(cTranspositionCase32);
            return false;
        } else
        {
            // Case 3.1
            i_prev->trail.add(i->trail, ccmp_);            // Add row i to i_prev
            i->cycle.add(i_prev->cycle, ccmp_);            // Add column i_prev to i
            swap(i_prev, i);
            i_prev->cycle.add(i->cycle, ccmp_);            // Add column i_prev to i (after transposition)
            i->trail.add(i_prev->trail, ccmp_);            // Add row i to i_prev
            pairing_switch(i_prev, i);
            visitor.switched(i_prev, Case31);
            rLog(rlTranspositions, "Case 3.1");
            Count(cTranspositionCase31);
            return true;
        }
    } else if (si && !sii)
    {
        // Case 4
        boost::optional<TrailIterator> i_in_i_prev = i_prev->trail.contains(i);
        if (i_in_i_prev)
        {
            rLog(rlTranspositions, "Case 4, U[i,i+1] = 1");
            i_prev->trail.remove(*i_in_i_prev);
        }
        swap(i_prev, i);
        rLog(rlTranspositions, "Case 4");
        Count(cTranspositionCase4);
        return false;
    }
    
    return false; // to avoid compiler complaints; we should never reach this point
}

template<class D, class CT, class Cmp, class OT, class CI, class CC, class E>
void
DynamicPersistenceTrails<D,CT,Cmp,OT,CI,CC,E>::
swap(OrderIndex i, OrderIndex j)
{
    std::swap<Data>(*i, *j);
    std::swap(i->pair, j->pair);

    std::swap(i->cycle, j->cycle);          // TODO: double-check that the STL container specializations actually get invoked
    std::swap(i->trail, j->trail);
}

template<class D, class CT, class Cmp, class OT, class CI, class CC, class E>
void
DynamicPersistenceTrails<D,CT,Cmp,OT,CI,CC,E>::
pairing_switch(OrderIndex i, OrderIndex j)
{
    OrderIndex i_pair = i->pair;
    OrderIndex j_pair = j->pair;

    if (i_pair == i)
        j->pair = j;
    else
    {
        j->pair = i_pair;
        i_pair->pair = j;
    }

    if (j_pair == j)
        i->pair = i;
    else
    {
        i->pair = j_pair;
        j_pair->pair = i;
    }
}


/* Chains */

template<class D, class CT, class Cmp, class OT, class CI, class CC, class E>
template<class Filtration>
DynamicPersistenceChains<D,CT,Cmp,OT,CI,CC,E>::
DynamicPersistenceChains(const Filtration& f, const OrderComparison& ocmp, const ConsistencyComparison& ccmp):
    Parent(f, ocmp), ccmp_(ccmp)
{}
        
template<class D, class CT, class Cmp, class OT, class CI, class CC, class E>
void
DynamicPersistenceChains<D,CT,Cmp,OT,CI,CC,E>::
pair_simplices()
{ 
    Parent::pair_simplices(begin(), end(), PairingChainsVisitor(begin(), ccmp_, size()));
}

