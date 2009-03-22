#ifndef __DYNAMIC_PERSISTENCE_H__
#define __DYNAMIC_PERSISTENCE_H__

#include "static-persistence.h"
#include <utilities/types.h>

#include <boost/progress.hpp>

#ifdef COUNTERS
static Counter*  cTrailLength =             GetCounter("persistence/pair/traillength");     // the size of matrix U in RU decomposition
static Counter*  cChainLength =             GetCounter("persistence/pair/chainlength");     // the size of matrix V in R=DV decomposition
#endif // COUNTERS

template<class Data, class OrderDescriptor_, class ConsistencyIndex>
struct TrailData_: public Data
{
    typedef TrailData_<Data, OrderDescriptor_, ConsistencyIndex>                Self;

    typedef typename OrderTraits<typename OrderDescriptor_::
                                          template RebindData<Self>::
                                          other>::Index                         OrderIndex;
    typedef typename OrderDescriptor_::
                     Chains::template rebind<OrderIndex>::other::Chain          Trail;
    
    template<class Comparison>
    struct ConsistencyComparison
    {
        typedef             const OrderIndex&                                   first_argument_type;
        typedef             const OrderIndex&                                   second_argument_type;
        typedef             bool                                                result_type;

        ConsistencyComparison(const Comparison& cmp = Comparison()): cmp_(cmp)  {}

        bool operator()(const OrderIndex& a, const OrderIndex& b) const         { return cmp_(a->consistency, b->consistency); }

        Comparison      cmp_;
    };

    Trail                                                                       trail;
    ConsistencyIndex                                                            consistency;
};

/**
 * Class: DynamicPersistenceTrails
 * Derives from StaticPersistence and allows one to update persistence 
 * after a transposition of two contiguous simplices in a filtration. 
 * In addition to reduced cycles, it stores each OrderElement's trails,
 * i.e. in addition to matrix R, it stores matrix U in vineyard notation.
 *
 * Template parameters:
 *   Data_ -                auxilliary contents to store with each OrderElement
 *   OrderDescriptor_ -     class describing how the order is stored; it defaults to <VectorOrderDescriptor> 
 *                          which serves as a prototypical class
 */
// TODO: perhaps Consistency should be wrapped into a ConsistencyDescriptor that somehow knows how to initialize it. 
// That way one could provide a simple consistency descriptor that just stored some integers describing the original 
// position, or one could provide consistency that is references into the complex
template<class Data_ =                  Empty<>, 
         class OrderDescriptor_ =       VectorOrderDescriptor<>,
         class ConsistencyIndex_ =      size_t,
         class ConsistencyComparison_ = std::less<ConsistencyIndex_> >
class DynamicPersistenceTrails: 
    public StaticPersistence<TrailData_<Data_, OrderDescriptor_, ConsistencyIndex_>, 
                             OrderDescriptor_>
{
    public:
        typedef         Data_                                                           Data;
        typedef         TrailData_<Data_, OrderDescriptor_, ConsistencyIndex_>          TrailData;
        typedef         StaticPersistence<TrailData, OrderDescriptor_>                  Parent;
 
        typedef         typename Parent::Traits                                         Traits;
        typedef         typename Parent::OrderDescriptor                                OrderDescriptor;
        typedef         typename Parent::OrderComparison                                OrderComparison;
        typedef         typename Parent::OrderIndex                                     OrderIndex;
        typedef         ConsistencyIndex_                                               ConsistencyIndex;
        typedef         ThreeOutcomeCompare<
                            typename TrailData::
                            template ConsistencyComparison<ConsistencyComparison_> >    ConsistencyComparison;

        /**
         * Constructor: DynamicPersistenceTrails()
         * TODO: write a description
         *
         * Template parameters:
         *   Filtration -           filtration of the complex whose persistence we are computing
         */
        template<class Filtration>      DynamicPersistenceTrails(const Filtration&              f, 
                                                                 const OrderComparison&         ocmp =  OrderComparison(),
                                                                 const ConsistencyComparison_&  ccmp =  ConsistencyComparison_());
        
        void                            pair_simplices();

        // Function: transpose(i)
        // Tranpose i and the next element. 
        // Returns: true iff the pairing switched.
        bool                            transpose(OrderIndex i)                         { return transpose(i, TranspositionVisitor()); }
        
        template<class Visitor>
        bool                            transpose(OrderIndex i, const Visitor& visitor = Visitor());

        using                           Parent::begin;
        using                           Parent::end;
        using                           Parent::size;

        // Struct: TranspositionVisitor
        //
        // For example, a VineardVisitor could implement this archetype.
        struct TranspositionVisitor
        {
            // Function: transpose(i)
            // This function is called before transposition is processed 
            // (at the very beginning of <transpose(i, visitor)>). It is meant to update any structures 
            // that may need to be updated, but perhaps it has other uses as well.
            void                        transpose(OrderIndex i) const                   {}

            // Function: switched(i, type)
            // This function is called after the transposition if the switch in pairing has occured.
            // `i` is the index of the preceding simplex after the transposition. 
            // `type` indicates the <SwitchType>.
            void                        switched(OrderIndex i, SwitchType type) const   {}
        };

    protected:
        using                           Parent::order;

    private:
        void                            swap(OrderIndex i, OrderIndex j);
        void                            pairing_switch(OrderIndex i, OrderIndex j);

        struct PairingTrailsVisitor: public Parent::PairVisitor 
        {
            // TODO: this is specialized for std::vector
                                        PairingTrailsVisitor(OrderIndex bg, ConsistencyComparison ccmp): 
                                            bg_(bg), ccmp_(ccmp)                        {}

            void                        init(OrderIndex i) const                        { i->consistency = i - bg_; i->trail.append(i, ccmp_); Count(cTrailLength); }
            void                        update(OrderIndex j, OrderIndex i) const        { i->pair->trail.append(j, ccmp_); Count(cTrailLength); }

            OrderIndex                  bg_;
            ConsistencyComparison       ccmp_;
        };

        ConsistencyComparison           ccmp_;
};

template<class Data, class OrderDescriptor_, class ConsistencyIndex>
struct ChainData_: public Data
{
    typedef ChainData_<Data, OrderDescriptor_, ConsistencyIndex>                Self;

    typedef typename OrderTraits<typename OrderDescriptor_::
                                          template RebindData<Self>::
                                          other>::Index                         OrderIndex;
    typedef typename OrderDescriptor_::
                     Chains::template rebind<OrderIndex>::other::Chain          Chain;
    
    template<class Comparison>
    struct ConsistencyComparison: public std::binary_function<const OrderIndex&, const OrderIndex&, bool>
    {
        ConsistencyComparison(const Comparison& cmp = Comparison()): cmp_(cmp)  {}

        bool operator()(const OrderIndex& a, const OrderIndex& b) const         { return cmp_(a->consistency, b->consistency); }

        Comparison      cmp_;
    };

    Chain                                                                       chain;
    ConsistencyIndex                                                            consistency;
};

/**
 * Class: DynamicPersistenceChains
 *
 * TODO: below comment is incorrect; nothing dynamic about this yet.
 * Derives from StaticPersistence and allows one to update persistence 
 * after a transposition of two contiguous simplices in a filtration. 
 * In addition to reduced cycles, it stores each OrderElement's chains,
 * i.e. in addition to matrix R, it stores matrix V in vineyard notation.
 *
 * Template parameters:
 *   Data_ -                auxilliary contents to store with each OrderElement
 *   OrderDescriptor_ -     class describing how the order is stored; it defaults to <VectorOrderDescriptor> 
 *                          which serves as a prototypical class
 */
template<class Data_ =                  Empty<>, 
         class OrderDescriptor_ =       VectorOrderDescriptor<>,
         class ConsistencyIndex_ =      size_t,
         class ConsistencyComparison_ = std::less<ConsistencyIndex_> >
class DynamicPersistenceChains: 
    public StaticPersistence<ChainData_<Data_, OrderDescriptor_, ConsistencyIndex_>, 
                             OrderDescriptor_>
{
    public:
        typedef         Data_                                                           Data;
        typedef         ChainData_<Data_, OrderDescriptor_, ConsistencyIndex_>          ChainData;
        typedef         StaticPersistence<ChainData, OrderDescriptor_>                  Parent;
 
        typedef         typename Parent::Traits                                         Traits;
        typedef         typename Parent::OrderDescriptor                                OrderDescriptor;
        typedef         typename Parent::OrderComparison                                OrderComparison;
        typedef         typename Parent::OrderIndex                                     OrderIndex;
        typedef         ConsistencyIndex_                                               ConsistencyIndex;
        typedef         ThreeOutcomeCompare<
                            typename ChainData::
                            template ConsistencyComparison<ConsistencyComparison_> >    ConsistencyComparison;

        /**
         * Constructor: DynamicPersistenceChains()
         * TODO: write a description
         *
         * Template parameters:
         *   Filtration -           filtration of the complex whose persistence we are computing
         */
        template<class Filtration>      DynamicPersistenceChains(const Filtration&              f, 
                                                                 const OrderComparison&         ocmp =  OrderComparison(),
                                                                 const ConsistencyComparison_&  ccmp =  ConsistencyComparison_());
        
        void                            pair_simplices();

        // Function: transpose(i)
        // Tranpose i and the next element. 
        // Returns: true iff the pairing switched.
        // TODO
        //bool                            transpose(OrderIndex i)                         { return transpose(i, TranspositionVisitor()); }
        
        // TODO: the main missing piece to be dynamic
        //template<class Visitor>
        //bool                            transpose(OrderIndex i, const Visitor& visitor = Visitor());

        using                           Parent::begin;
        using                           Parent::end;
        using                           Parent::size;

        // Struct: TranspositionVisitor
        //
        // For example, a VineardVisitor could implement this archetype.
        struct TranspositionVisitor
        {
            // Function: transpose(i)
            // This function is called before transposition is processed 
            // (at the very beginning of <transpose(i, visitor)>). It is meant to update any structures 
            // that may need to be updated, but perhaps it has other uses as well.
            void                        transpose(OrderIndex i) const                   {}

            // Function: switched(i, type)
            // This function is called after the transposition if the switch in pairing has occured.
            // `i` is the index of the preceding simplex after the transposition. 
            // `type` indicates the <SwitchType>.
            void                        switched(OrderIndex i, SwitchType type) const   {}
        };

    protected:
        using                           Parent::order;

    private:
        void                            swap(OrderIndex i, OrderIndex j);
        void                            pairing_switch(OrderIndex i, OrderIndex j);

        struct PairingChainsVisitor: public Parent::PairVisitor 
        {
            // TODO: this is specialized for std::vector
                                        PairingChainsVisitor(OrderIndex bg, ConsistencyComparison ccmp, unsigned size): 
                                            bg_(bg), ccmp_(ccmp), show_progress(size)   {}

            void                        init(OrderIndex i) const                        { i->consistency = i - bg_; i->chain.append(i, ccmp_); }
            void                        update(OrderIndex j, OrderIndex i) const        { j->chain.add(i->pair->chain, ccmp_); }
            void                        finished(OrderIndex i) const                    { CountBy(cChainLength, i->chain.size()); ++show_progress; }

            OrderIndex                  bg_;
            ConsistencyComparison       ccmp_;

            mutable boost::progress_display     
                                        show_progress;
        };

        ConsistencyComparison           ccmp_;
};


#include "dynamic-persistence.hpp"

#endif  // __DYNAMIC_PERSISTENCE_H__
