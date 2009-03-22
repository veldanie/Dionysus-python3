#include <utilities/log.h>
#include <utilities/containers.h>
#include <boost/utility/enable_if.hpp>
#include <boost/utility.hpp>
#include <utilities/property-maps.h>

#ifdef LOGGING
static rlog::RLogChannel* rlPersistence =                   DEF_CHANNEL("topology/persistence", rlog::Log_Debug);
#endif // LOGGING

#ifdef COUNTERS
static Counter*  cPersistencePair =                         GetCounter("persistence/pair");
static Counter*  cPersistencePairBoundaries =               GetCounter("persistence/pair/boundaries");
static Counter*  cPersistencePairCycleLength =              GetCounter("persistence/pair/cyclelength");
#endif // COUNTERS

template<class D, class OD>
template<class Filtration>
StaticPersistence<D, OD>::
StaticPersistence(const Filtration& filtration, 
                  const OrderComparison& ocmp):
    order_(filtration.size()),
    ocmp_(ocmp)
{ 
    OrderIndex                          ocur = begin();
    OffsetMap<typename Filtration::IntermediateIndex, OrderIndex>       om(0, ocur);            // TODO: this is customized for std::vector Order
    for (typename Filtration::Index cur = filtration.begin(); cur != filtration.end(); ++cur, ++ocur)
    {
        // Convert the Filtration::IndexBoundary into a Cycle, and 
        typedef typename OrderDescriptor::Chains::Cycle         Cycle;

        OrderElement& oe = *ocur;
        oe.pair = ocur;

        filtration.boundary(cur, oe.cycle, om);
        oe.cycle.sort(ocmp_); 
    }
}

template<class D, class OD>
template<class Visitor>
void 
StaticPersistence<D, OD>::
pair_simplices(OrderIndex bg, OrderIndex end, const Visitor& visitor)
{
#if LOGGING
    typename Traits::OutputMap outmap(order_);
#endif

    // FIXME: need sane output for logging
    rLog(rlPersistence, "Entered: pair_simplices");
    for (OrderIndex j = bg; j != end; ++j)
    {
        visitor.init(j);
        rLog(rlPersistence, "Simplex %s", outmap(j).c_str());

        OrderElement& oe = *j;
        typedef typename OrderDescriptor::Chains::Cycle         Cycle;
        Cycle& z = oe.cycle;
        rLog(rlPersistence, "  has boundary: %s", z.tostring(outmap).c_str());
        
        CountNum(cPersistencePairBoundaries, oe.cycle.size());
        Count(cPersistencePair);

        while(!z.empty())
        {
            OrderIndex i = z.top(ocmp_);            // take the youngest element with respect to the OrderComparison
            rLog(rlPersistence, "  %s: %s", outmap(i).c_str(), outmap(i->pair).c_str());
            // TODO: is this even a meaningful assert?
            AssertMsg(!ocmp_(i, j), 
                      "Simplices in the cycle must precede current simplex: (%s in cycle of %s)",
                      outmap(i).c_str(), outmap(j).c_str());

            // i is not paired, so we pair j with i
            if (i->pair == i)
            {
                rLog(rlPersistence, "  Pairing %s and %s with cycle %s", 
                                   outmap(i).c_str(), outmap(j).c_str(), 
                                   z.tostring(outmap).c_str());
                i->pair = j;
                j->pair = i;
                CountNum(cPersistencePairCycleLength, z.size());
                CountBy(cPersistencePairCycleLength, z.size());
                break;
            }

            // update element
            z.add(i->pair->cycle, ocmp_);
            visitor.update(j, i);
            rLog(rlPersistence, "    new cycle: %s", z.tostring(outmap).c_str());
        }
        visitor.finished(j);
        rLog(rlPersistence, "Finished with %s: %s", 
                            outmap(j).c_str(), outmap(oe.pair).c_str());
    }
}


/* Private */
