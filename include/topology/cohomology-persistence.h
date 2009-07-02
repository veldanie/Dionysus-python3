#ifndef __COHOMOLOGY_PERSISTENCE_H__
#define __COHOMOLOGY_PERSISTENCE_H__

#if DEBUG_CONTAINERS
    #include <debug/list>
    #include <debug/vector>
    namespace s = std::__debug;
    #warning "Using debug/list and debug/vector in CohomologyPersistence"
#else
    #include <list>
    #include <vector>
    namespace s = std;
#endif

#include <vector>
#include <list>
#include <utility>

#include "utilities/types.h"

#include <boost/optional.hpp>
#include <boost/intrusive/list.hpp>
namespace bi = boost::intrusive;


template<class BirthInfo_, class SimplexData_ = Empty<> >
class CohomologyPersistence
{
    public:
        typedef             BirthInfo_                                                  BirthInfo;
        typedef             SimplexData_                                                SimplexData;


        struct SNode;
        typedef             bi::list<SNode, bi::constant_time_size<false> >             ZRow;

        // Simplex representation
        struct SHead: public SimplexData
        {
                            SHead(const SHead& other):
                                SimplexData(other), order(other.order)                  {}  // don't copy row since we can't
                            SHead(const SimplexData& sd, unsigned o): 
                                SimplexData(sd), order(o)                               {}

            // intrusive list corresponding to row of s in Z^*, not ordered in any particular order
            ZRow            row;
            unsigned        order;
        };

        typedef             s::list<SHead>                                              Simplices;
        typedef             typename Simplices::iterator                                SimplexIndex;

        struct Cocycle;
        typedef             s::list<Cocycle>                                            Cocycles;
        typedef             typename Cocycles::iterator                                 CocycleIndex;
        
        // An entry in a cocycle column; it's also an element in an intrusive list, hence the list_base_hook<>
        typedef             bi::list_base_hook<bi::link_mode<bi::auto_unlink> >         auto_unlink_hook;
        struct SNode: public auto_unlink_hook
        {
                            SNode()                                                     {}
                            SNode(SimplexIndex sidx): si(sidx)                          {}

            // eventually store a field element

            SimplexIndex    si;
            CocycleIndex    cocycle;                    // TODO: is there no way to get rid of this overhead?

            void            unlink()                    { auto_unlink_hook::unlink(); }
        };
        class CompareSNode;

        typedef             s::vector<SNode>                                            ZColumn;
        struct Cocycle
        {
                            Cocycle(const BirthInfo& b, unsigned o):
                                birth(b), order(o)                                      {}

            ZColumn         cocycle;
            BirthInfo       birth;
            unsigned        order;

            bool            operator<(const Cocycle& other) const                       { return order > other.order; }
            bool            operator==(const Cocycle& other) const                      { return order == other.order; }
        };

        typedef             boost::optional<BirthInfo>  Death;
        typedef             std::pair<SimplexIndex,
                                      Death>            IndexDeathPair;

        // return either a SimplexIndex or a Death
        // BI = BoundaryIterator; it should dereference to a SimplexIndex
        template<class BI>
        IndexDeathPair      add(BI begin, BI end, BirthInfo b, const SimplexData& sd = SimplexData());

        void                show_cycles() const;


    private:
        void                add_cocycle(Cocycle& z1, Cocycle& z2);

    private:
        Simplices           simplices_;
        Cocycles            cocycles_;
};

#include "cohomology-persistence.hpp"

#endif // __COHOMOLOGY_PERSISTENCE_H__
