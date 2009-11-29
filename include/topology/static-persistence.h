#ifndef __STATIC_PERSISTENCE_H__
#define __STATIC_PERSISTENCE_H__

#include "order.h"
#include "cycles.h"
#include "filtration.h"

#include <utilities/types.h>

#include <boost/progress.hpp>


template<class Data_, class ChainTraits_, class ContainerTraits_, class Element_ = use_default> 
struct PairCycleData: public Data_
{
    typedef     Data_                                                                   Data;
    typedef     typename if_default<Element_, PairCycleData>::type                      Element;
    typedef     PairCycleData<Data, ChainTraits_, ContainerTraits_, Element>            Self;

    typedef     typename ContainerTraits_::template rebind<Element>::other              ContainerTraits;
    typedef     typename ContainerTraits::Index                                         Index;
    typedef     typename ChainTraits_::template rebind<Index>::other                    ChainTraits;
    typedef     typename ChainTraits::Chain                                             Chain;
    typedef     Chain                                                                   Cycle;

                PairCycleData(Index p = Index(), const Cycle& z = Cycle(), const Data& d = Data()):
                    Data(d), pair(p), cycle(z)
                {}
    
    bool        sign() const                                                            { return cycle.empty(); }

    Index       pair;
    Cycle       cycle;
};

/**
 * Class: StaticPersistence
 * The class that encapsulates order and pairing information as well as 
 * implements methods to compute and maintain the pairing. Its most 
 * significant method is <pair_simplices()>.
 *
 * Template parameters:
 *   Data_ -                auxilliary contents to store with each OrderElement
 *   OrderDescriptor_ -     class describing how the order is stored; it defaults to <VectorOrderDescriptor> 
 *                          which serves as a prototypical class
 */
template<class Data_ =              Empty<>,
         class ChainTraits_ =       VectorChains<>,
         class Comparison_ =        GreaterComparison<>,
         class ContainerTraits_ =   VectorContainer<>,
         class Element_ =           PairCycleData<Data_, ChainTraits_, ContainerTraits_> >
class StaticPersistence
{
    public:
        // Typedef: Data
        // The data type stored in each order element
        typedef                         Data_                                                   Data;

        typedef                         Element_                                                Element;
        typedef                         typename ContainerTraits_::
                                                    template rebind<Element>::other             ContainerTraits;
        typedef                         typename ContainerTraits::Container                     Container;
        typedef                         Container                                               Order;
        typedef                         typename ContainerTraits::Index                         OrderIndex;
        typedef                         typename ContainerTraits::Element                       OrderElement;
        typedef                         typename ChainTraits_::
                                                    template rebind<OrderIndex>::other          ChainTraits;
        typedef                         typename ChainTraits::Chain                             Chain;
        typedef                         Chain                                                   Cycle;
        
        typedef                         typename Comparison_::
                                                    template rebind<OrderIndex>::other          OrderComparison;

        /* Constructor: StaticPersistence()
         * TODO: write a description
         *
         * Template parameters:
         *   Filtration -           filtration of the complex whose persistence we are computing
         */
        template<class Filtration>      StaticPersistence(const Filtration& f, 
                                                          const OrderComparison& ocmp = OrderComparison());
        
        // Function: pair_simplices()                                        
        // Compute persistence of the filtration
        void                            pair_simplices()                                        { pair_simplices<PairVisitor>(begin(), end(), PairVisitor(size())); }

        // Functions: Accessors
        //   begin() -              returns OrderIndex of the first element
        //   end() -                returns OrderIndex of one past the last element
        //   size() -               number of elements in the StaticPersistence
        OrderIndex                      begin()                                                 { return order_.begin(); }
        OrderIndex                      end()                                                   { return order_.end(); }
        size_t                          size() const                                            { return order_.size(); }
        const OrderComparison&          order_comparison() const                                { return ocmp_; }

    protected:
        // Function: pair_simplices(bg, end)
        // Compute persistence of the simplices in filtration between bg and end
        template<class Visitor>
        void                            pair_simplices(OrderIndex bg, OrderIndex end, const Visitor& visitor = Visitor());

        // Struct: PairVisitor
        // Acts as an archetype and if necessary a base class for visitors passed to <pair_simplices(bg, end, visitor)>.
        struct                          PairVisitor
        {
                                        PairVisitor(unsigned size): show_progress(size)         {}
            // Function: init(i)
            // Called after OrderElement pointed to by `i` has been initialized 
            // (its cycle is set to be its boundary, and pair is set to self, i.e. `i`)
            void                        init(OrderIndex i) const                                {}
            
            // Function: update(j, i)
            // Called after the cycle of `i` has been added to the cycle of `j`, 
            // this allows the derived class to perform the necessary updates 
            // (e.g., add `i`'s chain to `j`'s chain)
            void                        update(OrderIndex j, OrderIndex i) const                {}

            // Function: finished(j)
            // Called after the processing of `j` is finished.
            void                        finished(OrderIndex j) const                            { ++show_progress; }

            mutable boost::progress_display     
                                        show_progress;
        };

        const Order&                    order() const                                           { return order_; }

    private:
        Order                           order_;
        OrderComparison                 ocmp_;
};

#include "static-persistence.hpp"

#endif // __STATIC_PERSISTENCE_H__
